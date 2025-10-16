////////////////////////////////////////////////////////////////////////////////
/// @file     hal_lpuart.c
/// @author   AE TEAM
/// @brief    THIS FILE PROVIDES ALL THE LPUART FIRMWARE FUNCTIONS.
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
#define _HAL_LPUART_C_

// Files includes
#include "hal_rcc.h"
#include "hal_lpuart.h"
// #include "hal_gpio.h"
// #include "hal_dma.h"
////////////////////////////////////////////////////////////////////////////////
/// @addtogroup MM32_Hardware_Abstract_Layer
/// @{

////////////////////////////////////////////////////////////////////////////////
///@addtogroup LPUART_HAL
///@{

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup LPUART_Exported_Functions
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief  Deinitializes the lpuart peripheral registers to their
///         default reset values.
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_DeInit(LPUART_TypeDef* lpuart)
{
    if(LPUART1 == lpuart) {
        exRCC_APB2PeriphReset(RCC_APB2ENR_LPUART1);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Initializes the lpuart peripheral according to the specified
///         parameters in the LPUART_InitStruct .
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @param  init_struct: pointer to a LPUART_InitTypeDef structure
///         that contains the configuration information for the
///         specified LPUART peripheral.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_Init(LPUART_TypeDef* lpuart, LPUART_InitTypeDef* init_struct)
{
    if(init_struct->LPUART_Clock_Source == 1) {
        RCC->CFGR2 |= RCC_CFGR2_LPUARTSEL;
    }
    else {
        RCC->CFGR2 &= ~RCC_CFGR2_LPUARTSEL;
    }
    lpuart->LPUBAUD &= (~LPUART_LPUBAUD_BAUD_Msk);
    lpuart->LPUBAUD |= (init_struct->LPUART_BaudRate);
    lpuart->LPUMODU &= (~LPUART_LPUMODU_MCTL);
    lpuart->LPUMODU |= init_struct->LPUART_MDU_Value;// 0x952 if use LSE32.768k as Clock source

    // LPUART LPUCON Configuration
    MODIFY_REG(lpuart->LPUCON, (LPUART_LPUCON_DL | LPUART_LPUCON_SL | \
                                LPUART_LPUCON_PAREN | LPUART_LPUCON_PTYP | \
                                LPUART_LPUCON_RXEV | LPUART_LPUCON_NEDET), \
               ((u32)init_struct->LPUART_WordLength) | ((u32)init_struct->LPUART_StopBits) | \
               ((u32)init_struct->LPUART_Parity) | ((u32)init_struct->LPUART_RecvEventCfg) | \
               ((u32)init_struct->LPUART_NEDET_Source));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Fills each LPUART_InitStruct member with its default value.
/// @param  init_struct: pointer to a LPUART_InitTypeDef structure
///         which will be initialized.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_StructInit(LPUART_InitTypeDef* init_struct)
{

    // LPUART_InitStruct members default value
    init_struct->LPUART_Clock_Source    = 0;
    init_struct->LPUART_BaudRate        = LPUART_Baudrate_9600;
    init_struct->LPUART_WordLength      = LPUART_WordLength_8b;
    init_struct->LPUART_StopBits        = LPUART_StopBits_1;
    init_struct->LPUART_Parity          = LPUART_Parity_No;
    init_struct->LPUART_MDU_Value       = 0x952;
    init_struct->LPUART_RecvEventCfg    = LPUART_RecvEvent_Start_Bit;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Enables or disables the specified LPUART Tx and Rx.
/// @param  lpuart: Select the LPUART peripheral.
/// @param  state: new state of the lpuart peripheral.
///         This parameter can be: ENABLE or DISABLE.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_Cmd(LPUART_TypeDef* lpuart, FunctionalState state)
{
    if (state != DISABLE) { //tx/rx enable
        do
        {
            SET_BIT(lpuart->LPUEN, (LPUART_LPUEN_TXEN | LPUART_LPUEN_RXEN));
            __NOP();
            __NOP();
            __NOP();
        }while(READ_BIT(lpuart->LPUEN, (LPUART_LPUEN_TXEN | LPUART_LPUEN_RXEN)) != (LPUART_LPUEN_TXEN | LPUART_LPUEN_RXEN));
    }
    else {
        do
        {
            CLEAR_BIT(lpuart->LPUEN, (LPUART_LPUEN_TXEN | LPUART_LPUEN_RXEN));
            __NOP();
            __NOP();
            __NOP();
        }while(READ_BIT(lpuart->LPUEN, (LPUART_LPUEN_TXEN | LPUART_LPUEN_RXEN)) != 0);
    }
}
////////////////////////////////////////////////////////////////////////////////
/// @brief  Enables or disables the specified LPUART Tx.
/// @param  lpuart: Select the LPUART.
/// @param  state: new state of the lpuart Tx.
///         This parameter can be: ENABLE or DISABLE.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_TX_Cmd(LPUART_TypeDef* lpuart, FunctionalState state)
{

    if (state != DISABLE) { //tx enable
        do
        {
            SET_BIT(lpuart->LPUEN, (LPUART_LPUEN_TXEN));
            __NOP();
            __NOP();
            __NOP();
        }while(READ_BIT(lpuart->LPUEN, (LPUART_LPUEN_TXEN)) != (LPUART_LPUEN_TXEN));
    }
    else {
        do
        {
            CLEAR_BIT(lpuart->LPUEN, (LPUART_LPUEN_TXEN));
            __NOP();
            __NOP();
            __NOP();
        }while(READ_BIT(lpuart->LPUEN, (LPUART_LPUEN_TXEN)) != 0);
    }
}
////////////////////////////////////////////////////////////////////////////////
/// @brief  Enables or disables the specified LPUART Rx.
/// @param  lpuart: Select the LPUART.
/// @param  state: new state of the lpuart Rx.
///         This parameter can be: ENABLE or DISABLE.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_RX_Cmd(LPUART_TypeDef* lpuart, FunctionalState state)
{
    if (state != DISABLE) { //rx enable
        do
        {
            SET_BIT(lpuart->LPUEN, (LPUART_LPUEN_RXEN));
            __NOP();
            __NOP();
            __NOP();
        }while(READ_BIT(lpuart->LPUEN, (LPUART_LPUEN_RXEN)) != (LPUART_LPUEN_RXEN));
    }
    else {
        do
        {
            CLEAR_BIT(lpuart->LPUEN, (LPUART_LPUEN_RXEN));
            __NOP();
            __NOP();
            __NOP();
        }while(READ_BIT(lpuart->LPUEN, (LPUART_LPUEN_RXEN)) != 0);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Enables or disables the specified LPUART interrupts.
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @param  it: specifies the LPUART interrupt sources to be
///         enabled or disabled.
///         This parameter can be one of the following values:
/// @arg    LPUART_LPUCON_ERRIE: Error interrupt Enable
/// @arg    LPUART_LPUCON_RXIE : Receive interrupt Enable
/// @arg    LPUART_LPUCON_TCIE : Transmit complete interrupt Enable
/// @arg    LPUART_LPUCON_TXIE : Transmit Buffer Empty interrupt Enable
///
/// @param  state: new state of the specified lpuart interrupts.
///         This parameter can be: ENABLE or DISABLE.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_ITConfig(LPUART_TypeDef* lpuart, u16 it, FunctionalState state)
{
    it = it & (LPUART_LPUCON_ERRIE | LPUART_LPUCON_RXIE | \
               LPUART_LPUCON_TCIE | LPUART_LPUCON_TXIE);
    (state) ? (lpuart->LPUCON |= it) : (lpuart->LPUCON &= ~it);
}
////////////////////////////////////////////////////////////////////////////////
/// @brief  Enables or disables the LPUART DMA interface.
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @param  dma_request: specifies the DMA request.
///         This parameter can be any combination of the following values:
/// @arg    LPUART_DMAReq_EN: LPUART DMA transmit request
///
/// @param  state: new state of the DMA Request sources.
///         This parameter can be: ENABLE or DISABLE.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_DMACmd(LPUART_TypeDef* lpuart, FunctionalState state)
{
    if (state != DISABLE)  //tx/rx enable
    {
        do
        {
            SET_BIT(lpuart->LPUEN, (LPUART_LPUEN_DMAT | LPUART_LPUEN_DMAR));
            __NOP();
            __NOP();
            __NOP();
        }while(READ_BIT(lpuart->LPUEN, (LPUART_LPUEN_DMAT | LPUART_LPUEN_DMAR)) != (LPUART_LPUEN_DMAT | LPUART_LPUEN_DMAR));
    }
    else
    {
        do
        {
            CLEAR_BIT(lpuart->LPUEN, (LPUART_LPUEN_DMAT | LPUART_LPUEN_DMAR));
            __NOP();
            __NOP();
            __NOP();
        }while(READ_BIT(lpuart->LPUEN, (LPUART_LPUEN_DMAT | LPUART_LPUEN_DMAR)) != 0);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Enables or disables the LPUART DMA Tx interface.
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @param  state: new state of the DMA Request sources.
///         This parameter can be: ENABLE or DISABLE.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_TX_DMACmd(LPUART_TypeDef* lpuart, FunctionalState state)
{
    if (state != DISABLE) { //tx enable
        do
        {
            SET_BIT(lpuart->LPUEN, LPUART_LPUEN_DMAT);
            __NOP();
            __NOP();
            __NOP();
        }while(READ_BIT(lpuart->LPUEN, (LPUART_LPUEN_DMAT )) != (LPUART_LPUEN_DMAT));
    }
    else {
        do
        {
            CLEAR_BIT(lpuart->LPUEN, LPUART_LPUEN_DMAT);
            __NOP();
            __NOP();
            __NOP();
        }while(READ_BIT(lpuart->LPUEN, (LPUART_LPUEN_DMAT )) != 0);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Enables or disables the LPUART DMA Rx interface.
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @param  state: new state of the DMA Request sources.
///         This parameter can be: ENABLE or DISABLE.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_RX_DMACmd(LPUART_TypeDef* lpuart, FunctionalState state)
{
    if (state != DISABLE) { //rx enable
        do
        {
            SET_BIT(lpuart->LPUEN, LPUART_LPUEN_DMAR);
            __NOP();
            __NOP();
            __NOP();
        }while(READ_BIT(lpuart->LPUEN, (LPUART_LPUEN_DMAR )) != (LPUART_LPUEN_DMAR));
    }
    else {
        do
        {
            CLEAR_BIT(lpuart->LPUEN, LPUART_LPUEN_DMAR);
            __NOP();
            __NOP();
            __NOP();
        }while(READ_BIT(lpuart->LPUEN, (LPUART_LPUEN_DMAR )) != 0);
    }
}
////////////////////////////////////////////////////////////////////////////////
/// @brief  Transmits single data through the lpuart peripheral.
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @param  value: the data to transmit.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_SendData(LPUART_TypeDef* lpuart, u8 value)
{
    // Transmit Data
    WRITE_REG(lpuart->LPUTXD, (value & 0xFFU));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Returns the most recent received data by the lpuart peripheral.
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @retval  The received data.
////////////////////////////////////////////////////////////////////////////////
u8 LPUART_ReceiveData(LPUART_TypeDef* lpuart)
{
    // Receive Data
    return (u8)(lpuart->LPURXD & 0xFFU);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Checks whether the specified LPUART flag is set or not.
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @param  flag: specifies the flag to check.
///         This parameter can be one of the following values:
/// @arg    LPUART_LPUSTA_START : Start bit detected flag
/// @arg    LPUART_LPUSTA_PERR  : Check bit error flag
/// @arg    LPUART_LPUSTA_TC    : Transmit data completed flag
/// @arg    LPUART_LPUSTA_TXE   : Transmit buffer empty flag
/// @arg    LPUART_LPUSTA_RXF   : Receive buffer is fulled flag
/// @arg    LPUART_LPUSTA_MATCH : Data is matched flag
/// @arg    LPUART_LPUSTA_FERR  : Frame format error flag
/// @arg    LPUART_LPUSTA_RXOV  : Receive buffer overflow flag
/// @retval  The new state of LPUART_FLAG (SET or RESET).
////////////////////////////////////////////////////////////////////////////////
FlagStatus LPUART_GetFlagStatus(LPUART_TypeDef* lpuart, u16 flag)
{
    return (lpuart->LPUSTA & flag) ? SET : RESET;
}
////////////////////////////////////////////////////////////////////////////////
/// @brief  Clear the specified LPUART status flag.
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @param  flag: specifies the flag to check.
///         This parameter can be one of the following values:
/// @arg    LPUART_LPUSTA_START : Start bit detected flag
/// @arg    LPUART_LPUSTA_PERR  : Check bit error flag
/// @arg    LPUART_LPUSTA_MATCH : Data is matched flag
/// @arg    LPUART_LPUSTA_FERR  : Frame format error flag
/// @arg    LPUART_LPUSTA_RXOV  : Receive buffer overflow flag
/// @retval  The new state of LPUART_FLAG (SET or RESET).
////////////////////////////////////////////////////////////////////////////////
void LPUART_ClearFlagStatus(LPUART_TypeDef* lpuart, u16 flag)
{
    lpuart->LPUSTA = flag;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Checks whether the specified LPUART interrupt has occurred or not.
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @param  it: specifies the LPUART interrupt source to check.
///         This parameter can be one of the following values:
/// @arg    LPUART_LPUIF_TCIF    : Transmit complete interrupt complete flag
/// @arg    LPUART_LPUIF_RXNEGIF : Received falling edge interrupt flag
/// @arg    LPUART_LPUIF_TXIF    : Transmit buffer empty interrupt flag
/// @arg    LPUART_LPUIF_RXIF    : Receive data finish interrupt flag
/// @retval  The new state of LPUART_IT (SET or RESET).
////////////////////////////////////////////////////////////////////////////////
ITStatus LPUART_GetITStatus(LPUART_TypeDef* lpuart, u16 it)
{
    return (lpuart->LPUIF & it) ? SET : RESET;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Clears the lpuart interrupt pending bits.
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @param  it: specifies the interrupt pending bit to clear.
///         This parameter can be one of the following values:
/// @arg    LPUART_LPUIF_TCIF    : Transmit complete interrupt complete flag
/// @arg    LPUART_LPUIF_RXNEGIF : Received falling edge interrupt flag
/// @arg    LPUART_LPUIF_TXIF    : Transmit buffer empty interrupt flag
/// @arg    LPUART_LPUIF_RXIF    : Receive data finish interrupt flag
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_ClearITPendingBit(LPUART_TypeDef* lpuart, u16 it)
{
    //clear LPUART_IT pendings bit
    lpuart->LPUIF = it;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Selects the LPUART WakeUp method.
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @param  mode: specifies the LPUART wakeup method.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_WakeUpConfig(LPUART_TypeDef* lpuart, LPUART_WakeUp_TypeDef mode)
{
    MODIFY_REG(lpuart->WKCKE, LPUART_WKCKE_WKUP_MODE, mode);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Set LPUART  TX polarity
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @param  state: new state of TX polarity.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_SetTXToggle(LPUART_TypeDef* lpuart, FunctionalState state)
{
    MODIFY_REG(lpuart->LPUCON, LPUART_LPUCON_TXPOL, state << LPUART_LPUCON_TXPOL_Pos);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Set LPUART  RX polarity
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @param  state: new state of RX polarity.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_SetRXToggle(LPUART_TypeDef* lpuart, FunctionalState state)
{
    MODIFY_REG(lpuart->LPUCON, LPUART_LPUCON_RXPOL, state << LPUART_LPUCON_RXPOL_Pos);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief  Set LPUART  TX Enable
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @param  state: new state of TX.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_SetTransmitEnable(LPUART_TypeDef* lpuart, FunctionalState state)
{

    MODIFY_REG(lpuart->LPUEN, LPUART_LPUEN_TXEN, state << LPUART_LPUEN_TXEN_Pos);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief  Set LPUART  RX Enable
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @param  state: new state of TX.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_SetRecevieEnable(LPUART_TypeDef* lpuart, FunctionalState state)
{
    MODIFY_REG(lpuart->LPUEN, LPUART_LPUEN_RXEN, state << LPUART_LPUEN_RXEN_Pos);
}


////////////////////////////////////////////////////////////////////////////////
/// @brief  Set match compare data through the lpuart peripheral.
/// @param  lpuart: Select the LPUART or the LPUART peripheral.
/// @param  value: the data to be compared.
/// @retval None.
////////////////////////////////////////////////////////////////////////////////
void LPUART_SetMatchData(LPUART_TypeDef* lpuart, u8 value)
{
    // Transmit Data
    WRITE_REG(lpuart->COMPARE, (value & 0xFFU));
}

/// @}

/// @}

/// @}
