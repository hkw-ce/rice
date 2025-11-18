/* Define to prevent recursive inclusion */
#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "hal_conf.h"
#include "rtthread.h"
//#include "mm32_soft_i2c.h"
#include "mm32_soft_i2c_v1.h"
#include "ulog.h"
#include <stdio.h>
#include "ina226.h" 
#include "SC7A20.h"
#include "stdlib.h"
 /********************* DEBUG相关的宏定义 *************************************/
#define NTC_DEBUG       0
#define INA226_DEBUG    0

  /**************************** shell color  *************************************/

  /**************************** device enum  *************************************/
  enum MCU_BASE
  {
    UART_IDLE = 0,
    UART_IT,
    IT_NULL
  };


  /********************* IO && Device config ************************************/
#define USER_I2C_V1 
	
#define USE_UART1 1
#define USE_UART2 2
#define USE_UART3 3
#define USE_UART4 4
#define USE_UART5 5
#define USE_UART6 6
#define USE_UART7 7
#define SHELL_UART UART1
#define SHELL_UART_PORT USE_UART1

#define UART_RX_BUF_SIZE 30
  typedef void (*uart_rx_callback_t)(uint8_t ch);                       // 接收字节回调函数类型
  typedef void (*uart_idle_rx_callback_t)(uint8_t *pBuf, uint16_t len); // 空闲接收回调函数类型
  typedef struct
  {
    UART_TypeDef *uart;
    uint8_t rx_buf[UART_RX_BUF_SIZE]; // 接收缓冲区
    volatile uint16_t rx_len;         // 当前接收长度
    volatile uint8_t rx_ok;           // 接收完成标志（IDLE触发后置1)
    uart_rx_callback_t callback;
    uart_idle_rx_callback_t idle_callback;
  } uart_t;


#ifdef USE_UART1
  extern uart_t uart1;
#endif
#ifdef USE_UART2
  extern uart_t uart2;
#endif
#ifdef USE_UART3
  extern uart_t uart3;
#endif
#ifdef USE_UART4
  extern uart_t uart4;
#endif
#ifdef USE_UART6
  extern uart_t uart6;
#endif


/********************* MCU 相关配置信息 ***************************************/
#define MM32_ROM_START ((uint32_t)0x08000000)
#define MM32_ROM_SIZE (64 * 1024)
#define MM32_ROM_END ((uint32_t)(STM32_ROM_START + STM32_ROM_SIZE))

/********************* 版本说明(尽量使用字符串类型的版本) **********************/
#define HARDWARE_VERSION 1
#define SOFTWARE_VERSION 1
#define HARDWARE_VERSION_STR "V1 2025/11/07"
#define SOFTWARE_VERSION_STR "V1 2025/11/07"

/********************* 更新日志 **********************************************/
#define UPDATE_LOG "更新日志 2025/05/16 新建工程框架\n "
#undef EXTERN

#ifdef _PLATFORM_C_
#define EXTERN
#else
#define EXTERN extern
#endif

  EXTERN volatile uint32_t PLATFORM_DelayTick;
  /* Public platform typedef ********************************************************************************************/
  typedef struct
  {
    GPIO_TypeDef *port; // GPIO 端口，例?? GPIOA
    uint16_t pin;       // GPIO 引脚，例?? GPIO_Pin_1

    uint32_t freq_hz; // 模拟PWM频率
    uint8_t duty;     // 占空比百分比 0~100

    uint32_t period_ticks; // 一个PWM周期包含多少个tick
    uint32_t on_ticks;     // 高电平持续多少tick
    uint32_t tick_counter; // 当前tick计数
  } soft_pwm_t;


// =========================================================
// 电饭锅数据结构体
// =========================================================
typedef struct {
    uint16_t     T_bottom;
    uint16_t     T_lid;
    uint16_t     T_side;
    uint16_t     T_mos1;
    uint16_t     T_mos2;
    uint16_t     T_out;
    uint16_t     T_base;
    uint32_t     V_resonant;
    uint32_t     V_supply;
    uint32_t     I_supply;
    uint32_t     P_supply;
    SL_SC7A20_t  gyro;
	uint32_t     W_rice_water
} rice_information_t;
extern rice_information_t rice_info;


  /* Exported functions *************************************************************************************************/
  void app(void);
  void platform_init(void);
  void IWDG_Configure(uint32_t time_ms);
  void soft_pwm_update(soft_pwm_t *pwm);
  // pwm_config
  void soft_pwm_init(soft_pwm_t *pwm,
                     GPIO_TypeDef *port,
                     uint16_t pin,
                     uint16_t freq_hz,
                     uint8_t duty,
                     uint32_t timer_tick_hz);
  void soft_pwm_set_duty(soft_pwm_t *pwm, uint8_t duty);
  void tim2_pwm_channel_config(uint32_t pwm_freq_hz,
                               uint8_t channel,
                               uint8_t duty_percent,
                               GPIO_TypeDef *gpio_port,
                               uint16_t gpio_pin,
                               uint8_t pin_source,
                               uint8_t af);
  // flash_config
  uint8_t flash_write(const uint8_t *bytes, uint32_t length);
  // usart_config
  void console_init(uint32_t Baudrate);
  void uart_config(UART_TypeDef *UARTx,
                   uint32_t Baudrate,
                   GPIO_TypeDef *GPIOx,
                   uint16_t TxPin,
                   uint16_t RxPin,
                   uint8_t TxPinSource,
                   uint8_t RxPinSource,
                   uint8_t GPIO_AF,
                   uint8_t it_mode,
                   IRQn_Type IRQn);
  void UART_SendBytes(UART_TypeDef *uart, const uint8_t *data, uint32_t length);
  void gpio_config(GPIO_TypeDef *GPIOx, uint16_t pin, GPIOMode_TypeDef mode, GPIOSpeed_TypeDef speed);
  void gpio_toggle(GPIO_TypeDef *GPIOx, uint16_t pin);
  // adc_config
  void adc_config_single(ADC_TypeDef *hadc);
  uint16_t adc_read_single(ADC_TypeDef *hadc, uint8_t channel);
 void adc_anychannel_dma(uint8_t *channels, uint8_t channel_count, uint16_t *adc_buffer, uint16_t buffer_size,DMA_Channel_TypeDef* dma_channel);
void rt_hw_us_delay(rt_uint32_t us);
  // i2c
  void i2c_config(I2C_TypeDef *i2c, uint8_t dev_addr, GPIO_TypeDef *GPIOx, uint16_t sck_pin, uint16_t sda_pin);
  void i2c_tx_polling(I2C_TypeDef *i2c, uint8_t *Buffer, uint8_t Length);
  void i2c_rx_polling(I2C_TypeDef *i2c, uint8_t *Buffer, uint16_t Length);
 void i2c_write_reg(i2c_bus_t *bus, uint8_t dev, uint8_t reg, uint8_t val);  
 uint8_t i2c_read_reg(i2c_bus_t *bus, uint8_t dev, uint8_t reg);
  void default_delay_us(uint32_t us);
  
  
  //v1680
  void thread_vl6180_task_entry(void *parameter);
  //ntc
  void thread_adc_task_entry(void *parameter);  
  void ntc_init(void);
  void ntc_read_rice_info(rice_information_t *info);
//  uint8_t i2c_scan(I2C_TypeDef *i2c, uint8_t *found, uint8_t max_found);

void ina226_read_rice_info(rice_information_t *info);
void INA226_Init(void);

//task
void thread_sample_task_entry(void *parameter);


//语音芯片
void WT588F_Play_Voice(uint8_t voice_addr);
void WT588F_GPIO_Init(void);

  /********************* 平台相关的宏定义 *************************************/

//pwm
#define PSFB_TIM               TIM1
#define PSFB_PWM1              GPIO_Pin_11
#define PSFB_PWM1_PIN_SOURCE   GPIO_PinSource8
#define PSFB_PWM2              GPIO_Pin_12
#define PSFB_PWM2_PIN_SOURCE   GPIO_PinSource1 
#define PSFB_GPIO              GPIOA
#define PSFB_GPIO_AF           GPIO_AF_2

#define PSF_COMP_TIM               TIM1
#define PSFB_DEADTIME              48  // 死区时间，单位：定时器时钟周期
#define PSFB_COMP_CH               1


void TIM1_PWM_Complementary_SingleChannel_Config(uint8_t channel, uint16_t period, uint16_t deadtime);

void full_bridge_init(uint16_t arr,uint16_t psc);
	
//ih_control
void set_ih_power_w(float power_w);
//extern uint8_t rice_type;
//extern uint8_t cook_mode;
//extern uint8_t book_hour;
//extern sys_state_t state;
extern void start_cooking(void);

	
	

#ifdef __cplusplus
}
#endif

#endif /* _PLATFORM_H_ */
/********************************************** (C) Copyright MindMotion **********************************************/
