////////////////////////////////////////////////////////////////////////////////
/// @file     hal_gpio.h
/// @author   AE TEAM
/// @brief    THIS FILE CONTAINS ALL THE FUNCTIONS PROTOTYPES FOR THE GPIO
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
#ifndef __HAL_GPIO_H
#define __HAL_GPIO_H

// Files includes
#include "types.h"
#include "reg_gpio.h"

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup MM32_Hardware_Abstract_Layer
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @defgroup GPIO_HAL
/// @brief GPIO HAL modules
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @defgroup GPIO_Exported_Types
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief  Output Maximum frequency selection
////////////////////////////////////////////////////////////////////////////////
typedef enum {
    GPIO_Speed_10MHz = 1,                                                       ///< Maximum speed is 10MHz
    GPIO_Speed_20MHz = 2,                                                       ///< Maximum speed is 20MHz
    GPIO_Speed_50MHz = 3                                                        ///< Maximum speed is 50MHz
} GPIOSpeed_TypeDef;


////////////////////////////////////////////////////////////////////////////////
/// @brief  Configuration Mode enumeration
////////////////////////////////////////////////////////////////////////////////
typedef enum {
    GPIO_Mode_AIN       = 0x00,  ///< Analog input
    GPIO_Mode_FLOATING  = 0x04,  ///< Floating input
    GPIO_Mode_IPD       = 0x28,  ///< Pull down input
    GPIO_Mode_IPU       = 0x48,  ///< Pull up input
    GPIO_Mode_Out_OD    = 0x14,  ///< Universal open drain output
    GPIO_Mode_Out_PP    = 0x10,  ///< Universal push-pull output
    GPIO_Mode_AF_OD     = 0x1C,  ///< Multiplex open drain output
    GPIO_Mode_AF_PP     = 0x18   ///< Multiplexed push-pull output
} GPIOMode_TypeDef;

////////////////////////////////////////////////////////////////////////////////
/// @brief  Bit_SET and Bit_RESET enumeration
////////////////////////////////////////////////////////////////////////////////
typedef enum {
    Bit_RESET = 0,  ///< bit reset
    Bit_SET         ///< bit set
} BitAction;

////////////////////////////////////////////////////////////////////////////////
/// @brief  GPIO Init structure definition
////////////////////////////////////////////////////////////////////////////////
typedef struct {
    u16               GPIO_Pin;    ///< GPIO_Pin
    GPIOSpeed_TypeDef GPIO_Speed;  ///< GPIO_Speed
    GPIOMode_TypeDef  GPIO_Mode;   ///< GPIO_Mode
} GPIO_InitTypeDef;

/// @}

////////////////////////////////////////////////////////////////////////////////
/// @defgroup GPIO_Exported_Constants
/// @{

#define GPIO_Speed_2MHz GPIO_Speed_20MHz

#define GPIO_Pin_0                  (0x0001U)                                   ///< Pin 0 selected
#define GPIO_Pin_1                  (0x0002U)                                   ///< Pin 1 selected
#define GPIO_Pin_2                  (0x0004U)                                   ///< Pin 2 selected
#define GPIO_Pin_3                  (0x0008U)                                   ///< Pin 3 selected
#define GPIO_Pin_4                  (0x0010U)                                   ///< Pin 4 selected
#define GPIO_Pin_5                  (0x0020U)                                   ///< Pin 5 selected
#define GPIO_Pin_6                  (0x0040U)                                   ///< Pin 6 selected
#define GPIO_Pin_7                  (0x0080U)                                   ///< Pin 7 selected
#define GPIO_Pin_8                  (0x0100U)                                   ///< Pin 8 selected
#define GPIO_Pin_9                  (0x0200U)                                   ///< Pin 9 selected
#define GPIO_Pin_10                 (0x0400U)                                   ///< Pin 10 selected
#define GPIO_Pin_11                 (0x0800U)                                   ///< Pin 11 selected
#define GPIO_Pin_12                 (0x1000U)                                   ///< Pin 12 selected
#define GPIO_Pin_13                 (0x2000U)                                   ///< Pin 13 selected
#define GPIO_Pin_14                 (0x4000U)                                   ///< Pin 14 selected
#define GPIO_Pin_15                 (0x8000U)                                   ///< Pin 15 selected
#define GPIO_Pin_All                (0xFFFFU)                                   ///< All pins selected


#define GPIO_AF_0                   (0x00U)                                     ///< Alternative function 0
#define GPIO_AF_1                   (0x01U)                                     ///< Alternative function 1
#define GPIO_AF_2                   (0x02U)                                     ///< Alternative function 2
#define GPIO_AF_3                   (0x03U)                                     ///< Alternative function 3
#define GPIO_AF_4                   (0x04U)                                     ///< Alternative function 4
#define GPIO_AF_5                   (0x05U)                                     ///< Alternative function 5
#define GPIO_AF_6                   (0x06U)                                     ///< Alternative function 6
#define GPIO_AF_7                   (0x07U)                                     ///< Alternative function 7
#define GPIO_PortSourceGPIOA        (0x00U)
#define GPIO_PortSourceGPIOB        (0x01U)
#define GPIO_PortSourceGPIOC        (0x02U)
#define GPIO_PortSourceGPIOD        (0x03U)

#define GPIO_PinSource0             (0x00U)
#define GPIO_PinSource1             (0x01U)
#define GPIO_PinSource2             (0x02U)
#define GPIO_PinSource3             (0x03U)
#define GPIO_PinSource4             (0x04U)
#define GPIO_PinSource5             (0x05U)
#define GPIO_PinSource6             (0x06U)
#define GPIO_PinSource7             (0x07U)
#define GPIO_PinSource8             (0x08U)
#define GPIO_PinSource9             (0x09U)
#define GPIO_PinSource10            (0x0AU)
#define GPIO_PinSource11            (0x0BU)
#define GPIO_PinSource12            (0x0CU)
#define GPIO_PinSource13            (0x0DU)
#define GPIO_PinSource14            (0x0EU)
#define GPIO_PinSource15            (0x0FU)


#define GPIO_AF_CAN_RX_PA11             GPIO_AF_MODE4
#define GPIO_AF_CAN_RX_PB8              GPIO_AF_MODE4
#define GPIO_AF_CAN_RX_PD0              GPIO_AF_MODE0
#define GPIO_AF_CAN_TX_PA12             GPIO_AF_MODE4
#define GPIO_AF_CAN_TX_PB9              GPIO_AF_MODE4
#define GPIO_AF_CAN_TX_PD1              GPIO_AF_MODE0
#define GPIO_AF_COMP1_OUT_PA0           GPIO_AF_MODE7
#define GPIO_AF_COMP1_OUT_PA11          GPIO_AF_MODE7
#define GPIO_AF_COMP1_OUT_PA6           GPIO_AF_MODE7
#define GPIO_AF_COMP2_OUT_PA12          GPIO_AF_MODE7
#define GPIO_AF_COMP2_OUT_PA2           GPIO_AF_MODE7
#define GPIO_AF_COMP2_OUT_PA7           GPIO_AF_MODE7
#define GPIO_AF_CRS_SYNC_PA8            GPIO_AF_MODE4
#define GPIO_AF_CRS_SYNC_PD15           GPIO_AF_MODE0
#define GPIO_AF_CRS_SYNC_PF0            GPIO_AF_MODE0
#define GPIO_AF_EVENTOUT_PA6            GPIO_AF_MODE6
#define GPIO_AF_EVENTOUT_PA7            GPIO_AF_MODE6
#define GPIO_AF_EVENTOUT_PB11           GPIO_AF_MODE0
#define GPIO_AF_EVENTOUT_PB12           GPIO_AF_MODE1
#define GPIO_AF_EVENTOUT_PB2            GPIO_AF_MODE2
#define GPIO_AF_EVENTOUT_PB9            GPIO_AF_MODE3
#define GPIO_AF_EVENTOUT_PC0            GPIO_AF_MODE0
#define GPIO_AF_EVENTOUT_PC1            GPIO_AF_MODE0
#define GPIO_AF_EVENTOUT_PC2            GPIO_AF_MODE0
#define GPIO_AF_EVENTOUT_PC3            GPIO_AF_MODE0
#define GPIO_AF_EVENTOUT_PC4            GPIO_AF_MODE0
#define GPIO_AF_EVENTOUT_PE0            GPIO_AF_MODE1
#define GPIO_AF_EVENTOUT_PE1            GPIO_AF_MODE1
#define GPIO_AF_EVENTOUT_PF2            GPIO_AF_MODE0
#define GPIO_AF_EVENTOUT_PF3            GPIO_AF_MODE0
#define GPIO_AF_I2C1_SCL_PA11           GPIO_AF_MODE5
#define GPIO_AF_I2C1_SCL_PA9            GPIO_AF_MODE4
#define GPIO_AF_I2C1_SCL_PB6            GPIO_AF_MODE1
#define GPIO_AF_I2C1_SCL_PB8            GPIO_AF_MODE1
#define GPIO_AF_I2C1_SDA_PA10           GPIO_AF_MODE4
#define GPIO_AF_I2C1_SDA_PA12           GPIO_AF_MODE5
#define GPIO_AF_I2C1_SDA_PB7            GPIO_AF_MODE1
#define GPIO_AF_I2C1_SDA_PB9            GPIO_AF_MODE1
#define GPIO_AF_I2C2_SCL_PB10           GPIO_AF_MODE1
#define GPIO_AF_I2C2_SCL_PB13           GPIO_AF_MODE5
#define GPIO_AF_I2C2_SDA_PB11           GPIO_AF_MODE1
#define GPIO_AF_I2C2_SDA_PB14           GPIO_AF_MODE5
#define GPIO_AF_LPTIM1_OUT_PB14         GPIO_AF_MODE3
#define GPIO_AF_LPTIM1_TRIG_PB13        GPIO_AF_MODE3
#define GPIO_AF_LPUART1_RX_PA5          GPIO_AF_MODE3
#define GPIO_AF_LPUART1_TX_PA4          GPIO_AF_MODE3
#define GPIO_AF_MCO_PA8                 GPIO_AF_MODE0
#define GPIO_AF_MCO_PA9                 GPIO_AF_MODE5
#define GPIO_AF_SPI1_MISO_I2S1_MCK_PA6  GPIO_AF_MODE0
#define GPIO_AF_SPI1_MISO_I2S1_MCK_PB4  GPIO_AF_MODE0
#define GPIO_AF_SPI1_MISO_I2S1_MCK_PE14 GPIO_AF_MODE1
#define GPIO_AF_SPI1_MOSI_I2S1_SD_PA7   GPIO_AF_MODE0
#define GPIO_AF_SPI1_MOSI_I2S1_SD_PB5   GPIO_AF_MODE0
#define GPIO_AF_SPI1_MOSI_I2S1_SD_PE15  GPIO_AF_MODE1
#define GPIO_AF_SPI1_NSS_I2S1_WS_PA4    GPIO_AF_MODE0
#define GPIO_AF_SPI1_NSS_I2S1_WS_PE12   GPIO_AF_MODE1
#define GPIO_AF_SPI1_NSS_PA15           GPIO_AF_MODE0
#define GPIO_AF_SPI1_SCK_I2S1_CK_PA5    GPIO_AF_MODE0
#define GPIO_AF_SPI1_SCK_I2S1_CK_PB3    GPIO_AF_MODE0
#define GPIO_AF_SPI1_SCK_I2S1_CK_PE13   GPIO_AF_MODE1
#define GPIO_AF_SPI2_MISO_I2S_MCK_PC2   GPIO_AF_MODE1
#define GPIO_AF_SPI2_MISO_I2S2_MCK_PB14 GPIO_AF_MODE0
#define GPIO_AF_SPI2_MISO_PD3           GPIO_AF_MODE1
#define GPIO_AF_SPI2_MOSI_I2S_SD_PC3    GPIO_AF_MODE1
#define GPIO_AF_SPI2_MOSI_I2S2_SD_PB15  GPIO_AF_MODE0
#define GPIO_AF_SPI2_MOSI_PD4           GPIO_AF_MODE1
#define GPIO_AF_SPI2_NSS_I2S2_WS_PB12   GPIO_AF_MODE0
#define GPIO_AF_SPI2_NSS_I2S2_WS_PB9    GPIO_AF_MODE5
#define GPIO_AF_SPI2_NSS_I2S2_WS_PD0    GPIO_AF_MODE1
#define GPIO_AF_SPI2_SCK_I2S2_CK_PB13   GPIO_AF_MODE0
#define GPIO_AF_SPI2_SCK_I2S2_MCK_PD1   GPIO_AF_MODE1
#define GPIO_AF_SPI2_SCK_PB10           GPIO_AF_MODE5
#define GPIO_AF_SWCLK_PA14              GPIO_AF_MODE0
#define GPIO_AF_SWDIO_PA13              GPIO_AF_MODE0
#define GPIO_AF_TIM1_BKIN_PA6           GPIO_AF_MODE2
#define GPIO_AF_TIM1_BKIN_PB12          GPIO_AF_MODE2
#define GPIO_AF_TIM1_BKIN_PE15          GPIO_AF_MODE0
#define GPIO_AF_TIM1_CH1_PA8            GPIO_AF_MODE2
#define GPIO_AF_TIM1_CH1_PE9            GPIO_AF_MODE0
#define GPIO_AF_TIM1_CH1N_PA7           GPIO_AF_MODE2
#define GPIO_AF_TIM1_CH1N_PB13          GPIO_AF_MODE2
#define GPIO_AF_TIM1_CH1N_PE8           GPIO_AF_MODE0
#define GPIO_AF_TIM1_CH2_PA9            GPIO_AF_MODE2
#define GPIO_AF_TIM1_CH2_PE11           GPIO_AF_MODE0
#define GPIO_AF_TIM1_CH2N_PB0           GPIO_AF_MODE2
#define GPIO_AF_TIM1_CH2N_PB14          GPIO_AF_MODE2
#define GPIO_AF_TIM1_CH2N_PE10          GPIO_AF_MODE0
#define GPIO_AF_TIM1_CH3_PA10           GPIO_AF_MODE2
#define GPIO_AF_TIM1_CH3_PE13           GPIO_AF_MODE0
#define GPIO_AF_TIM1_CH3N_PB1           GPIO_AF_MODE2
#define GPIO_AF_TIM1_CH3N_PB15          GPIO_AF_MODE2
#define GPIO_AF_TIM1_CH3N_PE12          GPIO_AF_MODE0
#define GPIO_AF_TIM1_CH4_PA11           GPIO_AF_MODE2
#define GPIO_AF_TIM1_CH4_PE14           GPIO_AF_MODE0
#define GPIO_AF_TIM1_ETR_PA12           GPIO_AF_MODE2
#define GPIO_AF_TIM1_ETR_PE7            GPIO_AF_MODE0
#define GPIO_AF_TIM14_CH1_PA4           GPIO_AF_MODE4
#define GPIO_AF_TIM14_CH1_PA7           GPIO_AF_MODE4
#define GPIO_AF_TIM14_CH1_PB1           GPIO_AF_MODE0
#define GPIO_AF_TIM15_BKIN_PA9          GPIO_AF_MODE0
#define GPIO_AF_TIM15_BKIN_PB12         GPIO_AF_MODE5
#define GPIO_AF_TIM15_CH1_PA2           GPIO_AF_MODE0
#define GPIO_AF_TIM15_CH1_PB14          GPIO_AF_MODE1
#define GPIO_AF_TIM15_CH1_PF9           GPIO_AF_MODE0
#define GPIO_AF_TIM15_CH1N_PA1          GPIO_AF_MODE5
#define GPIO_AF_TIM15_CH1N_PB15         GPIO_AF_MODE3
#define GPIO_AF_TIM15_CH2_PA3           GPIO_AF_MODE0
#define GPIO_AF_TIM15_CH2_PB15          GPIO_AF_MODE1
#define GPIO_AF_TIM15_CH2_PF10          GPIO_AF_MODE0
#define GPIO_AF_TIM16_BKIN_PB5          GPIO_AF_MODE2
#define GPIO_AF_TIM16_CH1_PA6           GPIO_AF_MODE5
#define GPIO_AF_TIM16_CH1_PB8           GPIO_AF_MODE2
#define GPIO_AF_TIM16_CH1_PE0           GPIO_AF_MODE0
#define GPIO_AF_TIM16_CH1N_PB6          GPIO_AF_MODE2
#define GPIO_AF_TIM17_BKIN_PA10         GPIO_AF_MODE0
#define GPIO_AF_TIM17_BKIN_PB4          GPIO_AF_MODE5
#define GPIO_AF_TIM17_CH1_PA7           GPIO_AF_MODE5
#define GPIO_AF_TIM17_CH1_PB9           GPIO_AF_MODE2
#define GPIO_AF_TIM17_CH1_PE1           GPIO_AF_MODE0
#define GPIO_AF_TIM17_CH1N_PB7          GPIO_AF_MODE2
#define GPIO_AF_TIM2_CH1_TIM2_ETR_PA0   GPIO_AF_MODE2
#define GPIO_AF_TIM2_CH1_TIM2_ETR_PA15  GPIO_AF_MODE2
#define GPIO_AF_TIM2_CH1_TIM2_ETR_PA5   GPIO_AF_MODE2
#define GPIO_AF_TIM2_CH2_PA1            GPIO_AF_MODE2
#define GPIO_AF_TIM2_CH2_PB3            GPIO_AF_MODE2
#define GPIO_AF_TIM2_CH3_PA2            GPIO_AF_MODE2
#define GPIO_AF_TIM2_CH3_PB10           GPIO_AF_MODE2
#define GPIO_AF_TIM2_CH4_PA3            GPIO_AF_MODE2
#define GPIO_AF_TIM2_CH4_PB11           GPIO_AF_MODE2
#define GPIO_AF_TIM3_CH1_PA6            GPIO_AF_MODE1
#define GPIO_AF_TIM3_CH1_PB4            GPIO_AF_MODE1
#define GPIO_AF_TIM3_CH1_PC6            GPIO_AF_MODE0
#define GPIO_AF_TIM3_CH1_PE3            GPIO_AF_MODE0
#define GPIO_AF_TIM3_CH2_PA7            GPIO_AF_MODE1
#define GPIO_AF_TIM3_CH2_PB5            GPIO_AF_MODE1
#define GPIO_AF_TIM3_CH2_PC7            GPIO_AF_MODE0
#define GPIO_AF_TIM3_CH2_PE4            GPIO_AF_MODE0
#define GPIO_AF_TIM3_CH3_PB0            GPIO_AF_MODE1
#define GPIO_AF_TIM3_CH3_PC8            GPIO_AF_MODE0
#define GPIO_AF_TIM3_CH3_PE5            GPIO_AF_MODE0
#define GPIO_AF_TIM3_CH4_PB1            GPIO_AF_MODE1
#define GPIO_AF_TIM3_CH4_PC9            GPIO_AF_MODE0
#define GPIO_AF_TIM3_CH4_PE6            GPIO_AF_MODE0
#define GPIO_AF_TIM3_ETR_PD2            GPIO_AF_MODE0
#define GPIO_AF_TIM3_ETR_PE2            GPIO_AF_MODE0
#define GPIO_AF_UART1_CTS_PA11          GPIO_AF_MODE1
#define GPIO_AF_UART1_RTS_PA12          GPIO_AF_MODE1
#define GPIO_AF_UART1_RX_PA10           GPIO_AF_MODE1
#define GPIO_AF_UART1_RX_PA9            GPIO_AF_MODE3
#define GPIO_AF_UART1_RX_PB7            GPIO_AF_MODE0
#define GPIO_AF_UART1_TX_PA10           GPIO_AF_MODE3
#define GPIO_AF_UART1_TX_PA9            GPIO_AF_MODE1
#define GPIO_AF_UART1_TX_PB6            GPIO_AF_MODE0
#define GPIO_AF_UART2_CTS_PA0           GPIO_AF_MODE1
#define GPIO_AF_UART2_CTS_PD3           GPIO_AF_MODE0
#define GPIO_AF_UART2_RTS_PA1           GPIO_AF_MODE1
#define GPIO_AF_UART2_RTS_PD4           GPIO_AF_MODE0
#define GPIO_AF_UART2_RX_PA15           GPIO_AF_MODE1
#define GPIO_AF_UART2_RX_PA3            GPIO_AF_MODE1
#define GPIO_AF_UART2_RX_PD6            GPIO_AF_MODE0
#define GPIO_AF_UART2_TX_PA14           GPIO_AF_MODE1
#define GPIO_AF_UART2_TX_PA2            GPIO_AF_MODE1
#define GPIO_AF_UART2_TX_PD5            GPIO_AF_MODE0
#define GPIO_AF_UART3_CTS_PA6           GPIO_AF_MODE4
#define GPIO_AF_UART3_CTS_PB13          GPIO_AF_MODE4
#define GPIO_AF_UART3_CTS_PD11          GPIO_AF_MODE0
#define GPIO_AF_UART3_RTS_PB1           GPIO_AF_MODE4
#define GPIO_AF_UART3_RTS_PB14          GPIO_AF_MODE4
#define GPIO_AF_UART3_RTS_PD12          GPIO_AF_MODE0
#define GPIO_AF_UART3_RTS_PD2           GPIO_AF_MODE1
#define GPIO_AF_UART3_RX_PB11           GPIO_AF_MODE4
#define GPIO_AF_UART3_RX_PC11           GPIO_AF_MODE1
#define GPIO_AF_UART3_RX_PC5            GPIO_AF_MODE1
#define GPIO_AF_UART3_RX_PD9            GPIO_AF_MODE0
#define GPIO_AF_UART3_TX_PB10           GPIO_AF_MODE4
#define GPIO_AF_UART3_TX_PC10           GPIO_AF_MODE1
#define GPIO_AF_UART3_TX_PC4            GPIO_AF_MODE1
#define GPIO_AF_UART3_TX_PD8            GPIO_AF_MODE0
#define GPIO_AF_UART4_CTS_PB7           GPIO_AF_MODE4
#define GPIO_AF_UART4_RTS_PA15          GPIO_AF_MODE4
#define GPIO_AF_UART4_RX_PA1            GPIO_AF_MODE4
#define GPIO_AF_UART4_RX_PC11           GPIO_AF_MODE0
#define GPIO_AF_UART4_TX_PA0            GPIO_AF_MODE4
#define GPIO_AF_UART4_TX_PC10           GPIO_AF_MODE0

/// @}

////////////////////////////////////////////////////////////////////////////////
/// @defgroup GPIO_Exported_Variables
/// @{

#ifdef _HAL_GPIO_C_

#define GLOBAL
#else
#define GLOBAL extern
#endif



#undef GLOBAL

/// @}

////////////////////////////////////////////////////////////////////////////////
/// @defgroup GPIO_Exported_Functions
/// @{
void GPIO_DeInit(GPIO_TypeDef* gpio);
void GPIO_AFIODeInit(void);
void GPIO_Init(GPIO_TypeDef* gpio, GPIO_InitTypeDef* init_struct);
void GPIO_StructInit(GPIO_InitTypeDef* init_struct);
void GPIO_SetBits(GPIO_TypeDef* gpio, u16 pin);
void GPIO_ResetBits(GPIO_TypeDef* gpio, u16 pin);
void GPIO_WriteBit(GPIO_TypeDef* gpio, u16 pin, BitAction value);
void GPIO_Write(GPIO_TypeDef* gpio, u16 value);
void GPIO_PinLock(GPIO_TypeDef* gpio, u16 pin, FunctionalState state);
void GPIO_PinLockConfig(GPIO_TypeDef* gpio, u16 pin);
bool GPIO_ReadInputDataBit(GPIO_TypeDef* gpio, u16 pin);
bool GPIO_ReadOutputDataBit(GPIO_TypeDef* gpio, u16 pin);

u16 GPIO_ReadInputData(GPIO_TypeDef* gpio);
u16 GPIO_ReadOutputData(GPIO_TypeDef* gpio);


void GPIO_PinAFConfig(GPIO_TypeDef* gpio, u8 pin, u8 alternate_function);

void exGPIO_PinAFConfig(GPIO_TypeDef* gpio, u16 pin, s32 remap, s8 alternate_function);


/// @}

/// @}

/// @}

////////////////////////////////////////////////////////////////////////////////
#endif // __HAL_GPIO_H 
////////////////////////////////////////////////////////////////////////////////
