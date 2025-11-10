
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
    uint32_t tim_clk = RCC_Clocks.PCLK1_Frequency * 2;  // TIM_CLK
    uint32_t psc = PSFB_TIM->PSC + 1;
    uint32_t arr = PSFB_TIM->ARR + 1;
    return (float)tim_clk / (psc * 2 * arr);  // 中心对齐频率
}
//==================== IH 基础控制 ===================
static void ih_pwm_enable(bool en)
{
    if (en) {
        // 启用 PWM：恢复定时器 + 输出
        TIM_Cmd(PSFB_TIM, ENABLE);
        TIM_CtrlPWMOutputs(PSFB_TIM, ENABLE);
        GPIO_SetBits(GPIOA, GPIO_Pin_4);  // 使能驱动
        IH_LOG_I("IH 加热开启（PWM 完全启用）");
    } else {
        // 关闭 PWM：停止输出 + 禁用定时器
        PSFB_TIM->CCR1 = 0;  // 先清占空比（安全）
        PSFB_TIM->CCR2 = 0;
        TIM_CtrlPWMOutputs(PSFB_TIM, DISABLE);  // 立即停止 PWM 输出
		
        TIM_Cmd(PSFB_TIM, DISABLE);             // 完全禁用定时器
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);      // 关闭驱动
//        IH_LOG_W("IH 加热关闭（PWM 完全禁用）");
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
  static float integral = 0;
    ina226_read_rice_info(&rice_info);
    static rt_tick_t last_pid_time = 0;
    rt_tick_t now = rt_tick_get();
   
    if (now - last_pid_time < 1000) return; // 1s 一次
    last_pid_time = now;
   
    float actual_power = rice_info.P_supply / 1000.0f;
    float error = target_power_w - actual_power;
    
    // 增大死区：|error|<200W 不调（防小扰动振荡）
    if (fabsf(error) < 50.0f) {
        #if IH_CONTROL_DEBUG
        IH_LOG_I("PID 跳过: error=%.1fW <50W (稳态死区)",actual_power);
        #endif
        return;
    }
    
    integral += error * 1.0f;  // dt=1s
    if (integral > 50) integral = 50;    // 超紧限幅，防饱和
    if (integral < -50) integral = -50;
    
    float output = 0.08 * error + 0.01f * integral;  // 极低增益：Kp=0.1, Ki=0.01
    if (output < 0.25f) output = 0.25f;  // 限 0.25-0.75，渐变避极值
    if (output > 0.75f) output = 0.75f;
    
    float fixed_duty = 0.5f;
    uint32_t old_arr_val = PSFB_TIM->ARR + 1;
    
    // 修正映射：高 output → scale 大 → ARR 小 → 高频增功率
    float freq_scale = 1.0f - (output - 0.5f) * 0.4f;  // 正比高频
    uint32_t new_arr = (uint32_t)(old_arr_val / freq_scale);
   
    // 修正限幅：400-1200 (60kHz ~ 20kHz)
    if (new_arr < 400 || new_arr > 1200) new_arr = old_arr_val;
    
    if (new_arr != old_arr_val) {
        PSFB_TIM->ARR = new_arr - 1;
    }
    
    PSFB_TIM->CCR1 = (uint32_t)(new_arr * fixed_duty * 0.98f);
    PSFB_TIM->CCR2 = (uint32_t)(new_arr * fixed_duty * 0.98f); // CCR1=CCR2
    
    // 动态 freq 日志（直接用时钟频率）
    #if IH_CONTROL_DEBUG
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    float tim_clk = RCC_Clocks.PCLK1_Frequency * 2.0f;
    float old_freq = tim_clk / (1 * 2 * old_arr_val);
    float new_freq = tim_clk / (1 * 2 * new_arr);
    IH_LOG_I("PID 1s 调节: error=%.1fW | output=%.2f | scale=%.2f | freq: %.0f→%.0f Hz (20-60kHz 限) | duty=50% | actual=%.0fW",
             error, output, freq_scale, old_freq, new_freq, actual_power);
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

    uint32_t tim_clk = RCC_Clocks.PCLK1_Frequency * 2;  // 48MHz *2 =96MHz
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