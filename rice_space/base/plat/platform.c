
/* Define to prevent recursive inclusion */
#define _PLATFORM_C_

/* Files include */

#include "platform.h"

/* Private typedef ****************************************************************************************************/

/* Private define *****************************************************************************************************/

/* Private macro ******************************************************************************************************/

/* Private variables **************************************************************************************************/

/* Private functions **************************************************************************************************/

void systick_init(void)
{
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);

  if (SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000))
  {
    while (1)
    {
    }
  }

  NVIC_SetPriority(SysTick_IRQn, 0x0);
}

void delay_ms(uint32_t Millisecond)
{
  PLATFORM_DelayTick = Millisecond;

  while (0 != PLATFORM_DelayTick)
  {
  }
}

void console_init(uint32_t Baudrate)
{
#if (SHELL_UART_PORT == USE_UART1)
  uart_config(SHELL_UART,
              Baudrate,
              GPIOA,
              GPIO_Pin_9, GPIO_Pin_10,
              GPIO_PinSource9, GPIO_PinSource10,
              GPIO_AF_7,
              IT_NULL,
              UART1_IRQn);
#elif (SHELL_UART_PORT == USE_UART2)
  uart_config(SHELL_UART,
              Baudrate,
              GPIOA,
              GPIO_Pin_2, GPIO_Pin_3,
              GPIO_PinSource2, GPIO_PinSource3,
              GPIO_AF_1,
              IT_NULL,
              UART2_IRQn);
#else
#error "Unsupported SHELL_UART_PORT definition"
#endif
}

int fputc(int ch, FILE *f)
{
  UART_SendData(SHELL_UART, (uint8_t)ch);

  while (RESET == UART_GetFlagStatus(SHELL_UART, UART_FLAG_TXEPT))
  {
  }

  return (ch);
}

int platform_info_print(void)
{
  RCC_ClocksTypeDef RCC_Clocks;

  RCC_GetClocksFreq(&RCC_Clocks);

  rt_kprintf("\r\n");
  rt_kprintf("\r\nSYSCLK Frequency : %7.3f MHz", (double)RCC_Clocks.SYSCLK_Frequency / (double)1000000.0);
  rt_kprintf("\r\nHCLK   Frequency : %7.3f MHz", (double)RCC_Clocks.HCLK_Frequency / (double)1000000.0);
  rt_kprintf("\r\nPCLK1  Frequency : %7.3f MHz", (double)RCC_Clocks.PCLK1_Frequency / (double)1000000.0);
  rt_kprintf("\r\nPCLK2  Frequency : %7.3f MHz", (double)RCC_Clocks.PCLK2_Frequency / (double)1000000.0);
  rt_kprintf("\r\n\r\n");

  rt_kprintf("\r\n--------------------------------------------\r\n");
  rt_kprintf("software--> %d  or  %s\r\n", SOFTWARE_VERSION, SOFTWARE_VERSION_STR);
  rt_kprintf("hardware--> %d  or  %s\r\n", HARDWARE_VERSION, HARDWARE_VERSION_STR);
  rt_kprintf("\n%s\n", UPDATE_LOG);
  rt_kprintf("\r\n--------------------------------------------\r\n");

  if (SET == RCC_GetFlagStatus(RCC_FLAG_PINRST))
  {
    rt_kprintf("PIN Reset Flag.\r\n");
  }

  if (SET == RCC_GetFlagStatus(RCC_FLAG_PORRST))
  {
    rt_kprintf("POR/PDR Reset Flag.\r\n");
  }

  if (SET == RCC_GetFlagStatus(RCC_FLAG_SFTRST))
  {
    rt_kprintf("Software Reset Flag.\r\n");
  }

  if (SET == RCC_GetFlagStatus(RCC_FLAG_IWDGRST))
  {
    rt_kprintf("Independent Watchdog Reset Flag.\r\n");
  }

  if (SET == RCC_GetFlagStatus(RCC_FLAG_WWDGRST))
  {
    rt_kprintf("Window Watchdog Reset Flag.\r\n");
  }

  RCC_ClearFlag();

  return 0;
}
void platform_init(void)
{
  systick_init();
  //  platform_info_print();

  //	IWDG_Configure(10000);
}

void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t start, now, delta, reload, us_tick;
    start = SysTick->VAL;
    reload = SysTick->LOAD;
    us_tick = 120000000 / 1000000UL;
    do
    {
        now = SysTick->VAL;
        delta = start > now ? start - now : reload + start - now;
    } while (delta < us_tick * us);
}

// void IWDG_Configure(uint32_t time_ms)
// {
//   if (time_ms > 3000)
//     time_ms = 3000; //
//   uint16_t Reload = LSI_VALUE / 1000 * time_ms / 32;

//   RCC_APB1PeriphClockCmd(RCC_APB1ENR_IWDG, ENABLE);

//   RCC_LSICmd(ENABLE);

//   while (RESET == RCC_GetFlagStatus(RCC_FLAG_LSIRDY))
//   {
//   }

//   PVU_CheckStatus();
//   IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
//   IWDG_SetPrescaler(IWDG_Prescaler_32);
//   PVU_CheckStatus();

//   RVU_CheckStatus();
//   IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
//   IWDG_SetReload(Reload);
//   RVU_CheckStatus();

//   IWDG_ReloadCounter();

//   IWDG_Enable();
// }
// INIT_APP_EXPORT(platform_info_print);