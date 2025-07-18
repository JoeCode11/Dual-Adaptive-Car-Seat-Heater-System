/**********************************************************************************************
 *
 * Module: GPIO
 *
 * File Name: GPIO.c
 *
 * Description: Source file for the TM4C123GH6PM DIO driver for TivaC Built-in Buttons and LEDs
 *
 * Author: Edges for Training Team
 *
 ***********************************************************************************************/
#include "gpio.h"
#include "tm4c123gh6pm_registers.h"
#include <stdint.h>


void GPIO_BuiltinButtonsLedsInit(void)
{
    /*
     * PF0 --> SW2
     * PF1 --> Red LED
     * PF2 --> Blue LED
     * PF3 --> Green LED
     * PF4 --> SW1
     *
     */

    /* Enable clock for PORTF and wait for clock to start */
    SYSCTL_RCGCGPIO_REG |= (1<<5) | (1<<0) | (1<<1); //PF, PA, PB
    while(!(SYSCTL_PRGPIO_REG & 0x20));
    while(!(SYSCTL_PRGPIO_REG & 0x01));
    while(!(SYSCTL_PRGPIO_REG & 0x02));

    /* ----------- PORTF for Built-in Buttons and LEDs ----------- */
    GPIO_PORTF_LOCK_REG   = 0x4C4F434B;                       /* Unlock the GPIO_PORTF_CR_REG */
    GPIO_PORTF_CR_REG    |= (1<<0);                           /* Enable changes on PF0 */
    GPIO_PORTF_AMSEL_REG &= 0xE0;                             /* Disable Analog on PF0, PF1, PF2, PF3 and PF4 */
    GPIO_PORTF_PCTL_REG  &= 0xFFF00000;                       /* Clear PMCx bits for PF0, PF1, PF2, PF3 and PF4 to use it as GPIO pins */
    GPIO_PORTF_DIR_REG   &= ~(1<<0) & ~(1<<4);                /* Configure PF0 & PF4 as input pins */
    GPIO_PORTF_DIR_REG   |= ((1<<1) | (1<<2) | (1<<3));       /* Configure PF1, PF2 & PF3 as output pins */
    GPIO_PORTF_AFSEL_REG &= 0xE0;                             /* Disable alternative function on PF0, PF1, PF2, PF3 and PF4 */
    GPIO_PORTF_PUR_REG   |= ((1<<0)|(1<<4));                  /* Enable pull-up on PF0 & PF4 */
    GPIO_PORTF_DEN_REG   |= 0x1F;                             /* Enable Digital I/O on PF0, PF1, PF2, PF3 and PF4 */
    GPIO_PORTF_DATA_REG  &= ~(1<<1) & ~(1<<2) & ~(1<<3);      /* Clear bits 1, 2 & 3 in Data register to turn off the LEDs */

    /* ----------- PORTA for External Button on PA5 ----------- */
    GPIO_PORTA_DIR_REG &= ~(1<<5);                     // PA5 = input
    GPIO_PORTA_DEN_REG |= (1<<5);                      // Digital enable
    GPIO_PORTA_PUR_REG |= (1<<5);                      // Pull-up resistor

    /* ----------- PORTB for Breadboard LED on PB2 ----------- */
//    GPIO_PORTB_DIR_REG |= (1<<2);                      // PB2 = output
//    GPIO_PORTB_DEN_REG |= (1<<2);                      // Digital enable
//    GPIO_PORTB_DATA_REG &= ~(1<<2);                    // LED off


    /* now PB2/PB3/PB4 each drive one external LED */
    GPIO_PORTB_DIR_REG |= ( (1<<2) | (1<<3) | (1<<4) );
    GPIO_PORTB_DEN_REG |= ( (1<<2) | (1<<3) | (1<<4) );
    /* start all three off */
    GPIO_PORTB_DATA_REG &= ~((1<<2) | (1<<3) | (1<<4));
}

void GPIO_RedLedOn(void)
{
    GPIO_PORTF_DATA_REG |= (1<<1);  /* Red LED ON */
}

void GPIO_BlueLedOn(void)
{
    GPIO_PORTF_DATA_REG |= (1<<2);  /* Blue LED ON */
}

void GPIO_GreenLedOn(void)
{
    GPIO_PORTF_DATA_REG |= (1<<3);  /* Green LED ON */
}

void GPIO_RedLedOff(void)
{
    GPIO_PORTF_DATA_REG &= ~(1<<1);  /* Red LED OFF */
}

void GPIO_BlueLedOff(void)
{
    GPIO_PORTF_DATA_REG &= ~(1<<2);  /* Blue LED OFF */
}

void GPIO_GreenLedOff(void)
{
    GPIO_PORTF_DATA_REG &= ~(1<<3);  /* Green LED OFF */
}

void GPIO_RedLedToggle(void)
{
    GPIO_PORTF_DATA_REG ^= (1<<1);  /* Red LED is toggled */
}

void GPIO_BlueLedToggle(void)
{
    GPIO_PORTF_DATA_REG ^= (1<<2);  /* Blue LED is toggled */
}

void GPIO_GreenLedToggle(void)
{
    GPIO_PORTF_DATA_REG ^= (1<<3);  /* Green LED is toggled */
}

//void GPIO_ExternLedToggle(void)
//{
//    GPIO_PORTB_DATA_REG ^= (1<<2);  /* Green LED is toggled */
//}


/* External LEDs functions */

void GPIO_ExternLedOn(uint8_t pin)
{
    GPIO_PORTB_DATA_REG |= (1 << pin);
}

void GPIO_ExternLedOff(uint8_t pin)
{
    GPIO_PORTB_DATA_REG &= ~(1 << pin);
}

void GPIO_ExternRedOn(void)   {
    GPIO_ExternLedOn(EXTERN_LED_RED);
}
void GPIO_ExternRedOff(void)  {
    GPIO_ExternLedOff(EXTERN_LED_RED);
}
void GPIO_ExternGreenOn(void) {
    GPIO_ExternLedOn(EXTERN_LED_GREEN);
}
void GPIO_ExternGreenOff(void){
    GPIO_ExternLedOff(EXTERN_LED_GREEN);
}
void GPIO_ExternBlueOn(void)  {
    GPIO_ExternLedOn(EXTERN_LED_BLUE);
}
void GPIO_ExternBlueOff(void) {
    GPIO_ExternLedOff(EXTERN_LED_BLUE);
}


uint8 GPIO_SW1GetState(void)
{
    return ((GPIO_PORTF_DATA_REG >> 4) & 0x01);
}

uint8 GPIO_SW2GetState(void)
{
    return ((GPIO_PORTF_DATA_REG >> 0) & 0x01);
}

void GPIO_SW1EdgeTriggeredInterruptInit(void)
{
    GPIO_PORTF_IS_REG    &= ~(1<<4);      /* PF4 detect edges */
    GPIO_PORTF_IBE_REG   &= ~(1<<4);      /* PF4 will detect a certain edge */
    GPIO_PORTF_IEV_REG   &= ~(1<<4);      /* PF4 will detect a falling edge */
    GPIO_PORTF_ICR_REG   |= (1<<4);       /* Clear Trigger flag for PF4 (Interrupt Flag) */
    GPIO_PORTF_IM_REG    |= (1<<4);       /* Enable Interrupt on PF4 pin */
    /* Set GPIO PORTF priority as 5 by set Bit number 21, 22 and 23 with value 2 */
    NVIC_PRI7_REG = (NVIC_PRI7_REG & GPIO_PORTF_PRIORITY_MASK) | (GPIO_PORTF_INTERRUPT_PRIORITY<<GPIO_PORTF_PRIORITY_BITS_POS);
    NVIC_EN0_REG         |= 0x40000000;   /* Enable NVIC Interrupt for GPIO PORTF by set bit number 30 in EN0 Register */
}

void GPIO_SW2EdgeTriggeredInterruptInit(void)
{
    GPIO_PORTF_IS_REG    &= ~(1<<0);      /* PF0 detect edges */
    GPIO_PORTF_IBE_REG   &= ~(1<<0);      /* PF0 will detect a certain edge */
    GPIO_PORTF_IEV_REG   &= ~(1<<0);      /* PF0 will detect a falling edge */
    GPIO_PORTF_ICR_REG   |= (1<<0);       /* Clear Trigger flag for PF0 (Interrupt Flag) */
    GPIO_PORTF_IM_REG    |= (1<<0);       /* Enable Interrupt on PF0 pin */
    /* Set GPIO PORTF priority as 5 by set Bit number 21, 22 and 23 with value 2 */
    NVIC_PRI7_REG = (NVIC_PRI7_REG & GPIO_PORTF_PRIORITY_MASK) | (GPIO_PORTF_INTERRUPT_PRIORITY<<GPIO_PORTF_PRIORITY_BITS_POS);
    NVIC_EN0_REG         |= 0x40000000;   /* Enable NVIC Interrupt for GPIO PORTF by set bit number 30 in EN0 Register */
}






void GPIO_PB0EdgeTriggeredInterruptInit(void)
{
    GPIO_PORTB_LOCK_REG = 0x4C4F434B;         /* Unlock if needed */
    GPIO_PORTB_CR_REG  |= (1 << 0);           /* Allow changes to PB0 */

    GPIO_PORTB_DIR_REG  &= ~(1 << 0);         /* PB0 = input */
    GPIO_PORTB_DEN_REG  |=  (1 << 0);         /* Digital enable */
    GPIO_PORTB_PUR_REG  |=  (1 << 0);         /* Pull-up resistor */

    GPIO_PORTB_IS_REG   &= ~(1 << 0);         /* Edge-sensitive */
    GPIO_PORTB_IBE_REG  &= ~(1 << 0);         /* Single edge */
    GPIO_PORTB_IEV_REG  &= ~(1 << 0);         /* Falling edge */
    GPIO_PORTB_ICR_REG  |=  (1 << 0);         /* Clear any prior flag */
    GPIO_PORTB_IM_REG   |=  (1 << 0);         /* Enable interrupt on PB0 */

    /* Priority 5 = binary 010 = shift into bits 13:15 of NVIC_PRI0 for IRQ1 (GPIO Port B) */
    NVIC_PRI0_REG = (NVIC_PRI0_REG & 0xFFFF1FFF) | (GPIO_PORTB_INTERRUPT_PRIORITY << 13);
    NVIC_EN0_REG  |= (1 << 1);                /* Enable interrupt for IRQ1 (GPIO Port B) */
}

