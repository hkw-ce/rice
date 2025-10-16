////////////////////////////////////////////////////////////////////////////////
/// @file     hal_lptim.c
/// @author   AE TEAM
/// @brief    THIS FILE PROVIDES ALL THE LPTIM FIRMWARE FUNCTIONS.
////////////////////////////////////////////////////////////////////////////////
/// @attention
///
/// THE EXISTING FIRMWARE IS ONLY FOR REFERENCE, WHICH IS DESIGNED TO PROVIDE
/// CUSTOMERS WITH CODING INFORMATION ABOUT THEIR PRODUCTS SO THEY CAN SAVE
/// TIME. THEREFORE, MINDMOTION SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT OR
/// CONSEQUENTIAL DAMAGES ABOUT ANY CLAIMS ARISING OUT OF THE CONTENT OF SUCH
/// HARDWARE AND/OR THE USE OF THE CODING INFORMATION CONTAINED HEREIN IN
/// CONNECTION WITH PRODUCTS MADE BY CUSTOMERS.
///
/// <H2><CENTER>&COPY; COPYRIGHT MINDMOTION </CENTER></H2>
////////////////////////////////////////////////////////////////////////////////

// Define to prevent recursive inclusion
#define _HAL_LPTIM_C_

// Files includes
#include "hal_rcc.h"
#include "hal_lptim.h"

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup MM32_Hardware_Abstract_Layer
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup LPTIM_HAL
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup LPTIM_Exported_Functions
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief  Deinitializes the lptim peripheral registers to their default reset values.
/// @param  lptim:  select the LPTIM peripheral.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPTIM_DeInit(LPTIM_TypeDef* lptim)
{
    if(lptim == LPTIM1) {
        RCC_APB2PeriphResetCmd(RCC_APB2ENR_LPTIM1, ENABLE);
        RCC_APB2PeriphResetCmd(RCC_APB2ENR_LPTIM1, DISABLE);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Configures the LPTIM1 clock (LPTIM1 CLK).
///         Once the RTC clock is selected it can be changed unless the
///         Backup domain is reset.
/// @param  rtc_clk_src: specifies the RTC clock source.
///         This parameter can be one of the following values:
/// @arg    LPTIM_LSE_Source : LSE selected as LPTIM1 clock
/// @arg    LPTIM_LSI_Source : LSI selected as LPTIM1 clock
/// @arg    LPTIM_PCLK_Source: PCLK(AHB) selected as LPTIM1 clock
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPTIM_CLKConfig(LPTIM_TypeDef* lptim, LPTIM_CLK_SOURCE_TypeDef lptim_clk_src)
{
    if(lptim == LPTIM1) {
        MODIFY_REG(RCC->CFGR2, RCC_CFGR2_LPTIMSEL_Msk, lptim_clk_src);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Fills each init_struct member with its default value.
/// @param  init_struct : pointer to a LPTIM_TimeBaseInit_TypeDef
///         structure which will be initialized.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPTIM_TimeBaseStructInit(LPTIM_TimeBaseInit_TypeDef* init_struct)
{
    init_struct->ClockSource            = LPTIM_PCLK_Source;
    init_struct->CountMode              = LPTIM_CONTINUOUS_COUNT_Mode;
    init_struct->OutputMode             = LPTIM_NORMAL_WAV_Mode;
    init_struct->Waveform               = LPTIM_CycleSquareOutput_Mode;
    init_struct->Polarity               = LPTIM_Positive_Wave;
    init_struct->ClockDivision          = LPTIM_CLK_DIV1;
}
////////////////////////////////////////////////////////////////////////////////
/// @brief  Initializes the lptim Time Base Unit peripheral according to
///         the specified parameters in the init_struct.
/// @param  lptim: select the LPTIM peripheral.
/// @param  init_struct: pointer to a LPTIM_TimeBaseInit_TypeDef
///         structure that contains the configuration information for the
///         specified LPTIM peripheral.
///         LPTIM_CLK_SOURCE_TypeDef ClockSource;                               ///< Specifies the clock source of the LPTIM.
///         LPTIM_Count_Mode_TypeDef CountMode;                                 ///< Specifies the Count mode
///         LPTIM_OUTPUT_Mode_TypeDef OutputMode;                               ///< Specifies the Output Mode
///         LPTIM_PWMOUT_Mode_TypeDef Waveform;                                 ///< Specifies the PWM wave form.
///         LPTIM_COMPARE_Polarity_TypeDef Polarity;                            ///< Specifies the Output Polarity
///         LPTIM_CLOCK_DIV_TypeDef ClockDivision;                              ///< Specifies the clock divide.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPTIM_TimeBaseInit(LPTIM_TypeDef* lptim, LPTIM_TimeBaseInit_TypeDef* init_struct)
{
    u32 temp = 0;
    RCC->CFGR2 &= ~(RCC_CFGR2_LPTIMSEL_Msk);
    RCC->CFGR2 |= (init_struct->ClockSource)&RCC_CFGR2_LPTIMSEL_Msk;
    temp |= (init_struct->CountMode);
    temp |= (init_struct->OutputMode);
    temp |= (init_struct->Waveform);
    temp |= (init_struct->Polarity);
    temp |= (init_struct->ClockDivision);
    lptim->CFGR = temp;

//    lptim->CFGR |= 0x1 << 3;
//    lptim->CMP = 2000;
//    lptim->TARGET = 4000;
//    lptim->IER = 0x1;
//    lptim->CR = 0x1;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Enables or disables the specified LPTIM peripheral.
/// @param  lptim: where x can be 1 to select the lptim peripheral.
/// @param  state: new state of the lptim peripheral.
///   This parameter can be: ENABLE or DISABLE.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPTIM_Cmd(LPTIM_TypeDef* lptim, FunctionalState state)
{
    (state) ? SET_BIT(lptim->CR, LPTCTRL_LPTEN) : CLEAR_BIT(lptim->CR, LPTCTRL_LPTEN);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Enables or disables the specified LPTIM input trig source.
/// @param  lptim:  select the lptim peripheral.
/// @param  source: LPTIM input trig source.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPTIM_InputTrigEdgeConfig(LPTIM_TypeDef* lptim, LPTIM_TrigEdgeConfig_TypeDef edgeselect)
{
    MODIFY_REG(lptim->CFGR, LPTCFG_TRIGCFG_Msk, edgeselect);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Enables or disables the specified LPTIM input trig source.
/// @param  lptim:  select the lptim peripheral.
/// @param  source: LPTIM input trig source.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPTIM_InputTrigSourceConfig(LPTIM_TypeDef* lptim, LPTIM_TrigSourceConfig_TypeDef source)
{
    MODIFY_REG(lptim->CFGR, LPTCFG_TRIGSEL, source);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Sets the lptim Clock Division value.
/// @param  lptim:  select
///   the LPTIM peripheral.
/// @param  clock_div: specifies the clock division value.
///   This parameter can be one of the following value:
///     @arg LPTIM_CLK_DIV1
///     @arg LPTIM_CLK_DIV2
///     @arg LPTIM_CLK_DIV4
///     @arg LPTIM_CLK_DIV8
///     @arg LPTIM_CLK_DIV16
///     @arg LPTIM_CLK_DIV32
///     @arg LPTIM_CLK_DIV64
///     @arg LPTIM_CLK_DIV128
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPTIM_SetClockDivision(LPTIM_TypeDef* lptim, LPTIM_CLOCK_DIV_TypeDef clock_div)
{
    MODIFY_REG(lptim->CFGR, LPTCFG_DIVSEL_Msk, clock_div);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Enables or disables the specified LPTIM input filter function.
/// @param  lptim:  select the lptim peripheral.
/// @param  state: new state of the LPTIM input filter mode.
///   This parameter can be: ENABLE or DISABLE.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPTIM_InputFilterConfig(LPTIM_TypeDef* lptim, FunctionalState state)
{
    (state) ? SET_BIT(lptim->CFGR, LPTCFG_FLTEN) : CLEAR_BIT(lptim->CFGR, LPTCFG_FLTEN);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Sets the lptim Counter Register value
/// @param  lptim:  select the LPTIM peripheral.
/// @param  counter: specifies the Counter register new value.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPTIM_SetCounter(LPTIM_TypeDef* lptim, u16 counter)
{
    WRITE_REG(lptim->CNT, (u32)counter);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Gets the lptim Counter value.
/// @param  lptim:  select the LPTIM peripheral.
/// @retval Value: Counter Register value.
////////////////////////////////////////////////////////////////////////////////
u32 LPTIM_GetCounter(LPTIM_TypeDef* lptim)
{
    return lptim->CNT;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Sets the lptim Compare Register value
/// @param  lptim:  select the LPTIM peripheral.
/// @param  compare: specifies the Compare register new value.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPTIM_SetCompare(LPTIM_TypeDef* lptim, u16 compare)
{
    WRITE_REG(lptim->CMP, (u32)compare);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Gets the lptim Compare value.
/// @param  lptim:  select the LPTIM peripheral.
/// @retval Value: Compare Register value.
////////////////////////////////////////////////////////////////////////////////
u16 LPTIM_GetCompare(LPTIM_TypeDef* lptim)
{
    return (u16)lptim->CMP;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Sets the lptim target Register value
/// @param  lptim:  select the LPTIM peripheral.
/// @param  target: specifies the target register new value.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPTIM_SetTarget(LPTIM_TypeDef* lptim, u16 target)
{
    WRITE_REG(lptim->TARGET, (u32)target);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Gets the lptim target value.
/// @param  lptim:  select the LPTIM peripheral.
/// @retval Value: target Register value.
////////////////////////////////////////////////////////////////////////////////
u16 LPTIM_GetTarget(LPTIM_TypeDef* lptim)
{
    return (u16)lptim->CNT;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Enables or disables the specified LPTIM interrupts.
/// @param  lptim:  select the lptim peripheral.
/// @param  it: specifies the LPTIM interrupts sources to be enabled or disabled.
///   This parameter can be any combination of the following values:
///     @arg LPTIE_COMPIE: LPTIM COMPARE Interrupt Enable Bit
///     @arg LPTIE_TRIGIE: LPTIM Ext Trig Interrupt Enable Bit
///     @arg LPTIE_OVIE  : LPTIM Overflow Interrupt Enable Bit
/// @param  state: new state of the LPTIM interrupts.
///   This parameter can be: ENABLE or DISABLE.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPTIM_ITConfig(LPTIM_TypeDef* lptim, u32 it, FunctionalState state)
{
    (state) ? SET_BIT(lptim->IER, it) : CLEAR_BIT(lptim->IER, it);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Checks whether the LPTIM interrupt has occurred or not.
/// @param  lptim:  select the LPTIM peripheral.
/// @param  it: specifies the LPTIM interrupt source to check.
///   This parameter can be one of the following values:
///     @arg LPTIF_COMPIF: LPTIM Compare Interrupt Flag
///     @arg LPTIF_TRIGIF: LPTIM Trig Interrupt Flag
///     @arg LPTIF_OVIF  : LPTIM Counter Overflow Interrupt Flag
/// @retval State: The new state of the LPTIM_IT(SET or RESET).
////////////////////////////////////////////////////////////////////////////////
ITStatus LPTIM_GetITStatus(LPTIM_TypeDef* lptim, u32 it)
{
    return ( (lptim->ISR & it)  ? SET : RESET);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Clears the lptim's interrupt pending bits, write 1 to clear bit.
/// @param  lptim:  select the LPTIM peripheral.
/// @param  it: specifies the pending bit to clear by write 1 to clear.
///   This parameter can be any combination of the following values:
///     @arg LPTIF_COMPIF: LPTIM Compare Interrupt Flag
///     @arg LPTIF_TRIGIF: LPTIM Trig Interrupt Flag
///     @arg LPTIF_OVIF  : LPTIM Counter Overflow Interrupt Flag
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPTIM_ClearITPendingBit(LPTIM_TypeDef* lptim,  u32 it)
{
    lptim->ISR = it;
}

/// @}

/// @}

/// @}
