#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "platform.h"
#define TD_MID_CHANNEL ADC_Channel_4 // PA4 中心温度
#define TD_MOS1_CHANNEL ADC_Channel_5 // PA5 MOS1温度
#define TD_MOS2_CHANNEL ADC_Channel_6 // PA6 MOS2温度
#define TD_OUT_CHANNEL ADC_Channel_7 // PA7 输出温度
#define TD_BASE_CHANNEL ADC_Channel_8 // PB0 锅底温度
#define RES_VOLT_CHANNEL ADC_Channel_9 // PB1 谐振电压采集
#define LOG_TAG "adc"
#define LOG_LVL ELOG_LVL_VERBOSE
#define ADC_MAX_VALUE 4095u
#define VREF 3.3f
#define ALPHA 0.15f // IIR滤波系数
#define MAX_STEP_RATIO 0.20f // 最大可变化比（相对于上次值）
uint8_t adc_channel[] = {
    TD_MID_CHANNEL,
    TD_MOS1_CHANNEL,
    TD_MOS2_CHANNEL,
    TD_OUT_CHANNEL,
    TD_BASE_CHANNEL,
    RES_VOLT_CHANNEL
};
BOOL adc_transtion_complete = FALSE; //adc转换完成标志
#define adc_channel_count sizeof(adc_channel)/sizeof(adc_channel[0])
#define DMA_SAMPLES 10
uint16_t adc_buffer[adc_channel_count*DMA_SAMPLES] = {0};
static float adc_filtered[adc_channel_count]; // 每通道4B，总24B
static bool adc_init_done[adc_channel_count]; // 初始化标志，每通道1B
// =========================================================
// NTC 枚举类型
// =========================================================
typedef enum {
    NTC_CMFB_503F3950 = 0, // 50kΩ @25°C, B=3950K, 100k上拉
    NTC_SDNT_1608X103F3435, // 10kΩ @25°C, B=3435K, 10k上拉
    NTC_MF52D_104F3950,    // 100kΩ @25°C, B=3950K, 100k上拉 (新增)
    NTC_TYPE_COUNT
} ntc_type_t;
// =========================================================
// R-T 表结构
// =========================================================
typedef struct {
    int8_t temp; // 温度 (°C)
    float r_kohm; // 电阻 (kΩ)
} rt_point_t;
// =========================================================
// NTC 配置结构
// =========================================================
typedef struct {
    float r_fixed; // 上拉电阻 (Ω)
    const rt_point_t* table;
    uint16_t table_size;
    const char* name;
} ntc_config_t;
// =========================================================
// R-T 数据表
// =========================================================
const rt_point_t cmfb_table[] = {
    {-40, 1726.374}, {-35, 1239.081}, {-30, 900.160}, {-25, 661.420},
    {-20, 490.927}, {-15, 368.128}, {-10, 278.722}, { -5, 212.960},
    { 0, 164.069}, { 5, 127.484}, { 10, 99.788}, { 15, 78.710},
    { 20, 62.520}, { 25, 50.000}, { 30, 40.247}, { 35, 32.600},
    { 40, 26.564}, { 45, 21.769}, { 50, 17.941}, { 55, 14.864},
    { 60, 12.378}, { 65, 10.361}, { 70, 8.709}, { 75, 7.361},
    { 80, 6.245}, { 85, 5.323}, { 90, 4.555}, { 95, 3.914},
    {100, 3.377}, {105, 2.925}, {110, 2.542}, {115, 2.217},
    {120, 1.941}, {125, 1.705}
};
const rt_point_t sdnt_table[] = {
    {-40, 335.68}, {-35, 249.85}, {-30, 187.88}, {-25, 142.74},
    {-20, 109.38}, {-15, 84.48}, {-10, 65.78}, { -5, 51.62},
    { 0, 40.83}, { 5, 32.55}, { 10, 26.16}, { 15, 21.18},
    { 20, 17.28}, { 25, 14.20}, { 30, 11.76}, { 35, 9.80},
    { 40, 8.23}, { 45, 6.96}, { 50, 5.93}, { 55, 5.09},
    { 60, 4.40}, { 65, 3.83}, { 70, 3.36}, { 75, 2.96},
    { 80, 2.63}, { 85, 2.35}, { 90, 2.11}, { 95, 1.91},
    {100, 1.74}, {105, 1.59}, {110, 1.46}, {115, 1.35},
    {120, 1.25}, {125, 1.16}
};
const rt_point_t mf52_table[] = {
    {-40, 3169.000},{-35, 2336.580},{-30, 1730.230},{-25, 1287.000},
    {-20, 962.912},{-15, 725.581},{-10, 551.100},{-5, 422.050},
    {0, 326.560},{5, 253.647},{10, 198.920},{15, 157.118},
    {20, 124.940},{25, 100.000},{30, 80.501},{35, 65.188},
    {40, 53.080},{45, 43.451},{50, 35.750},{55, 29.558},
    {60, 24.554},{65, 20.489},{70, 17.172},{75, 14.453},
    {80, 12.213},{85, 10.362},{90, 8.824},{95, 7.544},
    {100, 6.474},{105, 5.576},
};
// =========================================================
// NTC 配置表
// =========================================================
const ntc_config_t ntc_configs[NTC_TYPE_COUNT] = {
    [NTC_CMFB_503F3950] = {
        .r_fixed = 100000.0f,
        .table = cmfb_table,
        .table_size = sizeof(cmfb_table)/sizeof(cmfb_table[0]),
        .name = "CMFB 503F3950 (50kΩ, 100k pull-up)"
    },
    [NTC_SDNT_1608X103F3435] = {
        .r_fixed = 10000.0f,
        .table = sdnt_table,
        .table_size = sizeof(sdnt_table)/sizeof(sdnt_table[0]),
        .name = "SDNT1608X103F3435 (10kΩ, 10k pull-up)"
    },
    [NTC_MF52D_104F3950] = {
        .r_fixed = 100000.0f,
        .table = mf52_table,
        .table_size = sizeof(mf52_table)/sizeof(mf52_table[0]),
        .name = "MF52D 104F3950 (100kΩ, 100k pull-up)"
    }
};
// =========================================================
// 电压 → 电阻 (Ω)
// =========================================================
static float voltage_to_resistance(float vout, float r_fixed)
{
    if (vout < 0.01f || vout > 3.29f) return -1.0f;
    return r_fixed * (vout / (3.3f - vout));
}
// =========================================================
// 查表 + 线性插值
// =========================================================
static float lookup_temperature(const rt_point_t* table, uint16_t size, float r_kohm)
{
    if (r_kohm >= table[0].r_kohm) return table[0].temp;
    if (r_kohm <= table[size-1].r_kohm) return table[size-1].temp;
    for (int i = 0; i < size - 1; i++) {
        float r1 = table[i].r_kohm;
        float r2 = table[i+1].r_kohm;
        if (r_kohm >= r2 && r_kohm <= r1) {
            float t1 = table[i].temp;
            float t2 = table[i+1].temp;
            return t1 + (t2 - t1) * (r1 - r_kohm) / (r1 - r2);
        }
    }
    return -999.0f;
}
// =========================================================
// 电压 → 温度
// =========================================================
float ntc_voltage_to_temperature(float vout, ntc_type_t type)
{
    if (type >= NTC_TYPE_COUNT) return -999.0f;
    const ntc_config_t* cfg = &ntc_configs[type];
    float r_ohm = voltage_to_resistance(vout, cfg->r_fixed);
    if (r_ohm < 0) return -999.0f;
    return lookup_temperature(cfg->table, cfg->table_size, r_ohm / 1000.0f);
}
/**
 * 计算均值（DMA批量结果）
 */
static inline float adc_average(uint16_t *buf, uint8_t count)
{
    uint32_t sum = 0;
    for (uint8_t i = 0; i < count; i++)
        sum += buf[i];
    return (float)sum / count;
}
/**
 * 极限节省滤波器
 */
float adc_filter_process(uint8_t ch, uint16_t *samples, uint8_t count, float vcc)
{
    float raw_avg = adc_average(samples, count);
    float v_in = raw_avg * vcc / ADC_MAX_VALUE;
    // 第一次初始化
    if (!adc_init_done[ch]) {
        adc_filtered[ch] = v_in;
        adc_init_done[ch] = true;
        return v_in;
    }
    // 限幅判断
    float diff = v_in - adc_filtered[ch];
    float limit = MAX_STEP_RATIO * adc_filtered[ch];
    if (fabsf(diff) > limit)
        diff = (diff > 0 ? limit : -limit);
    // IIR更新
    adc_filtered[ch] += ALPHA * diff;
    return adc_filtered[ch];
}
/**
 * 更新全部通道
 */
void adc_filter_update_all(float *voltages, float vcc)
{
   for (uint8_t ch = 0; ch < adc_channel_count; ch++)
    {
        uint16_t samples[DMA_SAMPLES];
        for (uint8_t i = 0; i < DMA_SAMPLES; i++)
        {
            samples[i] = adc_buffer[i * adc_channel_count + ch];
        }
        voltages[ch] = adc_filter_process(ch, samples, DMA_SAMPLES, vcc);
    }
}
// =========================================================
// 独立函数：采集并更新温度信息
// =========================================================
void ntc_read_rice_info(rice_information_t *info)
{
// if(adc_transtion_complete == FALSE) return;
// if(RESET != DMA_GetFlagStatus(DMA1_FLAG_TC5))
  {
   // 参考电压采样（用于校准）
// float vcc_adc = adc_read_single(ADC1, ADC_Channel_VoltReference);
    float vcc =3.3; //vcc_adc * VREF / ADC_MAX_VALUE;
    float voltages[adc_channel_count];
    adc_filter_update_all(voltages, vcc);
    // === 电压 → 温度 ===
//    info->T_bottom = (uint16_t)ntc_voltage_to_temperature(voltages[0], NTC_SDNT_1608X103F3435);
    info->T_mos1 = (uint16_t)ntc_voltage_to_temperature(voltages[1], NTC_MF52D_104F3950);
    info->T_mos2 = (uint16_t)ntc_voltage_to_temperature(voltages[2], NTC_MF52D_104F3950);
    info->T_out = (uint16_t)ntc_voltage_to_temperature(voltages[3], NTC_SDNT_1608X103F3435);
    info->T_bottom = (uint16_t)ntc_voltage_to_temperature(voltages[4], NTC_CMFB_503F3950);
    // === 谐振电压采集 ===
    info->V_resonant = voltages[5]*1000.0f; // mV
adc_transtion_complete = false;
// DMA_ClearFlag(DMA1_FLAG_TC5);
#if NTC_DEBUG
    LOG_I("NTC SIDE Temp: %.1f°C", (float)info->T_side);
    LOG_I("NTC LID Temp: %.1f°C", (float)info->T_lid);
    LOG_I("NTC BOTTOM Temp: %.1f°C", (float)info->T_bottom);
    LOG_I("NTC MOS1 Temp: %.1f°C", (float)info->T_mos1);
    LOG_I("NTC MOS2 Temp: %.1f°C", (float)info->T_mos2);
    LOG_I("NTC OUT Temp: %.1f°C", (float)info->T_out);
    LOG_I("NTC BASE Temp: %.1f°C", (float)info->T_base);
    LOG_I("Resonant Voltage: %.3fV",  info->V_resonant);
#endif
}
}
void ntc_init(void)
{
// adc_config_single(ADC1);
    adc_anychannel_dma(adc_channel, sizeof(adc_channel)/sizeof(adc_channel[0]), (uint16_t*)adc_buffer, DMA_SAMPLES*adc_channel_count,DMA1_Channel1);
    #if NTC_DEBUG
    LOG_I("ADC1 Single Conversion Mode Configured");
    #endif
}
// =========================================================
// 主任务线程
// =========================================================
void thread_adc_task_entry(void *parameter)
{
    ntc_init();
    while (1)
    {
        ntc_read_rice_info(&rice_info);
        LOG_I("NTC Bottom Temp: %d °C", rice_info.T_bottom);
        LOG_I("SDNT Lid Temp: %d °C", rice_info.T_lid);
        LOG_I("SDNT Side Temp: %d °C", rice_info.T_side);
        rt_thread_mdelay(100);
    }
}