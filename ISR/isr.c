/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Header */
#include "isr.h"

extern SemaphoreHandle_t xButtonSemaphore;

extern SemaphoreHandle_t xButtonSemaphoreSw2;


#define NUMBER_OF_ITERATIONS_PER_ONE_MILI_SECOND 369



void Delay_MS(unsigned long long n)
{
    volatile unsigned long long count = 0;
    while(count++ < (NUMBER_OF_ITERATIONS_PER_ONE_MILI_SECOND * n) );
}


//void GPIOF_Handler(void) {
//    // Clear interrupt flag for PF4
//    GPIO_PORTF_ICR_REG |= (1 << 4);
//
//    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//    xSemaphoreGiveFromISR(xButtonSemaphore, &xHigherPriorityTaskWoken);
//    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//}


void GPIOF_Handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Check if PF4 (SW1) caused the interrupt
    if (GPIO_PORTF_RIS_REG & (1 << 4)) {
        GPIO_PORTF_ICR_REG |= (1 << 4);  // Clear interrupt flag
        xSemaphoreGiveFromISR(xButtonSemaphore, &xHigherPriorityTaskWoken);
    }

    // Check if PF0 (SW2) caused the interrupt
    if (GPIO_PORTF_RIS_REG & (1 << 0)) {
        GPIO_PORTF_ICR_REG |= (1 << 0);  // Clear interrupt flag
        xSemaphoreGiveFromISR(xButtonSemaphoreSw2, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}



void GPIOPortB_Handler(void)
{

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;


    if (GPIO_PORTB_RIS_REG & (1 << 0)) {
        GPIO_PORTB_ICR_REG |= (1 << 0); // Clear interrupt
        xSemaphoreGiveFromISR(xButtonSemaphore, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}
