#ifndef ISR_H
#define ISR_H

#include "tm4c123gh6pm_registers.h"


void GPIOF_Handler(void);
void Delay_MS(unsigned long long n);

void ADC0SS3_Handler(void);

extern QueueHandle_t xButtonQueue;


#endif
