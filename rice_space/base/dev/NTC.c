#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "platform.h"


#define LOG_TAG     "adc"
#define LOG_LVL     ELOG_LVL_VERBOSE

#define ADC_MAX_VALUE   4095u
#define VREF            3.3f

#define MR_WINDOW       5       // 中值去尖峰窗口
#define AVG_WINDOW      5       // 滚动平均窗口
#define ALPHA           0.25f   // IIR低通系数
#define REL_THRESH      0.12f   // 去尖峰相对阈值

// =========================================================
// NTC 枚举类型
// =========================================================
typedef enum {
    NTC_CMFB_503F3950 = 0,          // 50kΩ @25°C, B=3950K, 100k上拉
    NTC_SDNT_1608X103F3435,         // 10kΩ @25°C, B=3435K, 10k上拉
    NTC_TYPE_COUNT
} ntc_type_t;

// =========================================================
// R-T 表结构
// =========================================================
typedef struct {
    int8_t  temp;     // 温度 (°C)
    float   r_kohm;   // 电阻 (kΩ)
} rt_point_t;

// =========================================================
// NTC 配置结构
// =========================================================
typedef struct {
    float               r_fixed;      // 上拉电阻 (Ω)
    const rt_point_t*   table;
    uint16_t            table_size;
    const char*         name;
} ntc_config_t;

// =========================================================
// R-T 数据表
// =========================================================
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

// =========================================================
// NTC 配置表
// =========================================================
const ntc_config_t ntc_configs[NTC_TYPE_COUNT] = {
    [NTC_CMFB_503F3950] = {
        .r_fixed     = 100000.0f,
        .table       = cmfb_table,
        .table_size  = sizeof(cmfb_table)/sizeof(cmfb_table[0]),
        .name        = "CMFB 503F3950 (50kΩ, 100k pull-up)"
    },
    [NTC_SDNT_1608X103F3435] = {
        .r_fixed     = 10000.0f,
        .table       = sdnt_table,
        .table_size  = sizeof(sdnt_table)/sizeof(sdnt_table[0]),
        .name        = "SDNT1608X103F3435 (10kΩ, 10k pull-up)"
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



// =========================================================
// 滤波结构体定义
// =========================================================
typedef struct {
    uint16_t  mr_buf[MR_WINDOW];
    uint8_t   mr_idx;
    bool      mr_full;

    uint16_t  avg_buf[AVG_WINDOW];
    uint8_t   avg_idx;
    bool      avg_full;
    uint32_t  avg_sum;

    float     iir;
    bool      initialized;
} combined_filter_t;

// =============================
// 滤波器函数
// =============================
static void combined_filter_init(combined_filter_t *f, uint16_t init_sample)
{
    memset(f, 0, sizeof(*f));
    for (int i = 0; i < MR_WINDOW; i++) f->mr_buf[i] = init_sample;
    for (int i = 0; i < AVG_WINDOW; i++) f->avg_buf[i] = init_sample;
    f->mr_full = f->avg_full = true;
    f->avg_sum = (uint32_t)init_sample * AVG_WINDOW;
    f->iir = init_sample;
    f->initialized = true;
}

static uint16_t median_of_buf(uint16_t *buf, int n)
{
    uint16_t tmp[MR_WINDOW];
    for (int i = 0; i < n; i++) tmp[i] = buf[i];
    for (int i = 1; i < n; i++) {
        uint16_t key = tmp[i];
        int j = i - 1;
        while (j >= 0 && tmp[j] > key) { tmp[j+1] = tmp[j]; j--; }
        tmp[j+1] = key;
    }
    return tmp[n/2];
}

static float combined_filter_update(combined_filter_t *f, uint16_t sample)
{
    if (!f->initialized)
        combined_filter_init(f, sample);

    f->mr_buf[f->mr_idx++] = sample;
    if (f->mr_idx >= MR_WINDOW) { f->mr_idx = 0; f->mr_full = true; }
    uint16_t median = median_of_buf(f->mr_buf, MR_WINDOW);

    float diff = fabsf((float)sample - (float)median);
    uint16_t used = (diff > (REL_THRESH * median) && diff > 8.0f) ? median : sample;

    f->avg_sum -= f->avg_buf[f->avg_idx];
    f->avg_buf[f->avg_idx] = used;
    f->avg_sum += used;
    f->avg_idx++;
    if (f->avg_idx >= AVG_WINDOW) { f->avg_idx = 0; f->avg_full = true; }

    uint8_t count = f->avg_full ? AVG_WINDOW : f->avg_idx;
    float avg = (float)f->avg_sum / (float)count;

    f->iir = ALPHA * avg + (1.0f - ALPHA) * f->iir;

    return (f->iir * VREF) / ADC_MAX_VALUE;
}

// =========================================================
// 外部传感器函数声明
// =========================================================
extern void gn1650_demo(uint16_t freq);
extern uint32_t INA226_GetBusVoltage(void);
extern uint32_t INA226_GetCurrent(void);
extern uint32_t INA226_GetPower(void);

// =========================================================
// 滤波器状态
// =========================================================
static combined_filter_t filt_ch12, filt_ch11, filt_ch6;

static void adc_filter_init(void)
{
    uint16_t v1 = adc_read_single(ADC1, ADC_Channel_12);
    uint16_t v2 = adc_read_single(ADC1, ADC_Channel_11);
    uint16_t v3 = adc_read_single(ADC1, ADC_Channel_6);
    combined_filter_init(&filt_ch12, v1);
    combined_filter_init(&filt_ch11, v2);
    combined_filter_init(&filt_ch6, v3);
}

// =========================================================
// 独立函数：采集并更新温度信息
// =========================================================
void ntc_read_rice_info(rice_information_t *info)
{
    float vcc_adc = adc_read_single(ADC1, ADC_Channel_VoltReference);
    float vcc = vcc_adc * VREF / ADC_MAX_VALUE;

    uint16_t raw12 = adc_read_single(ADC1, ADC_Channel_12);
    uint16_t raw11 = adc_read_single(ADC1, ADC_Channel_11);
    uint16_t raw6  = adc_read_single(ADC1, ADC_Channel_6);

    float v_ch12 = combined_filter_update(&filt_ch12, raw12) * (vcc / VREF);
    float v_ch11 = combined_filter_update(&filt_ch11, raw11) * (vcc / VREF);
    float v_ch6  = combined_filter_update(&filt_ch6,  raw6)  * (vcc / VREF);

    info->T_bottom = (uint16_t)ntc_voltage_to_temperature(v_ch6,  NTC_CMFB_503F3950);
    info->T_lid    = (uint16_t)ntc_voltage_to_temperature(v_ch11, NTC_SDNT_1608X103F3435);
    info->T_side   = (uint16_t)ntc_voltage_to_temperature(v_ch12, NTC_SDNT_1608X103F3435);
    
    #if NTC_DEBUG   
        LOG_I("NTC Bottom Temp: %d °C", rice_info.T_bottom);
        LOG_I("SDNT Lid Temp: %d °C", rice_info.T_lid);
        LOG_I("SDNT Side Temp: %d °C", rice_info.T_side);
    #endif

}

void ntc_init(void)
{
	adc_config_single(ADC1);
    #if NTC_DEBUG
    LOG_I("ADC1 Single Conversion Mode Configured");
    #endif
    adc_filter_init();
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

        gn1650_demo(rice_info.T_bottom);

        rt_thread_mdelay(100);
    }
}
