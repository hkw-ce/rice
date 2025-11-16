#include "platform.h"

rice_information_t rice_info = {0};

void thread_sample_task_entry(void *parameter)
{
	INA226_Init();
    ntc_init();
    while (1)
    {
        ina226_read_rice_info(&rice_info);
        ntc_read_rice_info(&rice_info);
        rt_thread_mdelay(100);
    }
	
}



void thread_communication_task_entry(void *parameter)
{
    /
    while (1)
    {
        rt_thread_mdelay(100);
    }
    
}