#ifndef _ADC_H
#define _ADC_H

#include <stdint.h>
#include <stdbool.h>

#define ADC_BUFFER_SIZE 5

typedef struct
{
    float    Voltage;
    uint32_t Level;
} PVD_MAP_TypeDef;

extern uint16_t ADC_Buffer[ADC_BUFFER_SIZE];

void Adc_Init(void);
void PWR_Configure(void);
float Get_Load_Current(void);
float Get_Battery_Voltage(void);

#endif /* _ADC_H */

