/*=====================================================================
 * File: ih_control_adaptive.c
 * 商用级 IH 自适应谐振温度控制（1200W）
 * 优化：
 * - 阶段2最大功率运行，110°C加热，115°C降功率
 * - 底部温度 ≥120°C立即停止加热
 * - 温度低通滤波（alpha=0.9）
 * - 功率为0时立即关闭输出
 * - 煮饭过程中每分钟打印剩余时间
 * - 核心特性保持：PID 输出功率比
 * - 保留 RT-Thread 逻辑
 =====================================================================*/
#include "app.h"
#include "platform.h"
#include <math.h>
#include "key.h"

#define LOG_TAG "IH_ADAPT"
#ifndef CLAMP
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#endif

#ifndef IH_CONTROL_DEBUG
#define IH_CONTROL_DEBUG 1
#endif
#if IH_CONTROL_DEBUG
#define IH_LOG_I(...) LOG_I(__VA_ARGS__)
#define IH_LOG_W(...) LOG_W(__VA_ARGS__)
#define IH_LOG_E(...) LOG_E(__VA_ARGS__)
#else
#define IH_LOG_I(...) do{}while(0)
#define IH_LOG_W(...) do{}while(0)
#define IH_LOG_E(...) LOG_E(__VA_ARGS__)
#endif

//==================== 全局变量 ====================
extern rice_information_t rice_info;
static bool overheat_lock = false;

//==================== 烹饪模式配置表 ====================
typedef struct {
    uint32_t total_sec;
    uint32_t stage1_sec, stage2_sec, stage3_sec, stage4_sec;
    float t1, t2, t3, t4, t_keep;
} cook_profile_t;

static const cook_profile_t profiles[] = {
    {5*60, 1*60, 1*60, 2*60, 1*60, 110, 110, 110, 110, 110}, // 清洁
    {15*60, 1*30, 5*60, 4*60, 5*60, 70, 110, 98, 85, 72},    // 快煮
    {50*60, 10*60, 30*60, 5*60, 5*60, 60, 98, 95, 80, 70},   // 稀饭
    {10*60, 0*60, 5*60, 3*60, 2*60, 0, 95, 90, 80, 72}       // 热饭
};
static uint32_t stage_times[5] = {0};

//==================== IH 控制参数 ====================
#define FREQ_MIN_HZ 26500.0f
#define FREQ_MAX_HZ 40000.0f
#define FREQ_INIT_HZ 33000.0f

#define TEMP_STOP_HEAT 150.0f
#define TEMP_LIMIT_POWER 130.0f
#define LIMITED_POWER_W 400.0f

static float estimated_max_power = 1200.0f;
static float current_freq = FREQ_INIT_HZ;
static float last_power = 0.0f;
static float freq_step = 2000.0f;
static int perturb_dir = 1;
static rt_tick_t last_perturb_time = 0;
static float smooth_power_w = 0.0f;

static float filtered_temp = 0.0f;
static float target_temp = 0.0f;
rt_tick_t last_pid_time = 0;
//static uint32_t last_cook_remain = 0;

//==================== PWM 控制 ====================
static void set_pwm_freq(float freq_hz)
{
    freq_hz = CLAMP(freq_hz, FREQ_MIN_HZ, FREQ_MAX_HZ);
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    uint32_t tim_clk = RCC_Clocks.PCLK2_Frequency * ((RCC_Clocks.PCLK2_Frequency == RCC_Clocks.SYSCLK_Frequency) ? 1 : 2);
    uint32_t psc = (PSF_COMP_TIM->PSC) + 1;
    uint32_t arr = (uint32_t)((float)tim_clk / (psc * freq_hz) + 0.5f) - 1;
    uint32_t arr_min = (uint32_t)((float)tim_clk / (psc * FREQ_MAX_HZ) + 0.5f) - 1;
    uint32_t arr_max = (uint32_t)((float)tim_clk / (psc * FREQ_MIN_HZ) + 0.5f) - 1;
    if (arr < arr_min) arr = arr_min;
    if (arr > arr_max) arr = arr_max;
    PSF_COMP_TIM->ARR = arr;
    PSF_COMP_TIM->CCR1 = (uint32_t)(arr * 0.49f);
    current_freq = freq_hz;
    float real_freq = (float)tim_clk / (psc * (arr + 1));
    IH_LOG_I("set_freq: %.0fHz → ARR=%lu, real=%.1fHz", freq_hz, arr, real_freq);
}

static void ih_pwm_enable(bool en)
{
    if (en == !!(PSF_COMP_TIM->CR1 & TIM_CR1_CEN)) return;
    if (en) {
        TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
        TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Enable);
        TIM_CtrlPWMOutputs(PSF_COMP_TIM, ENABLE);
        TIM_Cmd(PSF_COMP_TIM, ENABLE);
        GPIO_SetBits(GPIOA, GPIO_Pin_4);
        IH_LOG_I("IH 加热开启");
    } else {
        TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
        TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Disable);
        TIM_CtrlPWMOutputs(PSF_COMP_TIM, DISABLE);
        TIM_Cmd(PSF_COMP_TIM, DISABLE);
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
        IH_LOG_W("IH 加热关闭");
    }
}

//==================== 温度控制 ====================
static void set_target_temperature(float temp)
{
    if (temp < 30) temp = 30;
    if (temp > 180) temp = 115;
    if (fabsf(temp - target_temp) > 0.5f) {
        target_temp = temp;
        IH_LOG_I("目标温度 → %.1f°C", target_temp);
    }
}

static void temp_control(void)
{
    if (overheat_lock) {
        ih_pwm_enable(false);
        return;
    }
	uint32_t now_time = rice_info.T_bottom;
    rt_tick_t now = rt_tick_get();
    if (now - last_pid_time < 1000) return; // 1s一次
    last_pid_time = now;
    filtered_temp = 0.9f * filtered_temp + 0.1f * now_time;
    float current_temp = filtered_temp;
	
    if (current_temp < 20 || current_temp > 200) return;

    // 高温保护
    if (now_time> TEMP_STOP_HEAT) {
        ih_pwm_enable(false);
        overheat_lock = true;
        IH_LOG_E("温度超150°C（%.1f°C），立即停止加热！", now_time);
        return;
    }

    // 限功率保护
    if (now_time> TEMP_LIMIT_POWER) {
        set_pwm_freq(32000.0f); // 最小功率
        ih_pwm_enable(true);
        return;
    }

    float error = target_temp - current_temp;

//    // ±5°C 内不加热
	if(now_time>=target_temp + 5.0f) 
	{ih_pwm_enable(false);return;}
    if (now_time>= target_temp - 5.0f) return;

    float target_freq;
    if (error > 15.0f) target_freq = 26500.0f; // 最大
    else if (error > 5.0f) target_freq = 30000.0f; // 中
    else
	{  
		ih_pwm_enable(false);
	}
    set_pwm_freq(target_freq);
    ih_pwm_enable(true);
}

//==================== 阶段2控制 ====================
static void stage2_max_power_control(void)
{
    if (overheat_lock) {
        ih_pwm_enable(false);
        return;
    }

    rt_tick_t now = rt_tick_get();
    static rt_tick_t lasttime = 0;
    if (now - lasttime < 1000) return;  // 1秒更新一次
    lasttime = now;

    filtered_temp = 0.9f * filtered_temp + 0.1f * rice_info.T_bottom;
    float current_temp = filtered_temp;

    set_target_temperature(110.0f);  // 阶段2目标温度 110°C

    // 底部温度到125°C立即保护
    if (rice_info.T_bottom >= 125) {
        ih_pwm_enable(false);
        overheat_lock = true;
        IH_LOG_E("底部温度 %d°C 超过安全保护 120°C，停止加热！", rice_info.T_bottom);
        return;
    }

    // 温度 ≥115°C 降为最小功率
    if ( rice_info.T_bottom >= 115.0f) {
        set_pwm_freq(32000.0f); // 最小功率
        ih_pwm_enable(true);
        IH_LOG_W("阶段2温度 %.1f°C ≥115°C，降为最小功率加热", current_temp);
        return;
    }

    // 温度 <110°C 最大功率加热
    set_pwm_freq(26500.0f); // 最大功率
    ih_pwm_enable(true);
    IH_LOG_I("阶段2最大功率运行，当前温度 %d°C,MOS1温度 %d°C，MOS2温度 %d°C",
             rice_info.T_bottom, rice_info.T_mos1, rice_info.T_mos2);
}

//==================== 安全保护 ====================
static bool safety_check(void)
{
    //---- 主锅底 > 150°C（致命保护，不恢复） ----
    if (filtered_temp > TEMP_STOP_HEAT) {
        IH_LOG_E("致命保护：锅底 %.1f°C 超150°C，锁死！", filtered_temp);
        ih_pwm_enable(false);
        overheat_lock = true;
        state = STANDBY;
        return false;
    }

    //---- 锅体结构温度保护（侧、盖），也不恢复 ----
    if (rice_info.T_side > 155 || rice_info.T_lid > 125) {
        IH_LOG_E("锅体结构温度过高：侧%d°C 盖%d°C（不恢复）", rice_info.T_side, rice_info.T_lid);
        ih_pwm_enable(false);
        overheat_lock = true;
        state = STANDBY;
        return false;
    }

    //=========================
    // 可自动恢复的保护
    //=========================

    //---- 底部温度 >=120°C（保护触发） ----
    if (rice_info.T_bottom >= 120) {
        IH_LOG_E("底部温度 %d°C ≥120°C，进入保护状态（可恢复）", rice_info.T_bottom);
        ih_pwm_enable(false);
        overheat_lock = true;
        return true;
    }

    //---- 底部温度恢复条件 ----
    if (overheat_lock && rice_info.T_bottom <= 110) {
        IH_LOG_W("底部温度降至 %d°C，恢复加热", rice_info.T_bottom);
        overheat_lock = false;
    }

    //---- MOS 温度保护 ----
    if (rice_info.T_mos1 > 80 || rice_info.T_mos2 > 80) {
        IH_LOG_W("MOS过温：MOS1=%d°C MOS2=%d°C → 保护中（可恢复）",
                 rice_info.T_mos1, rice_info.T_mos2);
        ih_pwm_enable(false);
        overheat_lock = true;
        return true;
    }

    //---- MOS 恢复条件 ----
    if (overheat_lock && rice_info.T_mos1 < 70 && rice_info.T_mos2 < 70) {
        IH_LOG_I("MOS温度恢复：MOS1=%d°C MOS2=%d°C → 解除保护", 
                 rice_info.T_mos1, rice_info.T_mos2);
        overheat_lock = false;
    }

    return true;
}

//==================== 状态机 ====================
static void state_machine(void)
{
    rt_tick_t now = rt_tick_get();
    if (!safety_check()) return;

    switch (state) {
        case COOKING:
            if (!cook_active) break;
            uint32_t cook_elapsed = (now - cook_start_tick) / RT_TICK_PER_SECOND;
            uint32_t cook_remain = (cook_total_sec > cook_elapsed) ? (cook_total_sec - cook_elapsed) : 0;
            if (cook_remain % 60 == 0 && cook_remain != last_cook_remain && cook_remain > 0) {
                IH_LOG_I("煮饭剩余：%02d:%02d", cook_remain / 60, cook_remain % 60);
                last_cook_remain = cook_remain;
            }

            const cook_profile_t *p = &profiles[cook_mode];
            if (cook_elapsed < stage_times[1]) {
                set_target_temperature(p->t1);
                temp_control();
            } else if (cook_elapsed < stage_times[2]) {
                stage2_max_power_control();
            } else if (cook_elapsed < stage_times[3]) {
                set_target_temperature(p->t3);
                temp_control();
            } else if (cook_elapsed < stage_times[4]) {
                set_target_temperature(p->t4);
                temp_control();
            } else {
                state = WARMING;
                set_target_temperature(p->t_keep);
                temp_control();
            }
            break;

        case WARMING:
            set_target_temperature(profiles[cook_mode].t_keep);
            temp_control();
            break;

        default:
            ih_pwm_enable(false);
            break;
    }
}

//==================== 启动烹饪 ====================
void start_cooking(void)
{
    cook_mode = CLAMP(cook_mode, 0, 3);
    const cook_profile_t *p = &profiles[cook_mode];
    cook_total_sec = p->total_sec;
    cook_start_tick = rt_tick_get();
    cook_active = RT_TRUE;
    state = COOKING;
    estimated_max_power = 1200.0f;
    stage_times[0] = 0;
    stage_times[1] = p->stage1_sec;
    stage_times[2] = stage_times[1] + p->stage2_sec;
    stage_times[3] = stage_times[2] + p->stage3_sec;
    stage_times[4] = stage_times[3] + p->stage4_sec;

    target_temp = p->t1;
    filtered_temp = p->t1;

    overheat_lock = false;
    last_cook_remain = 0;

    IH_LOG_I("开始煮饭：%s + %s | 最大功率 1200W | 总时长 %d分钟",
             rice_name[rice_type], mode_name[cook_mode], cook_total_sec / 60);
}

//==================== 主线程 ====================
void thread_logic_task_entry(void *param)
{
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    uint32_t tim_clk = RCC_Clocks.PCLK2_Frequency * ((RCC_Clocks.PCLK2_Frequency == RCC_Clocks.SYSCLK_Frequency) ? 1 : 2);
    uint16_t psc = 1;
    uint16_t arr = (uint32_t)((float)tim_clk / (psc * FREQ_INIT_HZ) + 0.5f) - 1;
    TIM1_PWM_Complementary_SingleChannel_Config(PSFB_COMP_CH, arr, PSFB_DEADTIME);
    IH_LOG_I("=== IH 自适应谐振控制启动 | 频率 26-40kHz | 商用级保护 ===");
    set_pwm_freq(FREQ_INIT_HZ);

    while (1) {
        state_machine();
        rt_thread_mdelay(10);
    }
}

//==================== 命令行状态查询 ====================
void cmd_ih_status(int argc, char **argv)
{
    const char *states[] = {"待机", "选米", "选模式", "预约", "煮饭", "保温", "预约等待"};
    rt_kprintf("\n=== IH 自适应谐振状态 ===\n");
    rt_kprintf("状态: %s | 模式: %s\n", states[state], mode_name[cook_mode]);
    rt_kprintf("温度: 底%d°C 侧%d°C 盖%d°C MOS1:%d°C MOS2:%d°C\n",
               rice_info.T_bottom, rice_info.T_side, rice_info.T_lid, rice_info.T_mos1, rice_info.T_mos2);
    rt_kprintf("功率: %.0fW / %.0fW (max) | 频率: %.0fHz\n",
               smooth_power_w, estimated_max_power, current_freq);
    rt_kprintf("======================\n");
}
MSH_CMD_EXPORT(cmd_ih_status, Show IH adaptive resonance status);
