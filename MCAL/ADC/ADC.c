#include "ADC.h"
#include "tm4c123gh6pm_registers.h"

void ADC0_Init(void) {
    // 1. Enable clock for ADC0 and GPIOE
    SYSCTL_RCGCADC_REG |= 0x01;       // Enable ADC0 clock
    SYSCTL_RCGCGPIO_REG |= 0x10;      // Enable clock for PORTE
    while ((SYSCTL_PRGPIO_REG & 0x10) == 0); // Wait for PORTE ready
//    for (volatile int i = 0; i < 1000; i++); // Delay for stabilization

    // 2. Configure PE3 as AIN0
    GPIO_PORTE_AFSEL_REG |= 0x08;     // Enable alternate function on PE3
    GPIO_PORTE_DEN_REG &= ~0x08;      // Disable digital on PE3
    GPIO_PORTE_AMSEL_REG |= 0x08;     // Enable analog function on PE3

    // 3. Configure ADC0 SS3
    ADC0_ACTSS_R &= ~0x08;          // Disable SS3 during config
    ADC0_EMUX_R &= ~0xF000;         // SS3 is software triggered
    ADC0_SSMUX3_R = 0;              // AIN0 (PE3)
    ADC0_SSCTL3_R = 0x06;           // Single sample, set IE0 and END0
    ADC0_ACTSS_R |= 0x08;           // Enable SS3
}

uint16_t ADC0_Read(void) {
    ADC0_PSSI_R = 0x08;                      // Initiate SS3 conversion
    while ((ADC0_RIS_R & 0x08) == 0);        // Wait for conversion complete
    uint16_t result = ADC0_SSFIFO3_R & 0xFFF; // 12-bit result
    ADC0_ISC_R = 0x08;                       // Clear interrupt flag
    return result;
}


float ADC_to_Temperature(uint16_t adc_val) {
    float voltage = (adc_val * 3.3f) / 4095.0f; // Convert to volts
    return (voltage / 3.3f) * 45.0f;            // Map to 0–45°C
}


bool is_temp_valid(float temp) {
    return (temp >= 5.0f && temp <= 40.0f);
}
