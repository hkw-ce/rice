#include "platform.h"

void amoled_qspi_config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    QSPI_InitTypeDef QSPI_InitStruct;
#if 1

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_10); /* QSPI_DA2 */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_10); /* QSPI_DA3 */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_10); /* QSPI_SCK */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_10); /* QSPI_NSS */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_10); /* QSPI_DA0 */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_10); /* QSPI_DA1 */

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 |
                               GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(GPIOA, &GPIO_InitStruct);

#else
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_10); /* QSPI_DA2 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_10); /* QSPI_DA3 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_10); /* QSPI_SCK */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_10); /* QSPI_NSS */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_10); /* QSPI_DA0 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_10); /* QSPI_DA1 */

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 |
                               GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_QSPI, ENABLE);
    QSPI_StructInit(&QSPI_InitStruct);
    QSPI_InitStruct.ClockPrescalar = 9; // 2025/09/29 17:23 根据布线布局限制目前测4m左右不会有颜色丢失
    QSPI_InitStruct.NssHighLevelHold = 0;
    QSPI_InitStruct.ModeSelection = QSPI_MODE_0;
    QSPI_Init(QSPI, &QSPI_InitStruct);
}

// ------------------ QSPI 发送接口 ------------------

// 单字节发送
void qspi_send_byte(uint8_t byte)
{
    QSPI_CommonConfig_TypeDef qspi_common_cfg;
    QSPI_IndirectExtendConfig_TypeDef qspi_extend_cfg;

    QSPI_CommonConfigStructInit(&qspi_common_cfg);

    // amoled 是标准 SPI 单线
    qspi_common_cfg.InstructionCode = 0; // amoled 不需要指令码，只发送数据
    qspi_common_cfg.InstructPhaseMode = QSPI_InstructPhaseMode_None;
    qspi_common_cfg.AddressPhaseMode = QSPI_AddressPhaseMode_None;
    qspi_common_cfg.DataPhaseMode = QSPI_DataPhaseMode_Quad;
    qspi_common_cfg.DataPhaseSize = QSPI_DataPhaseSize_8Bit;
    qspi_common_cfg.RxdSampleDelayCycles = QSPI_DelayCycles_0;
    qspi_common_cfg.DummyPhaseCycles = 0;

    qspi_extend_cfg.address = 0;
    qspi_extend_cfg.length = 0;
    qspi_extend_cfg.count = 0;

    QSPI_IndirectInit(QSPI, &qspi_common_cfg, &qspi_extend_cfg, QSPI_IndirectModeAccess_Write);

    while (QSPI_GetFlagStatus(QSPI, QSPI_FLAG_EMPTY) == RESET)
        ;
    QSPI_ClearFlag(QSPI, QSPI_FLAG_EMPTY);

    QSPI_SendIndirectDataFifo(QSPI, byte);

    while (QSPI_GetFlagStatus(QSPI, QSPI_FLAG_TCF) == RESET)
        ;
    QSPI_ClearFlag(QSPI, QSPI_FLAG_TCF);

    QSPI_SetOperationMode(QSPI, QSPI_OperationMode_Inactive);
}

// 多字节发送
void qspi_send_data(const uint8_t *buffer, uint16_t length)
{
    QSPI_CommonConfig_TypeDef qspi_common_cfg;
    QSPI_IndirectExtendConfig_TypeDef qspi_extend_cfg;
    uint16_t i;

    QSPI_CommonConfigStructInit(&qspi_common_cfg);

    qspi_common_cfg.InstructionCode = 0; // amoled不需要指令码
    qspi_common_cfg.InstructPhaseMode = QSPI_InstructPhaseMode_None;
    qspi_common_cfg.AddressPhaseMode = QSPI_AddressPhaseMode_None;
    qspi_common_cfg.DataPhaseMode = QSPI_DataPhaseMode_Quad;
    qspi_common_cfg.DataPhaseSize = QSPI_DataPhaseSize_8Bit;
    qspi_common_cfg.RxdSampleDelayCycles = QSPI_DelayCycles_0;
    qspi_common_cfg.DummyPhaseCycles = 0;

    qspi_extend_cfg.address = 0;
    qspi_extend_cfg.length = length - 1;
    qspi_extend_cfg.count = 0;

    QSPI_IndirectInit(QSPI, &qspi_common_cfg, &qspi_extend_cfg, QSPI_IndirectModeAccess_Write);

    for (i = 0; i < length; i++)
    {
        while (QSPI_GetFlagStatus(QSPI, QSPI_FLAG_FULL) == SET)
            ; // 等待 FIFO 有空位
        QSPI_SendIndirectDataFifo(QSPI, buffer[i]);
    }

    // 等待传输完成
    while (QSPI_GetFlagStatus(QSPI, QSPI_FLAG_TCF) == RESET)
        ;
    QSPI_ClearFlag(QSPI, QSPI_FLAG_TCF);

    QSPI_SetOperationMode(QSPI, QSPI_OperationMode_Inactive);
}

uint8_t buf[100];
void amoled_send_cmd(uint8_t cmd, const uint8_t *data, uint16_t data_len)
{
    QSPI_CommonConfig_TypeDef qspi_common_cfg;
    QSPI_IndirectExtendConfig_TypeDef qspi_extend_cfg;
    uint16_t i;

    // 初始化 QSPI 通用配置
    QSPI_CommonConfigStructInit(&qspi_common_cfg);

    /* Instruction = 0x02 单线写命令 */
    qspi_common_cfg.InstructionCode = 0x02;
    qspi_common_cfg.InstructPhaseMode = QSPI_InstructPhaseMode_Single;

    /* 不使用硬件地址，直接把 {0x00, cmd, 0x00} 放到数据里 */
    qspi_common_cfg.AddressPhaseMode = QSPI_AddressPhaseMode_None;

    /* 数据阶段：如果有数据，选择单线或四线 */
    qspi_common_cfg.DataPhaseMode = QSPI_DataPhaseMode_Single;
    qspi_common_cfg.DataPhaseSize = QSPI_DataPhaseSize_8Bit;

    /* 构建发送缓冲区：命令 + 参数/像素数据 */
    uint32_t total_len = data_len + 3; // 3字节命令头
    buf[0] = 0x00;
    buf[1] = cmd;
    buf[2] = 0x00;
    if (data_len > 0 && data != NULL)
        rt_memcpy(&buf[3], data, data_len);

    qspi_extend_cfg.address = 0; // 读需要使用这个地址
    qspi_extend_cfg.length = total_len - 1;
    qspi_extend_cfg.count = 0;

    // 初始化 QSPI 写入
    QSPI_IndirectInit(QSPI, &qspi_common_cfg,
                      &qspi_extend_cfg,
                      QSPI_IndirectModeAccess_Write);

    // 写入数据
    for (i = 0; i < total_len; i++)
    {
        while (QSPI_GetFlagStatus(QSPI, QSPI_FLAG_EMPTY) == RESET)
            ;
        QSPI_SendIndirectDataFifo(QSPI, buf[i]);
    }

    // 等待完成
    while (QSPI_GetFlagStatus(QSPI, QSPI_FLAG_TCF) == RESET)
        ;
    QSPI_ClearFlag(QSPI, QSPI_FLAG_TCF);
    QSPI_SetOperationMode(QSPI, QSPI_OperationMode_Inactive);
}

void amoled_send_sth_cmd(uint8_t cmd, const uint8_t *data, uint16_t data_len)
{
    QSPI_CommonConfig_TypeDef qspi_common_cfg;
    QSPI_IndirectExtendConfig_TypeDef qspi_extend_cfg;
    uint16_t i;

    // 初始化 QSPI 通用配置
    QSPI_CommonConfigStructInit(&qspi_common_cfg);

    /* Instruction = 0x02 单线写命令 */
    qspi_common_cfg.InstructionCode = 0x32;
    qspi_common_cfg.InstructPhaseMode = QSPI_InstructPhaseMode_Single;

    /* 不使用硬件地址，直接把 {0x00, cmd, 0x00} 放到数据里 */
    qspi_common_cfg.AddressPhaseMode = QSPI_AddressPhaseMode_None;

    /* 数据阶段：如果有数据，选择单线或四线 */
    qspi_common_cfg.DataPhaseMode = QSPI_DataPhaseMode_Single;
    qspi_common_cfg.DataPhaseSize = QSPI_DataPhaseSize_8Bit;

    /* 构建发送缓冲区：命令 + 参数/像素数据 */
    uint32_t total_len = data_len + 3; // 3字节命令头
    buf[0] = 0x00;
    buf[1] = cmd;
    buf[2] = 0x00;
    if (data_len > 0 && data != NULL)
        rt_memcpy(&buf[3], data, data_len);

    qspi_extend_cfg.address = 0; // 读需要使用这个地址
    qspi_extend_cfg.length = total_len - 1;
    qspi_extend_cfg.count = 0;

    // 初始化 QSPI 写入
    QSPI_IndirectInit(QSPI, &qspi_common_cfg,
                      &qspi_extend_cfg,
                      QSPI_IndirectModeAccess_Write);

    // 写入数据
    for (i = 0; i < total_len; i++)
    {
        while (QSPI_GetFlagStatus(QSPI, QSPI_FLAG_EMPTY) == RESET)
            ;
        QSPI_SendIndirectDataFifo(QSPI, buf[i]);
    }

    // 等待完成
    while (QSPI_GetFlagStatus(QSPI, QSPI_FLAG_TCF) == RESET)
        ;
    QSPI_ClearFlag(QSPI, QSPI_FLAG_TCF);
    QSPI_SetOperationMode(QSPI, QSPI_OperationMode_Inactive);
    //		rt_hw_us_delay(10);
}

void amoled_send_color(uint8_t cmd, const uint8_t *color_data, uint32_t length)
{
    amoled_send_sth_cmd(cmd, 0, 0);

    qspi_send_data(color_data, length);
}

// 2025/09/21 14:57 通过逻辑分析仪 有对应数据
uint8_t buf[] = {0x12, 0x34, 0x56, 0x78};
int test_qspi(void)
{

    amoled_qspi_config();
    rt_kprintf("test qspi\r\n");
    amoled_send_cmd(0x11, buf, 4);
    return 0;
}

MSH_CMD_EXPORT(test_qspi, qspi);

// 1 AMOLED 写命令： [0x02] + [00 CMD 00] + [参数...]
