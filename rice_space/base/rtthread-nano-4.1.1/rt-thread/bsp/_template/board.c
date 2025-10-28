/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-05-24                  the first version
 */

#include <rthw.h>
#include <rtthread.h>

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
/*
 * Please modify RT_HEAP_SIZE if you enable RT_USING_HEAP
 * the RT_HEAP_SIZE max value = (sram size - ZI size), 1024 means 1024 bytes
 */
#define RT_HEAP_SIZE (6* 1024)
static rt_uint8_t rt_heap[RT_HEAP_SIZE];

RT_WEAK void *rt_heap_begin_get(void)
{
    return rt_heap;
}

RT_WEAK void *rt_heap_end_get(void)
{
    return rt_heap + RT_HEAP_SIZE;
}
#endif

void rt_os_tick_callback(void)
{
    rt_interrupt_enter();

    rt_tick_increase();

    rt_interrupt_leave();
}

/**
 * This function will initial your board.
 */
void rt_hw_board_init(void)
{
    // #error "TODO 1: OS Tick Configuration."
    /*
     * TODO 1: OS Tick Configuration
     * Enable the hardware timer and call the rt_os_tick_callback function
     * periodically with the frequency RT_TICK_PER_SECOND.
     */

    /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif
}

#ifdef RT_USING_CONSOLE

#include "platform.h"

static int uart_init(void)
{
    // #error "TODO 2: Enable the hardware uart and config baudrate."
    console_init(115200);
    return 0;
}
INIT_BOARD_EXPORT(uart_init);

void rt_hw_console_output(const char *str)
{
    // #error "TODO 3: Output the string 'str' through the uart."
    while (*str)
    {
        if (*str == '\n')
        {
            /* 先发送 '\r' */
            while (UART_GetFlagStatus(SHELL_UART, UART_FLAG_TXEPT) == RESET)
                ;
            UART_SendData(SHELL_UART, '\r');
        }

        /* 发送当前字符 */
        while (UART_GetFlagStatus(SHELL_UART, UART_FLAG_TXEPT) == RESET)
            ;
        UART_SendData(SHELL_UART, *str++);
    }
}

/**
 * @brief  从硬件串口读取一个字符（阻塞方式）
 * @return 接收到的字符，如果没有接收到可以返回 -1
 */
char rt_hw_console_getchar(void)
{
    int ch = -1;
    if (UART_GetFlagStatus(SHELL_UART, UART_FLAG_RXAVL) != RESET)
    {
        ch = (int)UART_ReceiveData(SHELL_UART);
        return ch;
    }
    else
    {
        rt_thread_mdelay(10); // 没有数据就延时
    }
    return ch;
}

#ifdef RT_USING_HEAP
int cmd_free(void)
{
    rt_uint32_t total = 0, used = 0, max_used = 0;

    rt_memory_info(&total, &used, &max_used);
    rt_kprintf("total   : %d\n", total);
    rt_kprintf("used    : %d\n", used);
    rt_kprintf("maximum : %d\n", max_used);
    return 0;
}
MSH_CMD_EXPORT(cmd_free, Show the memory usage in the system.);
#endif

#endif
