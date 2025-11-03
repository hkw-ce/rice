
#include "bsp_vl53l0x.h"

VL53L0X_Dev_t vl53l0x_dev;                    // 设备I2C数据结构
VL53L0X_DeviceInfo_t vl53l0x_dev_info;        // 设备ID版本信息
uint8_t AjustOK = 0;                          // 校准标志位

// VL53L0X测量模式参数
// 0:默认; 1:高精度; 2:长距离; 3:高速
mode_data Mode_data[] =
{
    {(FixPoint1616_t)(0.25*65536), (FixPoint1616_t)(18*65536),  33000, 14, 10}, // 默认
    {(FixPoint1616_t)(0.25*65536), (FixPoint1616_t)(18*65536), 200000, 14, 10}, // 高精度
    {(FixPoint1616_t)(0.1*65536),  (FixPoint1616_t)(60*65536),  33000, 18, 14}, // 长距离
    {(FixPoint1616_t)(0.25*65536), (FixPoint1616_t)(32*65536),  20000, 14, 10}, // 高速
};

// API错误信息打印
// Status错误查看VL53L0X_Error枚举定义
void print_pal_error(VL53L0X_Error Status)
{
    char buf[VL53L0X_MAX_STRING_LENGTH];
    VL53L0X_GetPalErrorString(Status, buf);  // 根据Status状态获取错误信息字符串
    printf("API Status: %i : %s\r\n", Status, buf); // 打印状态和错误信息
}

// 设置VL53L0X设备I2C地址
// dev: 设备I2C数据结构
// newaddr: 设备新I2C地址
VL53L0X_Error vl53l0x_Addr_set(VL53L0X_Dev_t *dev, uint8_t newaddr)
{
    uint16_t Id;
    uint8_t FinalAddress;
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    u8 sta = 0x00;

    FinalAddress = newaddr;

    if (FinalAddress == dev->I2cDevAddr) // 如果设备I2C地址与目标地址一致，直接退出
        return VL53L0X_ERROR_NONE;

    // 在进行第一个寄存器写入之前，设置I2C标准模式(400Khz)
    Status = VL53L0X_WrByte(dev, 0x88, 0x00);
    if (Status != VL53L0X_ERROR_NONE)
    {
        sta = 0x01; // 设置I2C标准模式失败
        goto set_error;
    }

    // 使用默认的0x52地址读取一个寄存器
    Status = VL53L0X_RdWord(dev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Id);
    if (Status != VL53L0X_ERROR_NONE)
    {
        sta = 0x02; // 读取寄存器失败
        goto set_error;
    }

    if (Id == 0xEEAA)
    {
        // 设置设备新的I2C地址
        Status = VL53L0X_SetDeviceAddress(dev, FinalAddress);
        if (Status != VL53L0X_ERROR_NONE)
        {
            sta = 0x03; // 设置I2C地址失败
            goto set_error;
        }

        // 修改参数结构中的I2C地址
        dev->I2cDevAddr = FinalAddress;

        // 验证新I2C地址写入是否成功
        Status = VL53L0X_RdWord(dev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Id);
        if (Status != VL53L0X_ERROR_NONE)
        {
            sta = 0x04; // 新I2C地址验证失败
            goto set_error;
        }
    }

set_error:
    if (Status != VL53L0X_ERROR_NONE)
    {
        print_pal_error(Status); // 打印错误信息
    }
    if (sta != 0)
        printf("sta:0x%x\r\n", sta);
    return Status;
}

// vl53l0x复位函数
// dev: 设备I2C数据结构
void vl53l0x_reset(VL53L0X_Dev_t *dev)
{
    uint8_t addr;
    addr = dev->I2cDevAddr;                    // 保存设备原I2C地址
    VL53L0X_Xshut(0);                          // 失能VL53L0X
    delay_ms(30);
    VL53L0X_Xshut(1);                          // 使能VL53L0X，此时时钟在挂起（I2C地址指默认0X52）
    delay_ms(30);
    dev->I2cDevAddr = 0x52;
    vl53l0x_Addr_set(dev, addr);               // 恢复VL53L0X设置为原来当前原I2C地址
    VL53L0X_DataInit(dev);
}

// 初始化vl53l0x
// dev: 设备I2C数据结构
VL53L0X_Error vl53l0x_init(VL53L0X_Dev_t *dev)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    VL53L0X_Dev_t *pMyDevice = dev;

    gpio_config(GPIOA, GPIO_Pin_12, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);

    pMyDevice->I2cDevAddr = VL53L0X_Addr;      // I2C地址(上电默认0x52)
    pMyDevice->comms_type = 1;                 // I2C通信模式
    pMyDevice->comms_speed_khz = 400;          // I2C通信速率

    VL53L0X_i2c_init();                        // 初始化IIC总线

    VL53L0X_Xshut(0);                          // 失能VL53L0X
    delay_ms(50);
    VL53L0X_Xshut(1);                          // 使能VL53L0X，此时时钟在挂起
    delay_ms(50);

    vl53l0x_Addr_set(pMyDevice, 0x54);         // 设置VL53L0X新I2C地址为0x54
    if (Status != VL53L0X_ERROR_NONE) goto error;

    Status = VL53L0X_DataInit(pMyDevice);      // 设备初始化
    if (Status != VL53L0X_ERROR_NONE) goto error;

    delay_ms(2);
    Status = VL53L0X_GetDeviceInfo(pMyDevice, &vl53l0x_dev_info); // 获取设备ID信息
    if (Status != VL53L0X_ERROR_NONE) goto error;

error:
    if (Status != VL53L0X_ERROR_NONE)
    {
        print_pal_error(Status);               // 打印错误信息
        return Status;
    }

    return Status;
}

void VL53L0x_test(void)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE; // 错误状态
    uint8_t mode = 0;                          // 0:默认; 1:高精度; 2:长距离; 3:高速

    while (vl53l0x_init(&vl53l0x_dev))         // vl53l0x初始化
    {
        LOG_E("VL53L0X Error!!!\n\r");
        delay_ms(500);
    }
    LOG_I("VL53L0X OK\r\n");

    while (vl53l0x_set_mode(&vl53l0x_dev, mode)) // 设置测量模式
    {
        LOG_E("Mode Set Error\r\n");
        delay_ms(500);
    }

    while (1)
    {
        if (Status == VL53L0X_ERROR_NONE)
        {
            // 执行单次测距并获取测量数据
            Status = VL53L0X_PerformSingleRangingMeasurement(&vl53l0x_dev, &vl53l0x_data);
            LOG_I("d: %4imm\r\n", vl53l0x_data.RangeMilliMeter); // 打印测量距离
        }
        else
        {
            LOG_I("error\r\n");
        }
        delay_ms(500);
    }
}

MSH_CMD_EXPORT(VL53L0x_test, tew);

static rt_thread_t vl53_tid = RT_NULL;         // VL53L0X 任务句柄

/*======================================================================
 *  任务入口函数
 *======================================================================*/
static void thread_vl53l0x_task_entry(void *parameter)
{
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    uint8_t mode = 0;

    // 初始化 VL53L0X
    while (vl53l0x_init(&vl53l0x_dev))
    {
        rt_kprintf("VL53L0X 初始化失败！\r\n");
        rt_thread_mdelay(500);
    }
    rt_kprintf("VL53L0X 初始化成功\r\n");

    // 设置模式
    while (vl53l0x_set_mode(&vl53l0x_dev, mode))
    {
        rt_kprintf("模式设置失败\r\n");
        rt_thread_mdelay(500);
    }

    // 启动连续测距（可选）
    // VL53L0X_StartContinuous(&vl53l0x_dev, 0);

    // 主循环
    while (1)
    {
        Status = VL53L0X_PerformSingleRangingMeasurement(&vl53l0x_dev, &vl53l0x_data);
        if (Status == VL53L0X_ERROR_NONE)
        {
            rt_kprintf("距离: %4d mm\r\n", vl53l0x_data.RangeMilliMeter);
        }
        else
        {
            rt_kprintf("测距错误: %d\r\n", Status);
        }

        rt_thread_mdelay(500);
    }
}

/*======================================================================
 *  线程启动/停止命令（完全按你指定格式）
 * ------------------------------------------------------------- */
static void vl53_task_cmd(int argc, char **argv)
{
    if (argc < 2)
    {
        rt_kprintf("Usage: %s {start|stop}\n", argv[0]);
        return;
    }

    if (rt_strcmp(argv[1], "start") == 0)
    {
        if (vl53_tid != RT_NULL)
        {
            rt_kprintf("VL53L0X task already running.\n");
            return;
        }

        vl53_tid = rt_thread_create("vl53",
                                    thread_vl53l0x_task_entry,
                                    RT_NULL,
                                    1024,
                                    RT_THREAD_PRIORITY_MAX / 2,
                                    20);
        if (vl53_tid)
        {
            rt_thread_startup(vl53_tid);
            rt_kprintf("VL53L0X task started.\n");
        }
        else
        {
            rt_kprintf("Failed to create VL53L0X thread!\n");
        }
    }
    else if (rt_strcmp(argv[1], "stop") == 0)
    {
        if (vl53_tid)
        {
            rt_thread_delete(vl53_tid);
            vl53_tid = RT_NULL;
            rt_kprintf("VL53L0X task stopped.\n");
        }
        else
        {
            rt_kprintf("VL53L0X task not running.\n");
        }
    }
    else
    {
        rt_kprintf("Unknown command: %s\n", argv[1]);
    }
}
MSH_CMD_EXPORT(vl53_task_cmd, VL53L0X background task: start|stop);