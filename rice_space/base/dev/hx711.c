#include "hx711.h"
#include "hx711Config.h"
#include "platform.h" 
#if (_HX711_USE_FREERTOS == 1)
#include "cmsis_os.h"
#define hx711_delay(x)    osDelay(x)
#else
#define hx711_delay(x)    rt_hw_us_delay(x)
#endif

#define HAL_GetTick() rt_tick_get()
static hx711_t hx711_dev;
#define hx711_clk_pin  GPIO_Pin_7
#define hx711_dat_pin  GPIO_Pin_6
#define hx711_clk_gpio GPIOF
#define hx711_dat_gpio GPIOF

/*
 * @brief 写 GPIO 引脚电平的封装（平台抽象）
 *
 * 注意：工程中直接使用 HAL_GPIO_WritePin 的平台映射版本，
 *       在不同平台上应保证该宏/函数的行为与 HAL 的定义一致。
 */
#define GPIO_PIN_SET   1
#define GPIO_PIN_RESET 0

#define HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState)  \
  do{ \
    if(PinState) \
      GPIO_SetBits(GPIOx, GPIO_Pin); \
    else \
      GPIO_ResetBits(GPIOx, GPIO_Pin); \
  }while(0)


  #define HAL_GPIO_ReadPin(GPIOx, GPIO_Pin)  \
  GPIO_ReadInputDataBit(GPIOx, GPIO_Pin)
/*
 * @brief 读取 GPIO 引脚电平的封装（平台抽象）
 *
 * 注意：工程中直接使用 HAL_GPIO_ReadPin 的平台映射版本，
 *       在不同平台上应保证该宏/函数的行为与 HAL 的定义一致。
 */

//#############################################################################################
/*
 * @brief 微秒级延时函数（用于 HX711 的时序要求）
 *
 * HX711 在时序上要求时钟脉冲之间保持一定的短延时。
 * 这里实现为一个短延迟循环，调用处使用 hx711_delay_us()
 * 以满足数据采集时的最小脉冲宽度要求。
 *
 * 注意：该函数实现依赖于 RTOS 环境的毫秒/延时函数，
 *       故延时的精度取决于底层实现（_HX711_DELAY_US_LOOP）。
 */
void hx711_delay_us(void)
{
 rt_hw_us_delay(_HX711_DELAY_US_LOOP);
}
//#############################################################################################
/**
 * @brief 获取互斥锁（软件自旋锁）
 *
 * 等待直到 hx711->lock 变为 0，然后设置为 1 表示占用。
 * 该锁用于防止同一时刻多个上下文并发访问 HX711（GPIO 引脚）。
 *
 * 参数:
 *   hx711 - 指向设备结构体，包含 lock 字段
 *
 * 注意：这是一个简单的自旋锁实现，会在锁不可用时阻塞当前线程，
 *       因此应避免在中断上下文或长时间占用锁的场景中调用。
 */
void hx711_lock(hx711_t *hx711)
{
  while (hx711->lock)
    hx711_delay(1);
  hx711->lock = 1;      
}
//#############################################################################################
/**
 * @brief 释放互斥锁
 *
 * 将 hx711->lock 置 0，允许其他上下文获取锁。
 */
void hx711_unlock(hx711_t *hx711)
{
  hx711->lock = 0;
}
//#############################################################################################
/**
 * @brief 初始化 HX711 设备
 *
 * 该函数配置 GPIO 引脚、复位时钟线并读取两次数据以确保 HX711
 * 从上电/复位状态进入稳定工作状态。
 *
 * 参数:
 *   hx711    - 设备结构体指针
 *   clk_gpio - 时钟线的 GPIO 端口
 *   clk_pin  - 时钟线的引脚
 *   dat_gpio - 数据线的 GPIO 端口
 *   dat_pin  - 数据线的引脚
 *
 * 行为/注意事项:
 * - 函数使用软件锁保护，避免并发访问引脚
 * - 先将 CLK 拉高再拉低以确保设备进入已知状态，然后读取两次数据丢弃
 *   初始不稳定值（常见做法），保证后续读取的稳定性
 */
void hx711_init(hx711_t *hx711, GPIO_TypeDef *clk_gpio, uint16_t clk_pin, GPIO_TypeDef *dat_gpio, uint16_t dat_pin)
{
  hx711_lock(hx711);
  hx711->clk_gpio = clk_gpio;
  hx711->clk_pin = clk_pin;
  hx711->dat_gpio = dat_gpio;
  hx711->dat_pin = dat_pin;
  gpio_config(clk_gpio, clk_pin, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
  gpio_config(dat_gpio, dat_pin, GPIO_Mode_FLOATING, GPIO_Speed_50MHz);
 
  /* 复位时钟信号，等待设备稳定 */
  HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_SET);
  hx711_delay(10);
  HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_RESET);
  hx711_delay(10);  
  /* 丢弃前两次读取，以清除上电/复位时的残留数据 */
  hx711_value(hx711);
  hx711_value(hx711);
  hx711_unlock(hx711); 
}
//#############################################################################################
/**
 * @brief 读取一次 HX711 原始 24-bit 数据并返回有符号值（扩展到 32-bit）
 *
 * 流程：
 * 1. 等待 DAT 引脚变为低电平，表示数据已准备好。超时（150ms）返回 0。
 * 2. 通过产生 24 个时钟上升沿从设备读出 24 位原始数据（MSB 先）。
 * 3. 将读取到的 24-bit 数据转换为带符号的 32-bit 值，并在末尾产生一次额外的脉冲
 *    来设置增益/通道（这里保持默认时序）。
 *
 * 参数:
 *   hx711 - 设备结构体指针，包含 CLK/DAT 引脚信息
 *
 * 返回值:
 *   读取到的 32-bit 原始值（已对 24-bit 数据做符号扩展）；
 *   若等待 DAT 超时则返回 0（请注意：0 也可能是合法读数，调用处应考虑这一点）。
 *
 * 时序注意事项：
 * - 每个数据位在时钟上升沿有效，要求在上下沿之间提供短延时（使用 hx711_delay_us()）。
 */
int32_t hx711_value(hx711_t *hx711)
{
  uint32_t data = 0;
  uint32_t  startTime = HAL_GetTick();
  /* 等待数据准备好（DAT 变低），超时保护 */
  while(HAL_GPIO_ReadPin(hx711->dat_gpio, hx711->dat_pin) == GPIO_PIN_SET)
  {
    hx711_delay(1);
//    if(HAL_GetTick() - startTime > 150)
//      return 0;
  }
  /* 读取 24 个数据位，MSB 先 */
  for(int8_t i=0; i<24 ; i++)
  {
    HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_SET);   
    hx711_delay_us();
	data = data << 1;    
    HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_RESET);
    hx711_delay_us();

    if(HAL_GPIO_ReadPin(hx711->dat_gpio, hx711->dat_pin) == GPIO_PIN_SET)
      data ++;
  }
  /* 将 24-bit 原始值做符号位扩展（HX711 输出是 24-bit 补码） */
  data = data ^ 0x800000; 
  /* 产生额外脉冲以完成增益/通道配置（保留原实现） */
  HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_SET);   
  hx711_delay_us();
  HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_RESET);
  hx711_delay_us();
  return data;    
}
//#############################################################################################
/**
 * @brief 多次读取取平均（缓和噪声）
 *
 * 该函数在内部获取锁以避免并发访问 HX711。对连续的 sample 次读取求和并取平均。
 *
 * 参数:
 *   hx711  - 设备结构体指针
 *   sample - 采样次数（建议 >1）
 *
 * 返回值:
 *   平均后的 32-bit 原始值（整型），注意溢出风险已通过使用 64-bit 累加规避。
 */
int32_t hx711_value_ave(hx711_t *hx711, uint16_t sample)
{
  hx711_lock(hx711);
  int64_t  ave = 0;
  for(uint16_t i=0 ; i<sample ; i++)
  {
    ave += hx711_value(hx711);
    hx711_delay(5);
  }
  int32_t answer = (int32_t)(ave / sample);
  hx711_unlock(hx711);
  return answer;
}
//#############################################################################################
/**
 * @brief 校零（Tare）
 *
 * 通过多次读取计算无载情况下的平均原始值，并将其保存为 offset，后续
 * 计算重量时会从原始值中减去该 offset。
 *
 * 参数:
 *   hx711  - 设备结构体指针
 *   sample - 采样次数
 */
void hx711_tare(hx711_t *hx711, uint16_t sample)
{
//  hx711_lock(hx711);
  int64_t  ave = 0;
  for(uint16_t i=0 ; i<sample ; i++)
  {
    ave += hx711_value(hx711);
    hx711_delay(5);
  }
  hx711->offset = (int32_t)(ave / sample);
//  hx711_unlock(hx711);
}
//#############################################################################################
/**
 * @brief 根据已知负载进行标定
 *
 * 参数:
 *   hx711      - 设备结构体指针
 *   noload_raw  - 无载时的原始读数（raw）
 *   load_raw    - 已知载荷时的原始读数（raw）
 *   scale       - 已知载荷的实际量（单位由用户决定，例如克）
 *
 * 行为：
 *   计算 coef = (load_raw - noload_raw) / scale，后续计算重量时使用
 *   (raw - offset) / coef 得到物理单位下的重量。
 */
void hx711_calibration(hx711_t *hx711, int32_t noload_raw, int32_t load_raw, float scale)
{
  hx711_lock(hx711);
  hx711->offset = noload_raw;
  hx711->coef = (load_raw - noload_raw) / scale;  
  hx711_unlock(hx711);
}
//#############################################################################################
/**
 * @brief 读取并返回转换后的物理重量值
 *
 * 参数:
 *   hx711  - 设备结构体指针
 *   sample - 采样次数（用于平均化以降低噪声）
 *
 * 返回值:
 *   以校准时使用的单位表示的重量值（例如克）。函数使用 hx711->offset
 *   与 hx711->coef 进行转换： (raw - offset) / coef
 */
float hx711_weight(hx711_t *hx711, uint16_t sample)
{
  hx711_lock(hx711);
  int64_t  ave = 0;
  for(uint16_t i=0 ; i<sample ; i++)
  {
    ave += hx711_value(hx711);
    hx711_delay(5);
  }
  int32_t data = (int32_t)(ave / sample);
//  LOG_I("data ave = %d\n",data);
  float answer =  (data - hx711->offset) / hx711->coef;
//  rt_kprintf("answer value = %f",answer);
  hx711_unlock(hx711);
  return answer;
}
//#############################################################################################
/**
 * @brief 设置标定系数
 *
 * 将用户计算或实验得到的系数写入设备结构体，后续调用
 * `hx711_weight` 时会使用该系数进行原始值到物理单位的换算。
 *
 * 参数:
 *   hx711 - 设备结构体指针
 *   coef  - 标定系数，通常为 (raw_load - raw_noload) / known_weight
 */
void hx711_coef_set(hx711_t *hx711, float coef)
{
  hx711->coef = coef;  
}
//#############################################################################################
/**
 * @brief 获取当前标定系数
 *
 * 返回用于换算的系数（coef），注意在使用前应保证已正确标定或手动设置该系数。
 *
 * 参数:
 *   hx711 - 设备结构体指针
 *
 * 返回:
 *   当前保存的标定系数（float）
 */
float hx711_coef_get(hx711_t *hx711)
{
  return hx711->coef;  
}
//#############################################################################################
void hx711_power_down(hx711_t *hx711)
{
  /*
   * 将时钟线拉高保持超过 60us（设备手册要求）可进入掉电模式。
   * 这里通过先拉低再拉高的方式保证时序（与平台 HAL_GPIO 的行为一致）。
   */
  HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_SET);
  hx711_delay(1);  
}
//#############################################################################################
void hx711_power_up(hx711_t *hx711)
{
  /**
   * @brief 使能 HX711（从掉电模式恢复）
   *
   * 将时钟线拉低即可让 HX711 退出掉电并准备好响应数据读取。
   */
  HAL_GPIO_WritePin(hx711->clk_gpio, hx711->clk_pin, GPIO_PIN_RESET);
}
//#############################################################################################

// 一键初始化 + 写死校准参数
void hx711_setup_final(void)
{
    hx711_init(&hx711_dev, GPIOF, GPIO_Pin_7, GPIOF, GPIO_Pin_6);
    rt_thread_mdelay(5000);  // 预热 5 秒

    // 写死你的最终校准参数
    hx711_dev.offset = 8501363L;   // 真实 0g 值
    hx711_dev.coef   = 495.40f;    // 微调后精确 50.000g
}

//test

// 实时称重任务
static void weight_task(void *parameter)
{
    hx711_setup_final();
    LOG_I("HX711 已校准，开机即用！");

    while (1)
    {
        float weight = hx711_weight(&hx711_dev, 20);  // 20次平均
        LOG_I("重量: %.3f g", weight);
        rt_thread_mdelay(500);
    }
}

// 注册命令
MSH_CMD_EXPORT(weight_task, 启动称重);

 static void hx711_test_entry(void *parameter)
 {
  
 LOG_I("=== HX711 终极校准（50g 高精度）===");

     hx711_init(&hx711_dev, GPIOF, GPIO_Pin_7, GPIOF, GPIO_Pin_6);
     LOG_I("HX711 初始化完成");

     // 1. 预热 5 秒（减少漂移）
     LOG_I("预热 5 秒，稳定传感器...");
     rt_thread_mdelay(5000);

     // 2. 去皮（无负载，30次平均）
     LOG_I("请确保秤上无物 → 去皮...");
     rt_thread_mdelay(2000);
     hx711_tare(&hx711_dev, 30);
     LOG_I("去皮完成，offset = %ld", (long)hx711_dev.offset);

     // 3. 放 50g
     LOG_I("请放上 50g 砝码 → 3秒后读取...");
     rt_thread_mdelay(3000);
     int32_t raw_50g = hx711_value_ave(&hx711_dev, 30);  // 30次平均！
     LOG_I("50g 原始值 = %ld", (long)raw_50g);

     int32_t delta = raw_50g - hx711_dev.offset;
     if (delta < 1000) {
         LOG_E("增量太小！");
         return;
     }

     float coef = (float)delta / 50.0f;
     hx711_dev.coef = coef;

     LOG_I("高精度校准完成！");
     LOG_I("   delta = %ld", (long)delta);
     LOG_I("   coef  = %.6f", coef);
     LOG_I("   每克  = %.2f 计数", 1.0f/coef);

     // 4. 高精度验证（20次平均）
     LOG_I("高精度验证中...");
     for (int i = 0; i < 5; i++) {
         float w = hx711_weight(&hx711_dev, 20);  // 20次平均
         LOG_I("  [%d] 重量 = %.3f g", i+1, w);
         rt_thread_mdelay(1000);
     }

     LOG_I("请取下砝码 → 验证归零...");
     rt_thread_mdelay(3000);
     float w0 = hx711_weight(&hx711_dev, 20);
     LOG_I("无负载 = %.3f g", w0);

     LOG_I("=== 校准成功！可写死 coef ===");
 }
 MSH_CMD_EXPORT(hx711_test_entry,hx711test);



//static void hx711_test_entry(void *parameter)
//{
//    LOG_I("=== HX711 终极校准（50g 高精度）===");
//    hx711_init(&hx711_dev, GPIOF, GPIO_Pin_7, GPIOF, GPIO_Pin_6);
//    LOG_I("HX711 初始化完成");

//    LOG_I("预热 5 秒，稳定传感器...");
//    rt_thread_mdelay(5000);

//    LOG_I("请确保秤上无物 → 去皮...");
//    rt_thread_mdelay(2000);
//    hx711_tare(&hx711_dev, 30);
//    LOG_I("去皮完成，offset = %ld", (long)hx711_dev.offset);

//    LOG_I("请放上 50g 砝码 → 3秒后读取...");
//    rt_thread_mdelay(3000);
//    int32_t raw_50g = hx711_value_ave(&hx711_dev, 30);
//    LOG_I("50g 原始值 = %ld", (long)raw_50g);

//    int32_t delta = raw_50g - hx711_dev.offset;
//    if (delta < 1000) {
//        LOG_E("增量太小！校准失败");
//        return;  // 失败后退出线程
//    }

//    float coef = (float)delta / 50.0f;
//    hx711_dev.coef = coef;

//    LOG_I("高精度校准完成！");
//    LOG_I(" delta = %ld", (long)delta);
//    LOG_I(" coef = %.6f", coef);
//    LOG_I(" 每克 = %.2f 计数", 1.0f / coef);

//    LOG_I("高精度验证中...");
//    for (int i = 0; i < 5; i++) {
//        float w = hx711_weight(&hx711_dev, 20);
//        LOG_I(" [%d] 重量 = %.3f g", i + 1, w);
//        rt_thread_mdelay(1000);
//    }

//    LOG_I("请取下砝码 → 验证归零...");
//    rt_thread_mdelay(3000);
//    float w0 = hx711_weight(&hx711_dev, 20);
//    LOG_I("无负载 = %.3f g", w0);

//    LOG_I("=== 校准成功！开始自主重量监测（每秒一次） ===");

//    /* 自主监测循环 */
//    while (1)
//    {
//        float weight = hx711_weight(&hx711_dev, 20);
//        rt_kprintf("[AUTO] 重量 = %.2f g\n\r", weight);
//        rt_thread_mdelay(1000);
//    }
//}
rt_thread_t hx711_tid = RT_NULL;
/*======================================================================
 *  HX711 任务控制命令：start | stop
 * ------------------------------------------------------------- */
static void hx711_task_cmd(int argc, char **argv)
{
    if (argc < 2)
    {
        rt_kprintf("Usage: %s {start|stop}\n", argv[0]);
        return;
    }

    if (rt_strcmp(argv[1], "start") == 0)
    {
        if (hx711_tid != RT_NULL)
        {
            rt_kprintf("HX711 task already running.\n");
            return;
        }

        hx711_tid = rt_thread_create("hx711",
                                     hx711_test_entry,
                                     RT_NULL,
                                     512,                          // 栈大小
                                     RT_THREAD_PRIORITY_MAX / 2,    // 优先级
                                     20);                           // 时间片
        if (hx711_tid)
        {
            rt_thread_startup(hx711_tid);
            rt_kprintf("HX711 task started.\n");
        }
        else
        {
            rt_kprintf("Failed to create HX711 thread!\n");
        }
    }
    else if (rt_strcmp(argv[1], "stop") == 0)
    {
        if (hx711_tid)
        {
            rt_thread_delete(hx711_tid);
            hx711_tid = RT_NULL;
            rt_kprintf("HX711 task stopped.\n");
        }
        else
        {
            rt_kprintf("HX711 task not running.\n");
        }
    }
    else
    {
        rt_kprintf("Unknown command: %s\n", argv[1]);
    }
}
MSH_CMD_EXPORT(hx711_task_cmd, HX711 background task: start|stop);