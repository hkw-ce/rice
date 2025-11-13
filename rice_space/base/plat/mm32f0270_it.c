/***********************************************************************************************************************
    @file    mm32f0270_it.c
    @author  FAE Team
    @date    08-Oct-2023
    @brief   THIS FILE PROVIDES ALL THE SYSTEM FUNCTIONS.
  **********************************************************************************************************************
    @attention

    <h2><center>&copy; Copyright(c) <2023> <MindMotion></center></h2>

      Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
    following conditions are met:
    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
       the following disclaimer in the documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or
       promote products derived from this software without specific prior written permission.

      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *********************************************************************************************************************/

/* Define to prevent recursive inclusion */
#define _MM32F0270_IT_C_

/* Files include */
#include "platform.h"
#include "mm32f0270_it.h"


/**
 * @addtogroup MM32F0140_LibSamples
 * @{
 */

/**
 * @addtogroup UART
 * @{
 */

/**
 * @addtogroup UART_ReceiverIdleFrame_Interrupt
 * @{
 */

/* Private typedef ****************************************************************************************************/

/* Private define *****************************************************************************************************/

/* Private macro ******************************************************************************************************/

/* Private variables **************************************************************************************************/

/* Private functions **************************************************************************************************/

/***********************************************************************************************************************
 * @brief  This function handles NMI exception
 * @note   none
 * @param  none
 * @retval none
 *********************************************************************************************************************/
void NMI_Handler(void)
{
}

/***********************************************************************************************************************
 * @brief  This function handles Hard Fault exception
 * @note   none
 * @param  none
 * @retval none
 *********************************************************************************************************************/
__WEAK void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/***********************************************************************************************************************
 * @brief  This function handles SVCall exception
 * @note   none
 * @param  none
 * @retval none
 *********************************************************************************************************************/
void SVC_Handler(void)
{
}

/***********************************************************************************************************************
 * @brief  This function handles PendSVC exception
 * @note   none
 * @param  none
 * @retval none
 *********************************************************************************************************************/
__WEAK void PendSV_Handler(void)
{
}
#include "rtthread.h"
extern void rt_os_tick_callback(void);
/***********************************************************************************************************************
 * @brief  This function handles SysTick Handler
 * @note   none
 * @param  none
 * @retval none
 *********************************************************************************************************************/
void SysTick_Handler(void)
{
  if (0 != PLATFORM_DelayTick)
  {
    PLATFORM_DelayTick--;
  }
  rt_os_tick_callback();
}

/***********************************************************************************************************************
 * @brief  This function handles UART2 Handler
 * @note   none
 * @param  none
 * @retval none
 *********************************************************************************************************************/
__WEAK void UART2_IRQHandler(void)
{
}
/***********************************************************************************************************************
 * @brief  This function handles UART2 Handler
 * @note   none
 * @param  none
 * @retval none
 *********************************************************************************************************************/
__WEAK void UART3_IRQHandler(void)
{
}

void TIM3_IRQHandler(void)
{
}
__WEAK void TIM2_IRQHandler(void)
{
}
/**
 * @}
 */
extern bool adc_transtion_complete;
void DMA1_Channel1_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_FLAG_TC1))
    {
        DMA_ClearFlag(DMA1_FLAG_TC1);
        adc_transtion_complete = TRUE;
        // 在此处添加DMA传输完成后的处理代码

    }
}

/**
 * @}
 */

/**
 * @}
 */

/********************************************** (C) Copyright MindMotion **********************************************/

