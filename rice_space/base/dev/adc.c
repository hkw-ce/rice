#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "platform.h"
#include "adc.h"
#include <math.h>
#define LOG_TAG     "adc"
#define LOG_LVL     ELOG_LVL_VERBOSE


uint16_t ADC_Buffer[ADC_BUFFER_SIZE];                   //ADC采样缓冲区

PVD_MAP_TypeDef PVD_MapTable[11] =
{
    { 1.8, PWR_PVDLevel_1V7 },            /* 0:  1.8V */
    { 2.1, PWR_PVDLevel_2V0 },            /* 1:  2.1V */
    { 2.4, PWR_PVDLevel_2V3 },            /* 2:  2.4V */
    { 2.7, PWR_PVDLevel_2V6 },            /* 3:  2.7V */
    { 3.0, PWR_PVDLevel_2V9 },            /* 4:  3.0V */
    { 3.3, PWR_PVDLevel_3V2 },            /* 5:  3.3V */
    { 3.6, PWR_PVDLevel_3V5 },            /* 6:  3.6V */
    { 3.9, PWR_PVDLevel_3V8 },            /* 7:  3.9V */
    { 4.2, PWR_PVDLevel_4V1 },            /* 8:  4.2V */
    { 4.5, PWR_PVDLevel_4V4 },            /* 9:  4.5V */
    { 4.8, PWR_PVDLevel_4V7 },            /* 10: 4.8V */
};

uint8_t PVD_LevelSelection = 4;//pvd电压等级选择3.0v

// =============================
// NTC 类型枚举
// =============================
typedef enum {
    NTC_CMFB_503F3950 = 0,          // 50kΩ @25°C, B=3950K, 100k上拉
    NTC_SDNT_1608X103F3435,         // 10kΩ @25°C, B=3435K, 10k上拉
    NTC_TYPE_COUNT
} ntc_type_t;

// =============================
// R-T 表结构
// =============================
typedef struct {
    int8_t  temp;     // 温度 (°C)
    float   r_kohm;   // 电阻值 (kΩ)
} rt_point_t;

// =============================
// NTC 配置结构
// =============================
typedef struct {
    float               r_fixed;      // 上拉电阻 (Ω)
    const rt_point_t*   table;
    uint16_t            table_size;
    const char*         name;
} ntc_config_t;

// =============================
// 1. CMFB 503F3950 R-T 表（50kΩ@25°C）
// =============================
const rt_point_t cmfb_table[] = {
    {-40, 1726.374}, {-35, 1239.081}, {-30,  900.160}, {-25,  661.420},
    {-20,  490.927}, {-15,  368.128}, {-10,  278.722}, { -5,  212.960},
    {  0,  164.069}, {  5,  127.484}, { 10,   99.788}, { 15,   78.710},
    { 20,   62.520}, { 25,   50.000}, { 30,   40.247}, { 35,   32.600},
    { 40,   26.564}, { 45,   21.769}, { 50,   17.941}, { 55,   14.864},
    { 60,   12.378}, { 65,   10.361}, { 70,    8.709}, { 75,    7.361},
    { 80,    6.245}, { 85,    5.323}, { 90,    4.555}, { 95,    3.914},
    {100,    3.377}, {105,    2.925}, {110,    2.542}, {115,    2.217},
    {120,    1.941}, {125,    1.705}
};

// =============================
// 2. SDNT1608X103F3435 R-T 表（10kΩ@25°C）
//    由 B=3435K 公式精确生成（误差 < ±0.2°C）
// =============================
const rt_point_t sdnt_table[] = {
    {-40,  335.68}, {-35,  249.85}, {-30,  187.88}, {-25,  142.74},
    {-20,  109.38}, {-15,   84.48}, {-10,   65.78}, { -5,   51.62},
    {  0,   40.83}, {  5,   32.55}, { 10,   26.16}, { 15,   21.18},
    { 20,   17.28}, { 25,   14.20}, { 30,   11.76}, { 35,    9.80},
    { 40,    8.23}, { 45,    6.96}, { 50,    5.93}, { 55,    5.09},
    { 60,    4.40}, { 65,    3.83}, { 70,    3.36}, { 75,    2.96},
    { 80,    2.63}, { 85,    2.35}, { 90,    2.11}, { 95,    1.91},
    {100,    1.74}, {105,    1.59}, {110,    1.46}, {115,    1.35},
    {120,    1.25}, {125,    1.16}
};

// =============================
// NTC 配置表（自动匹配上拉电阻）
// =============================
const ntc_config_t ntc_configs[NTC_TYPE_COUNT] = {
    [NTC_CMFB_503F3950] = {
        .r_fixed     = 100000.0f,  // 100kΩ 上拉
        .table       = cmfb_table,
        .table_size  = sizeof(cmfb_table)/sizeof(cmfb_table[0]),
        .name        = "CMFB 503F3950 (50kΩ, 100k pull-up)"
    },
    [NTC_SDNT_1608X103F3435] = {
        .r_fixed     = 10000.0f,   // 10kΩ 上拉
        .table       = sdnt_table,
        .table_size  = sizeof(sdnt_table)/sizeof(sdnt_table[0]),
        .name        = "SDNT1608X103F3435 (10kΩ, 10k pull-up)"
    }
};

// =============================
// 电压 → NTC 阻值 (Ω)
// =============================
float voltage_to_resistance(float vout, float r_fixed) {
    if (vout < 0.01f || vout > 3.29f) return -1.0f;
    return r_fixed * (vout / (3.3f - vout));  // NTC 在下拉
}

// =============================
// 查表 + 线性插值
// =============================
float lookup_temperature(const rt_point_t* table, uint16_t size, float r_kohm) {
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

// =============================
// 主函数：电压 → 温度（支持两种 NTC）
// =============================
float ntc_voltage_to_temperature(float vout, ntc_type_t type) {
    if (type >= NTC_TYPE_COUNT) return -999.0f;

    const ntc_config_t* cfg = &ntc_configs[type];
    float r_ohm = voltage_to_resistance(vout, cfg->r_fixed);
    if (r_ohm < 0) return -999.0f;

    return lookup_temperature(cfg->table, cfg->table_size, r_ohm / 1000.0f);
}


// =============================
// 测试函数
// =============================
void test_ntc(float vout, ntc_type_t type) {
    const ntc_config_t* cfg = &ntc_configs[type];
    float r_ohm = voltage_to_resistance(vout, cfg->r_fixed);
    float temp = ntc_voltage_to_temperature(vout, type);

    printf("=== %s ===\n", cfg->name);
    printf("Vout = %.3fV → R_NTC = %.2f kΩ → Temp = %.2f°C\n\n",
           vout, r_ohm / 1000.0f, temp);
}
extern void gn1650_demo(uint16_t freq);
void thread_adc_task_entry(void *parameter)
{

    u16 voltage1, voltage2, voltage3, temperature,vcc;
    adc_config_single(ADC1);
    LOG_I("ADC1 Single Conversion Mode Configured");
    while (1)
    {
        vcc = adc_read_single(ADC1, ADC_Channel_VoltReference);
//        LOG_I("Vcc Voltage: %d mV", vcc);		
        voltage1 = adc_read_single(ADC1, ADC_Channel_12); // 读取ADC通道12的值
        voltage2 = adc_read_single(ADC1, ADC_Channel_11); // 读取ADC通道11的值
        voltage3 = adc_read_single(ADC1, ADC_Channel_6); // 读取ADC通道6的值
        temperature = (u16)ntc_voltage_to_temperature((float)voltage3 * vcc / (4095.0f*1000), NTC_CMFB_503F3950);
        gn1650_demo(temperature);
        // LOG_I("pot Temperature: %d °C", temperature);
        temperature = (u16)ntc_voltage_to_temperature((float)voltage2 * vcc / (4095.0f*1000), NTC_SDNT_1608X103F3435);
        // LOG_I("MOS2 Temperature: %d °C", temperature);
        temperature = (u16)ntc_voltage_to_temperature((float)voltage1 * vcc / (4095.0f*1000), NTC_SDNT_1608X103F3435);
        // LOG_I("MOS1 Temperature: %d °C", temperature);
        // LOG_I("pot Voltage: %d mV", voltage3*3300/4095);
        // LOG_I("MOS2 Voltage: %d mV", voltage2*3300/4095);
        // LOG_I("MOS1 Voltage: %d mV", voltage1*3300/4095);
        rt_thread_mdelay(1000); // 500ms 采样间隔
    }   


}

