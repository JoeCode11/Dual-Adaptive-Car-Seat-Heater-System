#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

void ADC0_Init(void);                 // Initialize ADC0 on PE3 (AIN0)
uint16_t ADC0_Read(void);            // Read ADC0 value (0–4095)


float ADC_to_Temperature(uint16_t adc_val);

bool is_temp_valid(float temp);

#endif
