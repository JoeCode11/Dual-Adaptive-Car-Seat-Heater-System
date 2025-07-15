/**********************************************************************************************
 *
 * Module: GPIO
 *
 * File Name: GPIO.h
 *
 * Description: Header file for the TM4C123GH6PM DIO driver for TivaC Built-in Buttons and LEDs
 *
 * Author: Edges for Training Team
 *
 ***********************************************************************************************/

#ifndef GPIO_H_
#define GPIO_H_

#include "std_types.h"

#define GPIO_PORTF_PRIORITY_MASK      0xFF1FFFFF
#define GPIO_PORTF_PRIORITY_BITS_POS  21
#define GPIO_PORTF_INTERRUPT_PRIORITY 5

#define PRESSED                ((uint8)0x00)
#define RELEASED               ((uint8)0x01)


/* For external LEDs */

#define EXTERN_LED_RED   2
#define EXTERN_LED_GREEN 3
#define EXTERN_LED_BLUE  4

void GPIO_ExternRedOn(void);
void GPIO_ExternRedOff(void);
void GPIO_ExternGreenOn(void);
void GPIO_ExternGreenOff(void);
void GPIO_ExternBlueOn(void);
void GPIO_ExternBlueOff(void);

#define GPIO_PORTB_PRIORITY_MASK     0xFFFF1FFF
#define GPIO_PORTB_PRIORITY_BITS_POS 13
#define GPIO_PORTB_INTERRUPT_PRIORITY 2   // Priority level 5 (binary 010)


void GPIO_PB0EdgeTriggeredInterruptInit(void);


void GPIO_BuiltinButtonsLedsInit(void);

void GPIO_RedLedOn(void);
void GPIO_BlueLedOn(void);
void GPIO_GreenLedOn(void);

void GPIO_RedLedOff(void);
void GPIO_BlueLedOff(void);
void GPIO_GreenLedOff(void);

void GPIO_RedLedToggle(void);
void GPIO_BlueLedToggle(void);
void GPIO_GreenLedToggle(void);
void GPIO_ExternLedToggle(void);

uint8 GPIO_SW1GetState(void);
uint8 GPIO_SW2GetState(void);

void GPIO_SW1EdgeTriggeredInterruptInit(void);
void GPIO_SW2EdgeTriggeredInterruptInit(void);

#endif /* GPIO_H_ */
