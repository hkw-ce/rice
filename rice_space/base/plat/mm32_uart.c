#include "hal_conf.h"
#include "platform.h"

uart_t uart1 = {
    .uart = UART1,
    .rx_len = 0,
    .rx_ok = 0,
    .callback = NULL};

uart_t uart2 = {
    .uart = UART2,
    .rx_len = 0,
    .rx_ok = 0,
};

uart_t uart3 = {
    .uart = UART3,
    .rx_len = 0,
    .rx_ok = 0,
};

/**
 * @brief  UART ��ʼ������
 * @param  UARTx: ָ��Ҫ���õ� UART �����ָ��
 * @param  Baudrate: ������
 * @param  GPIOx: ָ��Ҫ���õ� GPIO �����ָ��
 * @param  TxPin: TX ����
 * @param  RxPin: RX ����
 * @param  TxPinSource: TX ���Ÿ���Դ
 * @param  RxPinSource: RX ���Ÿ���Դ
 * @param  GPIO_AF: GPIO ���ù��� -- �������ֲ�
 * @param  IRQn: �ж�������
 * @retval None
 */
void uart_config(UART_TypeDef *UARTx,
                 uint32_t Baudrate,
                 GPIO_TypeDef *GPIOx,
                 uint16_t TxPin,
                 uint16_t RxPin,
                 uint8_t TxPinSource,
                 uint8_t RxPinSource,
                 uint8_t GPIO_AF,
                 uint8_t it_mode,
                 IRQn_Type IRQn)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    UART_InitTypeDef UART_InitStruct;

    // 1. ���� UART �� GPIO ʱ��
    if (UARTx == UART1)
        RCC_APB2PeriphClockCmd(RCC_APB2ENR_UART1, ENABLE);
    else if (UARTx == UART2)
        RCC_APB1PeriphClockCmd(RCC_APB1ENR_UART2, ENABLE);
    else if (UARTx == UART3)
        RCC_APB1PeriphClockCmd(RCC_APB1ENR_UART3, ENABLE);

    if (GPIOx == GPIOA)
        RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA, ENABLE);
    else if (GPIOx == GPIOB)
        RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOB, ENABLE);
    else if (GPIOx == GPIOC)
        RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOC, ENABLE);

    // 2. ���� UART ����
    UART_StructInit(&UART_InitStruct);
    UART_InitStruct.BaudRate = Baudrate;
    UART_InitStruct.WordLength = UART_WordLength_8b;
    UART_InitStruct.StopBits = UART_StopBits_1;
    UART_InitStruct.Parity = UART_Parity_No;
    UART_InitStruct.HWFlowControl = UART_HWFlowControl_None;
    UART_InitStruct.Mode = UART_Mode_Rx | UART_Mode_Tx;
    UART_Init(UARTx, &UART_InitStruct);

    // 3. ���� GPIO ���ù���
    GPIO_PinAFConfig(GPIOx, TxPinSource, GPIO_AF);
    GPIO_PinAFConfig(GPIOx, RxPinSource, GPIO_AF);

    // ���� TX ���ţ������������
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = TxPin;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOx, &GPIO_InitStruct);

    // ���� RX ���ţ���������
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = RxPin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOx, &GPIO_InitStruct);
    // ���� UART �ж�/�����ж�
    if (it_mode != IT_NULL)
    {
        UART_ITConfig(UARTx, UART_IT_RXIEN, ENABLE);
        if (it_mode == UART_IDLE)
            UARTx->IER |= UART_IER_RXIDLE;
        // 4. �����ж�
        NVIC_InitStruct.NVIC_IRQChannel = IRQn;
        NVIC_InitStruct.NVIC_IRQChannelPriority = 0x01;
        NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStruct);
    }
    // 5. ���� UART
    UART_Cmd(UARTx, ENABLE);
}

void UART_SendBytes(UART_TypeDef *uart, const uint8_t *data, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        while (RESET == UART_GetFlagStatus(uart, UART_FLAG_TXEPT))
            ;
        UART_SendData(uart, data[i]);
    }
}

void UART1_IRQHandler(void)
{
#if defined(USE_UART1)
    uart_t *huart = &uart1;

    // �����ж�
    if (UART_GetITStatus(huart->uart, UART_IT_RXIEN) != RESET)
    {
        if (huart->callback)
        {
            huart->callback(UART_ReceiveData(huart->uart));
        }
        UART_ClearITPendingBit(huart->uart, UART_IT_RXIEN);
    }

    // �����жϣ�һ֡�������
    if (huart->uart->ISR & UART_ISR_RXIDLE)
    {
        huart->uart->ICR = UART_ICR_RXIDLE;
        huart->rx_ok = 1;
    }
#endif
}

/**
 * @brief  UART2 �жϴ�����
 * @note   �Ұ�it.c ���жϺ��� ��Ϊ��������
 * @note   ����ͨ�ж�����������ݣ��ÿ����ж��жϡ�������ɡ�����������
 */
void UART2_IRQHandler(void)
{
#if defined(USE_UART2)
    uart_t *huart = &uart2;

    // �����ж�
    if (UART_GetITStatus(huart->uart, UART_IT_RXIEN) != RESET)
    {
        if (huart->rx_len < UART_RX_BUF_SIZE)
        {
            huart->rx_buf[huart->rx_len++] = UART_ReceiveData(huart->uart);
        }
        if (huart->callback)
        {
            huart->callback(UART_ReceiveData(huart->uart));
        }
        UART_ClearITPendingBit(huart->uart, UART_IT_RXIEN);
    }

    // �����жϣ�һ֡�������
    if (huart->uart->ISR & UART_ISR_RXIDLE)
    {
        huart->uart->ICR = UART_ICR_RXIDLE;
        huart->rx_ok = 1;
    }
#endif
}

void UART3_IRQHandler(void)
{
#if defined(USE_UART3)
    uart_t *huart = &uart3;

    // �����ж�
    if (UART_GetITStatus(huart->uart, UART_IT_RXIEN) != RESET)
    {
        if (huart->rx_len < UART_RX_BUF_SIZE)
        {
            huart->rx_buf[huart->rx_len++] = UART_ReceiveData(huart->uart);
        }
        if (huart->callback)
        {
            huart->callback(UART_ReceiveData(huart->uart));
        }
        UART_ClearITPendingBit(huart->uart, UART_IT_RXIEN);
    }

    // �����жϣ�һ֡�������
    if (huart->uart->ISR & UART_ISR_RXIDLE)
    {
        huart->uart->ICR = UART_ICR_RXIDLE;
        if (huart->idle_callback)
        {
            huart->idle_callback(huart->rx_buf, huart->rx_len); // ���ÿ��лص�����
        }
        huart->rx_len = 0;
        huart->rx_ok = 1;
    }
#endif
}

/* use demo
UART_Configure(UART2,
               115200,
               GPIOA,
               GPIO_Pin_2, GPIO_Pin_3,
               GPIO_PinSource2, GPIO_PinSource3,
               GPIO_AF_1,
               UART2_IRQn);
*/
