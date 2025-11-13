#if 1
/*=====================================================================
 * File: ih_control.c
 * IH控制模块 + 煮饭状态机（移植自 key.c）
 * 支持：按键交互 + 预约 + 自动曲线 + 功率闭环 + 安全保护
 * API：cook_rice_start() 等（key.c 可调用）
=====================================================================*/

#include "app.h"
#include "platform.h"
#include <math.h>
#include "key.h"
#define LOG_TAG    "IH_CTRL"

//==================== 调试开关====================
#ifndef IH_CONTROL_DEBUG
#define IH_CONTROL_DEBUG    1
#endif

#if IH_CONTROL_DEBUG
#define IH_LOG_I(...)    LOG_I(__VA_ARGS__)
#define IH_LOG_W(...)    LOG_W(__VA_ARGS__)
#define IH_LOG_E(...)    LOG_E(__VA_ARGS__)
#else
#define IH_LOG_I(...)    do{}while(0)
#define IH_LOG_W(...)    do{}while(0)
#define IH_LOG_E(...)    LOG_E(__VA_ARGS__)   // 错误保留
#endif

//==================== 全局变量（移植自 key.c）====================
extern rice_information_t rice_info;

// IH控制变量
static float target_power_w = 0;
static bool ih_enabled = false;
// 获取当前频率（正确）
static float get_pwm_freq(void)
{
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    uint32_t tim_clk = RCC_Clocks.PCLK2_Frequency * ((RCC_Clocks.PCLK2_Frequency == RCC_Clocks.SYSCLK_Frequency) ? 1 : 2);  // TIM1_CLK = APB2 × 2
    uint32_t psc = (PSF_COMP_TIM->PSC) + 1;
    uint32_t arr = (PSF_COMP_TIM->ARR) + 1;

    return (float)tim_clk / (2 * psc * arr);  // 中心对齐：×2
}

// 设置目标频率 → ARR（精确）
static void set_pwm_freq(float target_freq_hz)
{
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    uint32_t tim_clk = RCC_Clocks.PCLK2_Frequency * ((RCC_Clocks.PCLK2_Frequency == RCC_Clocks.SYSCLK_Frequency) ? 1 : 2);
    uint32_t psc = (PSF_COMP_TIM->PSC) + 1;

    // 公式：ARR = tim_clk / (2 * psc * f) - 1
    uint32_t arr = (uint32_t)((float)tim_clk / ( psc * target_freq_hz) + 0.5f) - 1;

    // 频率安全范围（IH 推荐 18~65kHz）
    uint32_t arr_min = (uint32_t)((float)tim_clk / ( psc * 65000) + 0.5f) - 1; // ~65kHz
    uint32_t arr_max = (uint32_t)((float)tim_clk / ( psc * 18000) + 0.5f) - 1; // ~18kHz

    if (arr < arr_min) arr = arr_min;
    if (arr > arr_max) arr = arr_max;

    PSF_COMP_TIM->ARR = arr;

    // 固定占空比 49%（防穿透）
    float duty = 0.49f;
    PSF_COMP_TIM->CCR1 = (uint32_t)(arr * duty);

#if IH_CONTROL_DEBUG
    float real_freq = (float)tim_clk / (psc * (arr + 1));
    IH_LOG_I("set_freq: %.0fHz → ARR=%lu, real=%.1fHz", target_freq_hz, arr, real_freq);
#endif
}
//==================== IH 基础控制 ===================
static void ih_pwm_enable(bool en)
{
    if (en ) {
        // 启用 PWM：恢复定时器 + 输出
        if(ih_enabled == en) return; // 状态未变无需操作
        // TIM_Cmd(PSF_COMP_TIM, ENABLE);
    
        TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
        TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Enable);
        TIM_CtrlPWMOutputs(PSF_COMP_TIM, ENABLE);
        GPIO_SetBits(GPIOA, GPIO_Pin_4);  // 使能驱动
        IH_LOG_I("IH 加热开启（PWM 完全启用）");
    } else {
        if(ih_enabled == en) return; // 状态未变无需操作
        // 关闭 PWM：停止输出 + 禁用定时器
        // PSF_COMP_TIM->CCR1 = 0;  // 先清占空比（安全）
        // PSF_COMP_TIM->CCR2 = 0;
        TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
        TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Disable);
        TIM_CtrlPWMOutputs(PSF_COMP_TIM, DISABLE);  // 立即停止 PWM 输出
		
        TIM_Cmd(PSF_COMP_TIM, DISABLE);             // 完全禁用定时器
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);      // 关闭驱动
        IH_LOG_W("IH 加热关闭（PWM 完全禁用）");
    }
    ih_enabled = en;
}

static void set_ih_power_w(float power_w)
{
    if (power_w < 0) power_w = 0;
    if (power_w > 1800) power_w = 1800;
    target_power_w = power_w;

    if (power_w > 50 && !ih_enabled) ih_pwm_enable(true);
    if (power_w <= 50) ih_pwm_enable(false);
}


// =============================================================
// 自动功率控制（快速响应 + ±50W 容差）
// =============================================================
// =============================================================
// 自动功率调节（非PID）- 从40kHz起步、20~60kHz范围
// 目标：自动调节频率以维持目标功率 target_power_w
// =============================================================
static void power_auto_control(void)
{
    static float freq = 40000.0f;      // 当前频率 Hz
    static bool initialized = false;
    static rt_tick_t last_tick = 0;
    static float filtered_power = 0.0f;

    // 每1s执行一次
    rt_tick_t now = rt_tick_get();
    if (now - last_tick < 1000) return;
    last_tick = now;

    // 读取当前功率
    ina226_read_rice_info(&rice_info);
    float measured_power = rice_info.P_supply / 1000.0f; // mW -> W（确认单位）

    // 一阶滤波
    if (!initialized) {
        filtered_power = measured_power;
        initialized = true;
    }
    filtered_power = 0.7f * filtered_power + 0.3f * measured_power;
    measured_power = filtered_power;

    // 误差计算
    float err = target_power_w - measured_power;
    float abs_err = fabsf(err);

    // 调节步进大小（误差越大调节越快）
    float step;
    if (abs_err > 800) step = 500.0f;
    else if (abs_err > 400) step = 300.0f;
    else if (abs_err > 150) step = 100.0f;
    else step = 100.0f;

    // 容差范围 ±50W 内不调节
    const float POWER_TOL = 50.0f;
    if (abs_err <= POWER_TOL) {
        IH_LOG_I("Hold: %.0fW (target %.0fW) @ %.0fHz", measured_power, target_power_w, freq);
        return;
    }

    // 关键调节逻辑（功率下降时 -> 降频，功率上升时 -> 升频）
    // 因为初始在过谐振点（40kHz右侧），
    // 所以降低频率（往左）会靠近谐振，功率↑；
    // 升高频率（往右）会远离谐振，功率↓。
    if (err > 0) {
        // 当前功率 < 目标功率 → 降频
        freq -= step;
    } else {
        // 当前功率 > 目标功率 → 升频
        freq += step;
    }

    // 限制频率范围
    if (freq < 20000.0f) freq = 20000.0f;
    if (freq > 60000.0f) freq = 60000.0f;

    // 应用频率
    set_pwm_freq(freq);

    IH_LOG_I("Adj: %.0fW(target %.0f) err=%.0f step=%.0f → f=%.0fHz",
             measured_power, target_power_w, err, step, freq);
}



// 安全保护
static bool safety_check(void)
{
    if (rice_info.T_bottom > 185 || rice_info.T_side > 155 || rice_info.T_lid > 125) {
        IH_LOG_E("过温保护！底:%d°C 侧:%d°C 盖:%d°C", rice_info.T_bottom, rice_info.T_side, rice_info.T_lid);
        ih_pwm_enable(false);
        state = STANDBY;
        return false;
    }
    return true;
}

//==================== 状态切换日志（移植自 key.c）====================
static void log_state_enter(void)
{
    if (state == last_state) return;
    const char *name[] = {"STANDBY", "RICE_SEL", "MODE_SEL", "BOOK_SET",
                          "COOKING", "WARMING", "BOOK_WAIT"};
    IH_LOG_I("状态切换 → %s", name[state]);
    last_state = state;
}

static void log_rice_change(void)
{
    static uint8_t last_rice = 0xFF;
    if (rice_type != last_rice) {
        IH_LOG_I("米种 → %s (r%d)", rice_name[rice_type], rice_type + 1);
        last_rice = rice_type;
    }
}

static void log_mode_change(void)
{
    static uint8_t last_mode = 0xFF;
    if (cook_mode != last_mode) {
        IH_LOG_I("模式 → %s (P%d)", mode_name[cook_mode], cook_mode + 1);
        last_mode = cook_mode;
    }
}

static void log_book_change(void)
{
    static uint8_t last_book = 0xFF;
    if (book_hour != last_book) {
        IH_LOG_I("预约 → %d小时", book_hour);
        last_book = book_hour;
    }
}

//==================== 启动烹饪（移植自 key.c）====================
void start_cooking(void)
{
    switch (cook_mode) {
        case 0: cook_total_sec = 30 * 60; IH_LOG_I("精煮 30分钟"); break;
        case 1: cook_total_sec = 15 * 60; IH_LOG_I("快煮 15分钟"); break;
        case 2: cook_total_sec = 45 * 60; IH_LOG_I("稀饭 45分钟"); break;
        case 3: cook_total_sec = 10 * 60; IH_LOG_I("热饭 10分钟"); break;
    }
    if (rice_type == 2) cook_total_sec = 48 * 60;  // 东北米顶级模式
    cook_start_tick = rt_tick_get();
    cook_active = true;
    state = COOKING;
    IH_LOG_I("IH加热启动：%s + %s", rice_name[rice_type], mode_name[cook_mode]);
}

//==================== 煮饭状态机（移植自 key.c + IH融合）====================
static void state_machine(void)
{
    rt_tick_t now = rt_tick_get();
    log_state_enter();
    log_rice_change();
    log_mode_change();
    log_book_change();

    if (!safety_check()) return;

    switch (state) {
        case STANDBY:
            set_ih_power_w(0);
            break;

        case RICE_SEL:
            set_ih_power_w(0);  // 选米时关加热
            // TODO: LED闪米种灯
            break;

        case MODE_SEL:
            set_ih_power_w(0);
            // TODO: LED闪模式灯
            break;

        case BOOK_SET:
            set_ih_power_w(0);
            // TODO: 显示预约小时
            break;

        case BOOK_WAIT:
            if (!book_active) {
                book_start_tick = rt_tick_get();
                book_active = true;
                IH_LOG_I("预约开始：%d小时", book_hour);
            }
            uint32_t book_elapsed = (now - book_start_tick) / RT_TICK_PER_SECOND;
            uint32_t book_remain = (book_hour * 3600 > book_elapsed) ? (book_hour * 3600 - book_elapsed) : 0;

            if (book_remain % 60 == 0 && book_remain != last_book_remain && book_remain > 0) {
                IH_LOG_I("预约剩余：%02d:%02d", book_remain / 3600, (book_remain % 3600) / 60);
                last_book_remain = book_remain;
            }

            if (book_remain == 0) {
                IH_LOG_I("预约完成 → 开始煮饭");
                start_cooking();
                book_active = false;
                last_book_remain = 0xFFFFFFFF;
            }
            set_ih_power_w(0);  // 预约时关加热
            break;

        case COOKING:
            if (!cook_active) break;
            uint32_t cook_elapsed = (now - cook_start_tick) / RT_TICK_PER_SECOND;
            uint32_t cook_remain = (cook_total_sec > cook_elapsed) ? (cook_total_sec - cook_elapsed) : 0;

            if (cook_remain % 60 == 0 && cook_remain != last_cook_remain && cook_remain > 0) {
                IH_LOG_I("煮饭剩余：%02d:%02d", cook_remain / 60, cook_remain % 60);
                last_cook_remain = cook_remain;
            }

            // IH自动曲线（根据时间/温度调整功率）
            uint32_t runtime = cook_elapsed;
            if (rice_type == 2) { 
                if (runtime < 18*60)      set_ih_power_w(600);   // 泡米
                else if (runtime < 26*60) set_ih_power_w(1400);  // 猛火
                else if (runtime < 30*60) set_ih_power_w(1100);  // 收汁
                else if (runtime < 42*60) set_ih_power_w(300);   // 焖饭
                else { state = WARMING; set_ih_power_w(80); }
            } else {
                // 其他米种简单曲线（可扩展）
                set_ih_power_w(cook_remain > 10*60 ? 1200 : 400);
            }

            if (cook_remain == 0) {
                IH_LOG_I("煮饭完成 → 保温");
                state = WARMING;
                cook_active = false;
                last_cook_remain = 0xFFFFFFFF;
            }

            // 功率闭环
            if (target_power_w > 50) power_auto_control();
            break;

        case WARMING:
            set_ih_power_w(rice_info.T_bottom < 70 ? 120 : 60);
            // TODO: LED黄灯常亮
            break;
    }
}


//==================== 主线程（移植 thread_logic_task_entry）====================
void thread_logic_task_entry(void *param)
{
   RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);

    uint32_t tim_clk = RCC_Clocks.PCLK2_Frequency * ((RCC_Clocks.PCLK2_Frequency == RCC_Clocks.SYSCLK_Frequency) ? 1 : 2); ;  // TIM1为高级定时器，TIM1_CLK
    uint16_t target_freq = 40000;
    uint16_t psc = 1;
    // 与TIM1_PWM_Complementary_SingleChannel_Config一致，中心对齐：freq = tim_clk / (2 * arr * psc)
    // 频率范围20kHz~60kHz，对应arr=2400~800
    uint16_t arr = (tim_clk + (target_freq * 2 * psc) / 2) / (target_freq * psc);  // 四舍五入

    IH_LOG_I("=== MM32F027 @PCLK2 配置 ===");
    IH_LOG_I("SYSCLK=%lu Hz, PCLK1=%lu Hz, PCLK2=%lu Hz, TIM1_CLK=%lu Hz", 
             RCC_Clocks.SYSCLK_Frequency, RCC_Clocks.PCLK1_Frequency, RCC_Clocks.PCLK2_Frequency, tim_clk);
    IH_LOG_I("40kHz PWM: arr=%u (ARR=%u), psc=%u | 实际f=%.1f Hz", 
             arr, arr-1, psc, (float)tim_clk / (psc * 2 * arr));

    // full_bridge_init(arr, psc);  // arr=1200, ARR=1199
    TIM1_PWM_Complementary_SingleChannel_Config(PSFB_COMP_CH, arr, PSFB_DEADTIME);
    // ====================================================

    rt_thread_mdelay(100);
    last_state = STANDBY;
    IH_LOG_I("IH + 状态机启动。初始: STANDBY (PWM: 40kHz)");

    while (1) {
        ntc_read_rice_info(&rice_info);
        ina226_read_rice_info(&rice_info);
        state_machine();  // ← 核心！煮饭状态机在这里运行
        rt_thread_mdelay(10);  // 100Hz刷新
    }
}



//==================== 命令行（可选）====================
void cmd_status(int argc, char **argv)
{
    const char *states[] = {"待机", "选米", "选模式", "预约", "煮饭", "保温", "预约等待"};
    rt_kprintf("\n=== IH状态 ===\n");
    rt_kprintf("状态: %s\n", states[state]);
    rt_kprintf("米种: %s\n", rice_name[rice_type]);
    rt_kprintf("模式: %s\n", mode_name[cook_mode]);
    rt_kprintf("功率: %.0fW\n", rice_info.P_supply / 1000.0f);
    rt_kprintf("温度: 底%d°C 侧%d°C 盖%d°C\n", rice_info.T_bottom, rice_info.T_side, rice_info.T_lid);
    rt_kprintf("============\n");
}
MSH_CMD_EXPORT(cmd_status, Show IH status);

#else

/*=====================================================================
 * File: ih_control.c
 * IH控制模块 + 煮饭状态机（移植自 key.c）
 * 支持：按键交互 + 预约 + 自动曲线 + 功率闭环 + 安全保护
 * API：cook_rice_start() 等（key.c 可调用）
=====================================================================*/

#include "app.h"
#include "platform.h"
#include <math.h>
#include "key.h"
#define LOG_TAG    "IH_CTRL"

//==================== 调试开关====================
#ifndef IH_CONTROL_DEBUG
#define IH_CONTROL_DEBUG    1
#endif

#if IH_CONTROL_DEBUG
#define IH_LOG_I(...)    LOG_I(__VA_ARGS__)
#define IH_LOG_W(...)    LOG_W(__VA_ARGS__)
#define IH_LOG_E(...)    LOG_E(__VA_ARGS__)
#else
#define IH_LOG_I(...)    do{}while(0)
#define IH_LOG_W(...)    do{}while(0)
#define IH_LOG_E(...)    LOG_E(__VA_ARGS__)   // 错误保留
#endif

//==================== 全局变量（移植自 key.c）====================
extern rice_information_t rice_info;

// IH控制变量
static float target_power_w = 0;
static bool ih_enabled = false;

static float get_pwm_freq(void)
{
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    uint32_t tim_clk =  RCC_Clocks.PCLK2_Frequency * ((RCC_Clocks.PCLK2_Frequency == RCC_Clocks.SYSCLK_Frequency) ? 1 : 2);  // TIM_CLK
    uint32_t psc = PSF_COMP_TIM->PSC + 1;
    uint32_t arr = PSF_COMP_TIM->ARR + 1;
    return (float)tim_clk / (psc * 2 * arr);  // 中心对齐频率
}
//==================== IH 基础控制 ===================
static void ih_pwm_enable(bool en)
{
    static bool last_en = false;
    if (en ) {
        // 启用 PWM：恢复定时器 + 输出
        TIM_Cmd(PSF_COMP_TIM, ENABLE);
        TIM_CtrlPWMOutputs(PSF_COMP_TIM, ENABLE);
        GPIO_SetBits(GPIOA, GPIO_Pin_4);  // 使能驱动
        IH_LOG_I("IH 加热开启（PWM 完全启用）");
    } else {
        // 关闭 PWM：停止输出 + 禁用定时器
        PSF_COMP_TIM->CCR1 = 0;  // 先清占空比（安全）
        PSF_COMP_TIM->CCR2 = 0;
        TIM_CtrlPWMOutputs(PSF_COMP_TIM, DISABLE);  // 立即停止 PWM 输出
		
        TIM_Cmd(PSF_COMP_TIM, DISABLE);             // 完全禁用定时器
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);      // 关闭驱动
        IH_LOG_W("IH 加热关闭（PWM 完全禁用）");
    }
    ih_enabled = en;
}

static void set_ih_power_w(float power_w)
{
    if (power_w < 0) power_w = 0;
    if (power_w > 1800) power_w = 1800;
    target_power_w = power_w;

    if (power_w > 50 && !ih_enabled) ih_pwm_enable(true);
    if (power_w <= 50) ih_pwm_enable(false);
}

// 功率PI闭环（简版）
  static void power_pi_control(void)
{
    static float integral = 0.0f;
    static float last_output = 0.5f;
    static rt_tick_t last_pid_time = 0;
    rt_tick_t now = rt_tick_get();
    if (now - last_pid_time < 1000) return;  // 每1s调节一次
    last_pid_time = now;

    float actual_power = rice_info.I_supply*rice_info.V_supply / 1000000.0f;
    float error = target_power_w - actual_power;

    // -------------------------------
    // 1. 死区防抖
    // -------------------------------
    if (fabsf(error) < 30.0f) {
        return;
    }

    // -------------------------------
    // 2. 自适应增益：误差越大 → 增益越高
    // -------------------------------
    float abs_err = fabsf(error);
    float adapt_scale;

    if (abs_err > 1000.0f)      adapt_scale = 1.0f;   // 大误差全速
    else if (abs_err > 300.0f)  adapt_scale = 0.5f;   // 中等误差
    else                        adapt_scale = 0.2f;   // 小误差慢调

    // -------------------------------
    // 3. PI计算（动态Kp/Ki）
    // -------------------------------
    const float baseKp = 0.02f;
    const float baseKi = 0.005f;
    const float dt = 1.0f;

    float Kp = baseKp * adapt_scale;
    float Ki = baseKi * adapt_scale;

    integral += error * dt;
    if (integral > 500) integral = 500;
    if (integral < -500) integral = -500;

    float output = Kp * error + Ki * integral;

    // -------------------------------
    // 4. 输出限幅与低通滤波
    // -------------------------------
    if (output > 1.0f) output = 1.0f;
    if (output < 0.0f) output = 0.0f;

    // 输出平滑：0.7旧 + 0.3新
    output = 0.7f * last_output + 0.3f * output;
    last_output = output;

    // -------------------------------
    // 5. 输出映射（保持方向不变）
    //    output ↑ → freq ↓ → 功率 ↑
    // -------------------------------
    float freq_scale = 1.0f - (output - 0.5f) * 0.3f; // 0.3 控制调节幅度

    uint32_t old_arr_val = PSF_COMP_TIM->ARR + 1;
    uint32_t target_arr = (uint32_t)(old_arr_val / freq_scale);

    // -------------------------------
    // 6. 频率变化限速（防止抖动）
    // -------------------------------
    const int max_step = 15;  // 每次最大变化 ±15
    uint32_t new_arr = old_arr_val;

    if (target_arr > old_arr_val + max_step) new_arr = old_arr_val + max_step;
    else if (target_arr + max_step < old_arr_val) new_arr = old_arr_val - max_step;
    else new_arr = target_arr;

    // 限制最终范围
    if (new_arr < 400) new_arr = 400;
    if (new_arr > 1200) new_arr = 1200;

    // -------------------------------
    // 7. 写入寄存器
    // -------------------------------
    PSF_COMP_TIM->ARR = new_arr - 1;

    // float fixed_duty = 0.5f;
    // PSFB_TIM->CCR1 = (uint32_t)(new_arr * fixed_duty * 0.98f);
    // PSFB_TIM->CCR2 = PSFB_TIM->CCR1;

#if IH_CONTROL_DEBUG
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    float tim_clk = RCC_Clocks.PCLK2_Frequency * ((RCC_Clocks.PCLK2_Frequency == RCC_Clocks.SYSCLK_Frequency) ? 1 : 2);
    float freq = tim_clk / (1 * 2 * new_arr);
    IH_LOG_I("PI自适应: err=%.1fW, scale=%.1f, out=%.2f, freq=%.0fHz, act=%.0fW",
             error, adapt_scale, output, freq, actual_power);
#endif
}


// 安全保护
static bool safety_check(void)
{
    if (rice_info.T_bottom > 185 || rice_info.T_side > 155 || rice_info.T_lid > 125) {
        IH_LOG_E("过温保护！底:%d°C 侧:%d°C 盖:%d°C", rice_info.T_bottom, rice_info.T_side, rice_info.T_lid);
        ih_pwm_enable(false);
        state = STANDBY;
        return false;
    }
    return true;
}

//==================== 状态切换日志（移植自 key.c）====================
static void log_state_enter(void)
{
    if (state == last_state) return;
    const char *name[] = {"STANDBY", "RICE_SEL", "MODE_SEL", "BOOK_SET",
                          "COOKING", "WARMING", "BOOK_WAIT"};
    IH_LOG_I("状态切换 → %s", name[state]);
    last_state = state;
}

static void log_rice_change(void)
{
    static uint8_t last_rice = 0xFF;
    if (rice_type != last_rice) {
        IH_LOG_I("米种 → %s (r%d)", rice_name[rice_type], rice_type + 1);
        last_rice = rice_type;
    }
}

static void log_mode_change(void)
{
    static uint8_t last_mode = 0xFF;
    if (cook_mode != last_mode) {
        IH_LOG_I("模式 → %s (P%d)", mode_name[cook_mode], cook_mode + 1);
        last_mode = cook_mode;
    }
}

static void log_book_change(void)
{
    static uint8_t last_book = 0xFF;
    if (book_hour != last_book) {
        IH_LOG_I("预约 → %d小时", book_hour);
        last_book = book_hour;
    }
}

//==================== 启动烹饪（移植自 key.c）====================
void start_cooking(void)
{
    switch (cook_mode) {
        case 0: cook_total_sec = 30 * 60; IH_LOG_I("精煮 30分钟"); break;
        case 1: cook_total_sec = 15 * 60; IH_LOG_I("快煮 15分钟"); break;
        case 2: cook_total_sec = 45 * 60; IH_LOG_I("稀饭 45分钟"); break;
        case 3: cook_total_sec = 10 * 60; IH_LOG_I("热饭 10分钟"); break;
    }
    if (rice_type == 2) cook_total_sec = 48 * 60;  // 东北米顶级模式
    cook_start_tick = rt_tick_get();
    cook_active = true;
    state = COOKING;
    IH_LOG_I("IH加热启动：%s + %s", rice_name[rice_type], mode_name[cook_mode]);
}

//==================== 煮饭状态机（移植自 key.c + IH融合）====================
static void state_machine(void)
{
    rt_tick_t now = rt_tick_get();
    log_state_enter();
    log_rice_change();
    log_mode_change();
    log_book_change();

    if (!safety_check()) return;

    switch (state) {
        case STANDBY:
            set_ih_power_w(0);
            break;

        case RICE_SEL:
            set_ih_power_w(0);  // 选米时关加热
            // TODO: LED闪米种灯
            break;

        case MODE_SEL:
            set_ih_power_w(0);
            // TODO: LED闪模式灯
            break;

        case BOOK_SET:
            set_ih_power_w(0);
            // TODO: 显示预约小时
            break;

        case BOOK_WAIT:
            if (!book_active) {
                book_start_tick = rt_tick_get();
                book_active = true;
                IH_LOG_I("预约开始：%d小时", book_hour);
            }
            uint32_t book_elapsed = (now - book_start_tick) / RT_TICK_PER_SECOND;
            uint32_t book_remain = (book_hour * 3600 > book_elapsed) ? (book_hour * 3600 - book_elapsed) : 0;

            if (book_remain % 60 == 0 && book_remain != last_book_remain && book_remain > 0) {
                IH_LOG_I("预约剩余：%02d:%02d", book_remain / 3600, (book_remain % 3600) / 60);
                last_book_remain = book_remain;
            }

            if (book_remain == 0) {
                IH_LOG_I("预约完成 → 开始煮饭");
                start_cooking();
                book_active = false;
                last_book_remain = 0xFFFFFFFF;
            }
            set_ih_power_w(0);  // 预约时关加热
            break;

        case COOKING:
            if (!cook_active) break;
            uint32_t cook_elapsed = (now - cook_start_tick) / RT_TICK_PER_SECOND;
            uint32_t cook_remain = (cook_total_sec > cook_elapsed) ? (cook_total_sec - cook_elapsed) : 0;

            if (cook_remain % 60 == 0 && cook_remain != last_cook_remain && cook_remain > 0) {
                IH_LOG_I("煮饭剩余：%02d:%02d", cook_remain / 60, cook_remain % 60);
                last_cook_remain = cook_remain;
            }

            // IH自动曲线（根据时间/温度调整功率）
            uint32_t runtime = cook_elapsed;
            if (rice_type == 2) { 
                if (runtime < 18*60)      set_ih_power_w(600);   // 泡米
                else if (runtime < 26*60) set_ih_power_w(1400);  // 猛火
                else if (runtime < 30*60) set_ih_power_w(1100);  // 收汁
                else if (runtime < 42*60) set_ih_power_w(300);   // 焖饭
                else { state = WARMING; set_ih_power_w(80); }
            } else {
                // 其他米种简单曲线（可扩展）
                set_ih_power_w(cook_remain > 10*60 ? 1200 : 400);
            }

            if (cook_remain == 0) {
                IH_LOG_I("煮饭完成 → 保温");
                state = WARMING;
                cook_active = false;
                last_cook_remain = 0xFFFFFFFF;
            }

            // 功率闭环
            if (target_power_w > 50) power_pi_control();
            break;

        case WARMING:
            set_ih_power_w(rice_info.T_bottom < 70 ? 120 : 60);
            // TODO: LED黄灯常亮
            break;
    }
}


//==================== 主线程（移植 thread_logic_task_entry）====================
void thread_logic_task_entry(void *param)
{
   RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);

    uint32_t tim_clk =  RCC_Clocks.PCLK2_Frequency * ((RCC_Clocks.PCLK2_Frequency == RCC_Clocks.SYSCLK_Frequency) ? 1 : 2);  
    uint16_t target_freq = 40000;
    uint16_t arr = (tim_clk + (target_freq * 2)) / (target_freq * 2);  // 四舍五入
    uint16_t psc = 1;

    IH_LOG_I("=== MM32F027 @12MHz 配置 ===");
    IH_LOG_I("SYSCLK=%lu Hz, PCLK1=%lu Hz, TIM_CLK=%lu Hz", 
             RCC_Clocks.SYSCLK_Frequency, RCC_Clocks.PCLK1_Frequency, tim_clk);
    IH_LOG_I("40kHz PWM: arr=%u (ARR=%u), psc=%u | 实际f=%.1f Hz", 
             arr, arr-1, psc, (float)tim_clk / (psc * 2 * arr));

    full_bridge_init(arr, psc);  // arr=1200, ARR=1199
  
    // ====================================================

    rt_thread_mdelay(100);
    last_state = STANDBY;
    IH_LOG_I("IH + 状态机启动。初始: STANDBY (PWM: 40kHz)");

    while (1) {
        ntc_read_rice_info(&rice_info);
        ina226_read_rice_info(&rice_info);
        state_machine();  // ← 核心！煮饭状态机在这里运行
        rt_thread_mdelay(10);  // 100Hz刷新
    }
}



//==================== 命令行（可选）====================
void cmd_status(int argc, char **argv)
{
    const char *states[] = {"待机", "选米", "选模式", "预约", "煮饭", "保温", "预约等待"};
    rt_kprintf("\n=== IH状态 ===\n");
    rt_kprintf("状态: %s\n", states[state]);
    rt_kprintf("米种: %s\n", rice_name[rice_type]);
    rt_kprintf("模式: %s\n", mode_name[cook_mode]);
    rt_kprintf("功率: %.0fW\n", rice_info.P_supply / 1000.0f);
    rt_kprintf("温度: 底%d°C 侧%d°C 盖%d°C\n", rice_info.T_bottom, rice_info.T_side, rice_info.T_lid);
    rt_kprintf("============\n");
}
MSH_CMD_EXPORT(cmd_status, Show IH status);


#endif