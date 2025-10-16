////////////////////////////////////////////////////////////////////////////////
/// @file     hal_lptim.h
/// @author   AE TEAM
/// @brief    THIS FILE CONTAINS ALL THE FUNCTIONS PROTOTYPES FOR THE TIM
///           FIRMWARE LIBRARY.
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
#ifndef __HAL_LPTIM_H
#define __HAL_LPTIM_H

// Files includes
#include "types.h"
#include "reg_lptim.h"

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup MM32_Hardware_Abstract_Layer
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @defgroup LPTIM_HAL
/// @brief TIM HAL modules
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @defgroup LPTIM_Exported_Types
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief LPTIM_CLK_SOURCE_TypeDef
/// @anchor LPTIM_CLK_SOURCE_TypeDef
typedef enum {
    LPTIM_LSE_Source                    = RCC_CFGR2_LPTIMSEL_LSE,               ///< LPTIM clock source select LSE
    LPTIM_LSI_Source                    = RCC_CFGR2_LPTIMSEL_LSI,               ///< LPTIM clock source select LSI
    LPTIM_PCLK_Source                   = RCC_CFGR2_LPTIMSEL_PCLK               ///< LPTIM clock source select PCLK
} LPTIM_CLK_SOURCE_TypeDef;



////////////////////////////////////////////////////////////////////////////////
/// @brief LPTIM_Count_Mode_TypeDef
/// @anchor LPTIM_Count_Mode_TypeDef
typedef enum {
    LPTIM_CONTINUOUS_COUNT_Mode         = 0,                                    ///< LPTIM Continuous Count mode
    LPTIM_SINGLE_COUNT_Mode             = LPTCFG_MODE,                          ///< LPTIM Single Count mode
} LPTIM_Count_Mode_TypeDef;

////////////////////////////////////////////////////////////////////////////////
/// @brief LPTIM_OUTPUT_Mode_TypeDef
/// @anchor LPTIM_OUTPUT_Mode_TypeDef
typedef enum {
    LPTIM_NORMAL_WAV_Mode               = LPTCFG_TMODE_0,                       ///< LPTIM Normal TIM Wave output mode
    LPTIM_PULSE_TRIG_Mode               = LPTCFG_TMODE_1,                       ///< LPTIM pulse trig count output mode
    LPTIM_TIMEOUT_Mode                  = LPTCFG_TMODE_3                        ///< LPTIM time out output mode
} LPTIM_OUTPUT_Mode_TypeDef;

////////////////////////////////////////////////////////////////////////////////
/// @brief LPTIM_PWMOUT_Mode_TypeDef
/// @anchor LPTIM_PWMOUT_Mode_TypeDef
typedef enum {
    LPTIM_CycleSquareOutput_Mode        = 0,                                    ///< LPTIM Cycle Square Wave output mode
    LPTIM_AdjustPwmOutput_Mode          = LPTCFG_PWM,                           ///< LPTIM Pulse Wave Modified output mode
} LPTIM_PWMOUT_Mode_TypeDef;

////////////////////////////////////////////////////////////////////////////////
/// @brief LPTIM_COMPARE_Polarity_TypeDef
/// @anchor LPTIM_COMPARE_Polarity_TypeDef
typedef enum {
    LPTIM_Positive_Wave                 = 0,                                    ///< LPTIM Compare Match Wave mode(positive)
    LPTIM_Negative_Wave                 = LPTCFG_POLARITY,                      ///< LPTIM Compare Match Wave mode(negative)
} LPTIM_COMPARE_Polarity_TypeDef;

////////////////////////////////////////////////////////////////////////////////
/// @brief LPTIM_TrigSourceConfig_TypeDef
/// @anchor LPTIM_TrigSourceConfig_TypeDef
typedef enum {
    LPTIM_External_PIN_Trig             = 0,                                    ///< LPTIM use external pin as trig source
    LPTIM_COMP_OUT_Trig                 = LPTCFG_TRIGSEL,                       ///< LPTIM out COPM output as trig source
} LPTIM_TrigSourceConfig_TypeDef;

////////////////////////////////////////////////////////////////////////////////
/// @brief LPTIM_TrigEdgeConfig_TypeDef
/// @anchor LPTIM_TrigEdgeConfig_TypeDef
typedef enum {
    LPTIM_ExInputUpEdge                 = LPTCFG_TRIGCFG_Rise,                  ///< LPTIM use external signal raise edge trig
    LPTIM_ExInputDownEdge               = LPTCFG_TRIGCFG_Fall,                  ///< LPTIM use external signal fall edge trig
    LPTIM_ExInputUpDownEdge             = LPTCFG_TRIGCFG_Both,                  ///< LPTIM use external signal raise and fall edge trig
} LPTIM_TrigEdgeConfig_TypeDef;

////////////////////////////////////////////////////////////////////////////////
/// @brief LPTIM_CLOCK_DIV_TypeDef
/// @anchor LPTIM_CLOCK_DIV_TypeDef
typedef enum {
    LPTIM_CLK_DIV1                      = LPTCFG_DIVSEL_1,                      ///< LPTIM  Counter Clock div 1
    LPTIM_CLK_DIV2                      = LPTCFG_DIVSEL_2,                      ///< LPTIM  Counter Clock div 2
    LPTIM_CLK_DIV4                      = LPTCFG_DIVSEL_4,                      ///< LPTIM  Counter Clock div 4
    LPTIM_CLK_DIV8                      = LPTCFG_DIVSEL_8,                      ///< LPTIM  Counter Clock div 8
    LPTIM_CLK_DIV16                     = LPTCFG_DIVSEL_16,                     ///< LPTIM  Counter Clock div 16
    LPTIM_CLK_DIV32                     = LPTCFG_DIVSEL_32,                     ///< LPTIM  Counter Clock div 32
    LPTIM_CLK_DIV64                     = LPTCFG_DIVSEL_64,                     ///< LPTIM  Counter Clock div 64
    LPTIM_CLK_DIV128                    = LPTCFG_DIVSEL_128,                    ///< LPTIM  Counter Clock div 128
} LPTIM_CLOCK_DIV_TypeDef;

////////////////////////////////////////////////////////////////////////////////
/// @brief LPTIM_FilterTrig_TypeDef
/// @anchor LPTIM_FilterTrig_TypeDef
typedef enum {
    LPTIM_FilterTrig_Disable            = 0,                                    ///< LPTIM Filter Trig disable
    LPTIM_FilterTrig_Enable             = LPTCFG_FLTEN,                         ///< LPTIM Filter Trig enable
} LPTIM_FilterTrig_TypeDef;


////////////////////////////////////////////////////////////////////////////////
/// @brief  TIM Time Base Init structure definition
/// @note   This structure is used with all lptim.
////////////////////////////////////////////////////////////////////////////////
typedef struct {
    LPTIM_CLK_SOURCE_TypeDef ClockSource;                                       ///< Specifies the clock source of the LPTIM.
    LPTIM_Count_Mode_TypeDef CountMode;                                         ///< Specifies the Count mode
    LPTIM_OUTPUT_Mode_TypeDef OutputMode;                                       ///< Specifies the Output Mode
    LPTIM_PWMOUT_Mode_TypeDef Waveform;                                         ///< Specifies the PWM wave form.
    LPTIM_COMPARE_Polarity_TypeDef Polarity;                                    ///< Specifies the Output Polarity
    LPTIM_CLOCK_DIV_TypeDef ClockDivision;                                      ///< Specifies the clock divide.
} LPTIM_TimeBaseInit_TypeDef;


/// @}

////////////////////////////////////////////////////////////////////////////////
/// @defgroup LPTIM_Exported_Variables
/// @{
#ifdef _HAL_LPTIM_C_
#define GLOBAL

#else
#define GLOBAL extern
#endif

#undef GLOBAL
/// @}

////////////////////////////////////////////////////////////////////////////////
/// @defgroup LPTIM_Exported_Functions
/// @{

////////////////////////////////////////////////////////////////////////////////
//=================  TimeBase management  ======================================
void LPTIM_DeInit(LPTIM_TypeDef* lptim);
void LPTIM_Cmd(LPTIM_TypeDef* lptim, FunctionalState state);
void LPTIM_CLKConfig(LPTIM_TypeDef* lptim, LPTIM_CLK_SOURCE_TypeDef lptim_clk_src);
void LPTIM_TimeBaseStructInit(LPTIM_TimeBaseInit_TypeDef* init_struct);
void LPTIM_TimeBaseInit(LPTIM_TypeDef* lptim, LPTIM_TimeBaseInit_TypeDef* init_struct);
//void LPTIM_CounterModeConfig(LPTIM_TypeDef* lptim, TIMCOUNTMODE_Typedef counter_mode);
void LPTIM_SetClockDivision(LPTIM_TypeDef* lptim, LPTIM_CLOCK_DIV_TypeDef clock_div);
void LPTIM_SetCounter(LPTIM_TypeDef* lptim, u16 counter);
u32 LPTIM_GetCounter(LPTIM_TypeDef* lptim);
void LPTIM_SetCompare(LPTIM_TypeDef* lptim, u16 compare);
u16 LPTIM_GetCompare(LPTIM_TypeDef* lptim);
void LPTIM_SetTarget(LPTIM_TypeDef* lptim, u16 target);
u16 LPTIM_GetTarget(LPTIM_TypeDef* lptim);

void LPTIM_ITConfig(LPTIM_TypeDef* lptim, u32 it, FunctionalState state);
ITStatus LPTIM_GetITStatus(LPTIM_TypeDef* lptim, u32 it);
void LPTIM_ClearITPendingBit(LPTIM_TypeDef* lptim,  u32 it);

void LPTIM_InputTrigEdgeConfig(LPTIM_TypeDef* lptim, LPTIM_TrigEdgeConfig_TypeDef edgeselect);
void LPTIM_InputTrigSourceConfig(LPTIM_TypeDef* lptim, LPTIM_TrigSourceConfig_TypeDef source);
/// @}

/// @}

/// @}

////////////////////////////////////////////////////////////////////////////////
#endif // __HAL_LPTIM_H
////////////////////////////////////////////////////////////////////////////////
