////////////////////////////////////////////////////////////////////////////////
/// @file     hal_lpuart.h
/// @author   AE TEAM
/// @brief    THIS FILE CONTAINS ALL THE FUNCTIONS PROTOTYPES FOR THE UART
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
#ifndef __HAL_LPUART_H
#define __HAL_LPUART_H

// Files includes
#include "reg_lpuart.h"

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup MM32_Hardware_Abstract_Layer
/// @{

/////////////////////////////////////1///////////////////////////////////////////
/// @defgroup LPUART_HAL
/// @brief UART HAL modules
/// @{


////////////////////////////////////////////////////////////////////////////////
/// @defgroup LPUART_Exported_Types
/// @{
///

////////////////////////////////////////////////////////////////////////////////
/// @brief UART Word Length Enumerate definition
/// @anchor LPUART_Word_Length
////////////////////////////////////////////////////////////////////////////////
typedef enum {
    LPUART_WordLength_7b                = LPUART_LPUCON_DL,
    LPUART_WordLength_8b                = 0
} LPUART_WordLength_TypeDef;

////////////////////////////////////////////////////////////////////////////////
/// @brief UART Stop Bits Enumerate definition
/// @anchor LPUART_Stop_Bits
////////////////////////////////////////////////////////////////////////////////
typedef enum {
    LPUART_StopBits_1                   = 0U,
    LPUART_StopBits_2                   = LPUART_LPUCON_SL,
} LPUART_Stop_Bits_TypeDef;

////////////////////////////////////////////////////////////////////////////////
/// @brief UART Falling Edge Detect Enumerate definition
/// @anchor LPUART_Stop_Bits
////////////////////////////////////////////////////////////////////////////////
typedef enum {
    LPUART_NegativeDectect_Source1      = 0U,
    LPUART_NegativeDectect_Source2      = LPUART_LPUCON_NEDET,
} LPUART_NeDet_TypeDef;

////////////////////////////////////////////////////////////////////////////////
/// @brief UART Parity Enumerate definition
/// @anchor LPUART_Parity
////////////////////////////////////////////////////////////////////////////////
typedef enum {
    LPUART_Parity_No                    = 0U,
    LPUART_Parity_Even                  = LPUART_LPUCON_PAREN,
    LPUART_Parity_Odd                   = LPUART_LPUCON_PAREN | LPUART_LPUCON_PTYP
} LPUART_Parity_TypeDef;

typedef enum {
    LPUART_WakeUp_By_Data               = 0U,  //
    LPUART_WakeUp_By_Match              = LPUART_WKCKE_WKUP_MODE
} LPUART_WakeUp_TypeDef;

typedef enum {
    LPUART_Baudrate_9600                = LPUART_LPUBAUD_BAUD_0,                ///< Baud is 9600 bps
    LPUART_Baudrate_4800                = LPUART_LPUBAUD_BAUD_1,                ///< Baud is 4800 bps
    LPUART_Baudrate_2400                = LPUART_LPUBAUD_BAUD_2,                ///< Baud is 2400 bps
    LPUART_Baudrate_1200                = LPUART_LPUBAUD_BAUD_3,                ///< Baud is 1200 bps
    LPUART_Baudrate_600                 = LPUART_LPUBAUD_BAUD_4,                ///< Baud is 600  bps
    LPUART_Baudrate_300                 = LPUART_LPUBAUD_BAUD_5,                ///< Baud is 300  bps
} LPUART_Baudrate_TypeDef;

////////////////////////////////////////////////////////////////////////////////
/// @brief UART Auto BaudRate definition
////////////////////////////////////////////////////////////////////////////////
typedef enum  {
    LPUART_RecvEvent_Start_Bit          = LPUART_LPUCON_RXEV_0,
    LPUART_RecvEvent_OneByte_Complete   = LPUART_LPUCON_RXEV_1,
    LPUART_RecvEvent_RecvData_Mactched  = LPUART_LPUCON_RXEV_2,
    LPUART_RecvEvent_Falling_Edge       = LPUART_LPUCON_RXEV_3,
} LPUART_Recv_Event_Cfg_TypeDef;
////////////////////////////////////////////////////////////////////////////////
/// @brief UART Init Structure definition
////////////////////////////////////////////////////////////////////////////////
typedef struct {
    LPUART_Baudrate_TypeDef             LPUART_BaudRate;                        ///< This member configures the UART communication baud rate.
    LPUART_WordLength_TypeDef           LPUART_WordLength;                      ///< Specifies the number of data bits transmitted or received in a frame.
    LPUART_Stop_Bits_TypeDef            LPUART_StopBits;                        ///< Specifies the number of stop bits transmitted.
    LPUART_Parity_TypeDef               LPUART_Parity;                          ///< Specifies the parity mode.
    u16                                 LPUART_Clock_Source;                    ///< Specifies Clock Source (0, 1, 2, 3 ... )
    u16                                 LPUART_MDU_Value;                       ///< Specifies the MDU value
    LPUART_NeDet_TypeDef                LPUART_NEDET_Source;                    ///< Specifies Negative Detect Clock Source Ext 32K, or ...
    LPUART_Recv_Event_Cfg_TypeDef       LPUART_RecvEventCfg;                    ///< Specifies wether the receoved event mode
} LPUART_InitTypeDef;

/// @}

////////////////////////////////////////////////////////////////////////////////
/// @defgroup LPUART_Exported_Constants
/// @{

/// @}

////////////////////////////////////////////////////////////////////////////////
/// @defgroup LPUART_Exported_Variables
/// @{
#ifdef _HAL_LPUART_C_

#define GLOBAL
#else
#define GLOBAL extern
#endif

#undef GLOBAL
/// @}

////////////////////////////////////////////////////////////////////////////////
/// @defgroup LPUART_Exported_Functions
/// @{
FlagStatus LPUART_GetFlagStatus(LPUART_TypeDef* lpuart, u16 flag);
ITStatus LPUART_GetITStatus(LPUART_TypeDef* lpuart, u16 it);
u8 LPUART_ReceiveData(LPUART_TypeDef* lpuart);
void LPUART_ClearITPendingBit(LPUART_TypeDef* lpuart, u16 it);
void LPUART_Cmd(LPUART_TypeDef* lpuart, FunctionalState state);
void LPUART_DeInit(LPUART_TypeDef* lpuart);
void LPUART_DMACmd(LPUART_TypeDef* lpuart, FunctionalState state);
void LPUART_Init(LPUART_TypeDef* lpuart, LPUART_InitTypeDef* init_struct);
void LPUART_ITConfig(LPUART_TypeDef* lpuart, u16 it, FunctionalState state);
void LPUART_RX_Cmd(LPUART_TypeDef* lpuart, FunctionalState state);
void LPUART_RX_DMACmd(LPUART_TypeDef* lpuart, FunctionalState state);
void LPUART_SendData(LPUART_TypeDef* lpuart, u8 value);
void LPUART_SetMatchData(LPUART_TypeDef* lpuart, u8 value);
void LPUART_SetRecevieEnable(LPUART_TypeDef* lpuart, FunctionalState state);
void LPUART_SetRXToggle(LPUART_TypeDef* lpuart, FunctionalState state);
void LPUART_SetTransmitEnable(LPUART_TypeDef* lpuart, FunctionalState state);
void LPUART_SetTXToggle(LPUART_TypeDef* lpuart, FunctionalState state);
void LPUART_StructInit(LPUART_InitTypeDef* init_struct);
void LPUART_TX_Cmd(LPUART_TypeDef* lpuart, FunctionalState state);
void LPUART_TX_DMACmd(LPUART_TypeDef* lpuart, FunctionalState state);
void LPUART_WakeUpConfig(LPUART_TypeDef* lpuart, LPUART_WakeUp_TypeDef mode);



/// @}

/// @}

/// @}

////////////////////////////////////////////////////////////////////////////////
#endif // __HAL_LPUART_H
////////////////////////////////////////////////////////////////////////////////
