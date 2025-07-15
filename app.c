

//Libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>


/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"



/* MCAL includes. */
#include "uart0.h"
#include "gpio.h"
#include "GPTM.h"
#include "ADC/ADC.h"


/* Interrupts */
#include "ISR/isr.h"

/* The HW setup function */
static void prvSetupHardware( void );

/* Task Handles */
TaskHandle_t xHeaterOnOffHandle     = NULL;
TaskHandle_t xHeaterOnOffHandle2     = NULL; // For passenger

TaskHandle_t xTempSetterHandle      = NULL;
TaskHandle_t xTempReaderHandle      = NULL;

TaskHandle_t xTempReaderHandle2      = NULL;


TaskHandle_t xHeatControlHandle     = NULL;

TaskHandle_t xTempSetterHandle2      = NULL;
TaskHandle_t xHeatControlHandle2     = NULL;

TaskHandle_t xDisplayHandle         = NULL;
TaskHandle_t xDiagnosticHandle      = NULL;

TaskHandle_t xDiagnosticHandle2      = NULL;



/* FreeRTOS tasks */
void vHeaterOnOff(void *pvParameters);
void vHeaterOnOff2(void *pvParameters); // For passenger

void vTempSetterTask(void *pvParameters);
void vTempReaderTask(void *pvParameters);

void vTempReaderTask2(void *pvParameters);


void vHeatControlTask(void *pvParameters);

void vTempSetterTask2(void *pvParameters);
void vHeatControlTask2(void *pvParameters);

void vDisplayTask(void *pvParameters);
void vDiagnosticTask(void *pvParameters);

void vDiagnosticTask2(void *pvParameters);

/* Definitions for the event bits in the event group. */
#define LowTempBit ( 1UL << 0UL )  /* Event bit 0 */
#define MedTempBit   ( 1UL << 1UL )  /* Event bit 1 */
#define HighTempBit   ( 1UL << 2UL )  /* Event bit 2 */

#define LowTempBit2 ( 1UL << 3UL )  /* Event bit 3 */
#define MedTempBit2   ( 1UL << 4UL )  /* Event bit 4 */
#define HighTempBit2   ( 1UL << 5UL )  /* Event bit 5 */

/* Resource Management */
SemaphoreHandle_t xButtonSemaphore;
SemaphoreHandle_t xButtonSemaphoreSw2;
xSemaphoreHandle xMutexCount;
xSemaphoreHandle xMutexCount2; // Passenger
xSemaphoreHandle xMutexUart;
EventGroupHandle_t xTempSetEventGroup; // 35, 30, 25 degrees, Event group for desired temperature setting
EventGroupHandle_t xTempSetEventGroup2; // 35, 30, 25 degrees, Event group for desired temperature setting, Passenger
QueueHandle_t xCurrentTempQueue; //Sends current temperature to temperature controller task
QueueHandle_t xInvalidTempQueue; //Sends current invalid temperature to temperature controller task
QueueHandle_t xCurrentTempQueue2; //Sends current temperature to temperature controller task, Passenger
QueueHandle_t xInvalidTempQueue2; //Sends current invalid temperature to temperature controller task, Passenger


/* Shared Resources */
int count = 0;
int count2 = 0;

int error = 0;
int error1 = 0;


/* Unshared Resources */
sint64 desiredTempDisplay = 0;
sint64 currentTempDisplay = 0;

/* Passenger */
sint64 desiredTempDisplay2 = 0;
sint64 currentTempDisplay2 = 0;


typedef enum {
    NONE,
    MODE_LOW,
    MODE_MEDIUM,
    MODE_HIGH
} Mode;
Mode currentMode = NONE;
sint64 on_off = 0;

Mode currentMode2 = NONE;
sint64 on_off2 = 0;




/* Runtime measurements */
#define RUNTIME_MEASUREMENTS_TASK_PERIODICITY (5000U)
#define LOAD1_TASK_PERIODICITY                (100U)
#define LOAD2_TASK_PERIODICITY                (50U)

#define LOAD1_TASK_EXECUTION_TIME             (50U)
#define LOAD2_TASK_EXECUTION_TIME             (10U)

void vRunTimeMeasurementsTask(void *pvParameters);
TaskHandle_t xTask0Handle; // The handle for the rtm task



uint32 ullTasksOutTime[12];
uint32 ullTasksInTime[12];
uint32 ullTasksTotalTime[12];

int main()
{


    /* Setup the hardware for use with the Tiva C board. */
    prvSetupHardware();

    /* Create Queues Here */
    xButtonSemaphore = xSemaphoreCreateBinary(); // To check the button clicks & change modes
    xButtonSemaphoreSw2 = xSemaphoreCreateBinary(); // To check the button clicks & change modes for passenger seat

    xMutexCount = xSemaphoreCreateMutex();
    xMutexCount2 = xSemaphoreCreateMutex();

    xMutexUart = xSemaphoreCreateMutex();

    xTempSetEventGroup = xEventGroupCreate(); // To determine intensity required (Low = 25, Med = 30, High = 35)
    xCurrentTempQueue = xQueueCreate(5, sizeof(uint32)); // Sends valid current temperature
    xInvalidTempQueue = xQueueCreate(3, sizeof(uint32)); // Sends invalid current temperature

    xTempSetEventGroup2 = xEventGroupCreate(); // To determine intensity required (Low = 25, Med = 30, High = 35)
    xCurrentTempQueue2 = xQueueCreate(5, sizeof(uint32)); // Sends valid current temperature
    xInvalidTempQueue2 = xQueueCreate(3, sizeof(uint32)); // Sends invalid current temperature




    /* Create Tasks here */
    // App tasks

    xTaskCreate(vHeaterOnOff, "Heater Turn ON/OFF Task", 128, NULL, 5, &xHeaterOnOffHandle);
    xTaskCreate(vHeaterOnOff2, "Heater Turn ON/OFF Task for Passenger", 128, NULL, 5, &xHeaterOnOffHandle2);
    xTaskCreate(vTempSetterTask, "TempSetter", 128, NULL, 3, &xTempSetterHandle);
    xTaskCreate(vTempReaderTask, "TempReader", 128, NULL, 4, &xTempReaderHandle); //prio = 3
    xTaskCreate(vTempReaderTask2, "TempReader", 128, NULL, 4, &xTempReaderHandle2); //prio = 3

    xTaskCreate(vHeatControlTask, "HeatControl", 128, NULL, 2, &xHeatControlHandle); //prio = 2

    xTaskCreate(vTempSetterTask2, "TempSetter", 128, NULL, 3, &xTempSetterHandle2);
    xTaskCreate(vHeatControlTask2, "HeatControl", 128, NULL, 2, &xHeatControlHandle2); //prio = 2

    xTaskCreate(vDisplayTask, "Display", 256, NULL, 1, &xDisplayHandle); //prio = 1
    xTaskCreate(vDiagnosticTask, "Diagnostic", 128, NULL, 3, &xDiagnosticHandle); //prio = 3

    xTaskCreate(vDiagnosticTask2, "Diagnostic", 128, NULL, 3, &xDiagnosticHandle2); //prio = 3

    //Run time Task
    xTaskCreate(vRunTimeMeasurementsTask, "Run time", 256, NULL, 1, &xTask0Handle);

    //Tags

    vTaskSetApplicationTaskTag( xHeaterOnOffHandle, ( TaskHookFunction_t ) 1 );
    vTaskSetApplicationTaskTag( xHeaterOnOffHandle2, ( TaskHookFunction_t ) 2 );
    vTaskSetApplicationTaskTag( xTempSetterHandle, ( TaskHookFunction_t ) 3 );
    vTaskSetApplicationTaskTag( xTempReaderHandle, ( TaskHookFunction_t ) 1 );
    vTaskSetApplicationTaskTag( xTempReaderHandle2, ( TaskHookFunction_t ) 2 );
    vTaskSetApplicationTaskTag( xHeatControlHandle, ( TaskHookFunction_t ) 3 );
    vTaskSetApplicationTaskTag( xTempSetterHandle2, ( TaskHookFunction_t ) 1 );
    vTaskSetApplicationTaskTag( xHeatControlHandle2, ( TaskHookFunction_t ) 2 );
    vTaskSetApplicationTaskTag( xDisplayHandle, ( TaskHookFunction_t ) 3 );
    vTaskSetApplicationTaskTag( xDiagnosticHandle, ( TaskHookFunction_t ) 1 );
    vTaskSetApplicationTaskTag( xDiagnosticHandle2, ( TaskHookFunction_t ) 2 );

    vTaskStartScheduler();

    /* Should never reach here!  If you do then there was not enough heap
    available for the idle task to be created. */
    for (;;);

}


static void prvSetupHardware( void )
{
    /* Place here any needed HW initialization such as GPIO, UART, etc.  */
    UART0_Init();
    GPIO_BuiltinButtonsLedsInit();
    ADC0_Init();
    GPTM_WTimer0Init();
    GPIO_SW1EdgeTriggeredInterruptInit();
    GPIO_SW2EdgeTriggeredInterruptInit();
    GPIO_PB0EdgeTriggeredInterruptInit();
}


void vRunTimeMeasurementsTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        uint8 ucCounter, ucCPU_Load;
        uint32 ullTotalTasksTime = 0;
        vTaskDelayUntil(&xLastWakeTime, RUNTIME_MEASUREMENTS_TASK_PERIODICITY);
        for(ucCounter = 1; ucCounter < 4; ucCounter++)
        {
            ullTotalTasksTime += ullTasksTotalTime[ucCounter];
        }
        ucCPU_Load = (ullTotalTasksTime * 100) /  GPTM_WTimer0Read();

        taskENTER_CRITICAL();
        xSemaphoreTake(xMutexUart, portMAX_DELAY);
        UART0_SendString("CPU Load is ");
        UART0_SendInteger(ucCPU_Load);
        UART0_SendString("% \r\n");
        xSemaphoreGive(xMutexUart);
        taskEXIT_CRITICAL();

    }
}


void vHeaterOnOff(void *pvParameters)
{


    vTaskSuspend(xTempSetterHandle);
    vTaskSuspend(xTempReaderHandle);
    vTaskSuspend(xHeatControlHandle);
    if (!on_off2) vTaskSuspend(xDisplayHandle);
    vTaskSuspend(xDiagnosticHandle);


    for(;;){
        if (xSemaphoreTake(xButtonSemaphore, portMAX_DELAY) == pdTRUE) {
            if (xSemaphoreTake(xMutexCount, portMAX_DELAY) == pdTRUE) {
                // Debounce delay: ignore any further bounces for 50 ms
                vTaskDelay(pdMS_TO_TICKS(200));

                // Drain any extra bounces that happened during the 50 ms
                while (xSemaphoreTake(xButtonSemaphore, 0) == pdTRUE) { /* nothing */ }
                count++;
                if(count==1 ){
                    xSemaphoreTake(xMutexUart, portMAX_DELAY);
                    UART0_SendString("Turning Heater ON\r\n");
                    xSemaphoreGive(xMutexUart);

                    xEventGroupClearBits(xTempSetEventGroup, LowTempBit | MedTempBit | HighTempBit);

                    // Resume tasks
                    Delay_MS(100);  // Debounce delay
                    if (!error) vTaskResume(xTempSetterHandle); //prio = 3
                    if (!error) vTaskResume(xHeatControlHandle); //prio = 2
                    if (!error) vTaskResume(xDisplayHandle); //prio = 1
                    vTaskResume(xDiagnosticHandle); //prio = 3
                    vTaskResume(xTempReaderHandle); //prio = 3
                    Delay_MS(100);  // Debounce delay
                    on_off = 1;
                    xSemaphoreGive(xMutexCount);
                    vTaskSuspend(NULL);  // Suspend self
                }

                if (count == 5){  // Task only runs if count == 5 OR 1 anyways, no need to check for count value twice
                  xSemaphoreTake(xMutexUart, portMAX_DELAY);
                  UART0_SendString("Turning Heater OFF\r\n");
                  xSemaphoreGive(xMutexUart);
                    count =0;
                    // Suspend all other tasks
                    vTaskSuspend(xTempSetterHandle);
                    vTaskSuspend(xTempReaderHandle);
                    vTaskSuspend(xHeatControlHandle);
                    if (!on_off2) vTaskSuspend(xDisplayHandle);
                    vTaskSuspend(xDiagnosticHandle);
                    GPIO_BlueLedOff();
                    GPIO_GreenLedOff();
                    on_off = 0;
                    xEventGroupClearBits( xTempSetEventGroup, LowTempBit | MedTempBit | HighTempBit );
                    desiredTempDisplay = 0;
                    currentMode = NONE;
                    desiredTempDisplay = 0;
                    currentTempDisplay = 0;
                }

            }
        }

    }
}


void vHeaterOnOff2(void *pvParameters)
{


    vTaskSuspend(xTempSetterHandle2);
    vTaskSuspend(xTempReaderHandle2);
    vTaskSuspend(xHeatControlHandle2);
    if (!on_off) vTaskSuspend(xDisplayHandle);
    vTaskSuspend(xDiagnosticHandle2);


    for(;;){

        if (xSemaphoreTake(xButtonSemaphoreSw2, portMAX_DELAY) == pdTRUE) {
            if (xSemaphoreTake(xMutexCount2, portMAX_DELAY) == pdTRUE) {
                // Debounce delay: ignore any further bounces for 50 ms
                vTaskDelay(pdMS_TO_TICKS(200));

                // Drain any extra bounces that happened during the 50 ms
                while (xSemaphoreTake(xButtonSemaphoreSw2, 0) == pdTRUE) { /* nothing */ }
                count2++;
                if(count2==1 ){
                    xSemaphoreTake(xMutexUart, portMAX_DELAY);
                    UART0_SendString("Turning Passenger Heater ON\r\n");
                    xSemaphoreGive(xMutexUart);

                    xEventGroupClearBits(xTempSetEventGroup2, LowTempBit2 | MedTempBit2 | HighTempBit2);

                    // Resume tasks
                    Delay_MS(100);  // Debounce delay
                    if (!error1) vTaskResume(xTempSetterHandle2); //prio = 3
                    if (!error1) vTaskResume(xHeatControlHandle2); //prio = 2
                    if (!error1) vTaskResume(xDisplayHandle); //prio = 1
                    vTaskResume(xDiagnosticHandle2); //prio = 3
                    vTaskResume(xTempReaderHandle2); //prio = 3
                    Delay_MS(100);  // Debounce delay
                    on_off2 = 1;
                    xSemaphoreGive(xMutexCount2);
                    vTaskSuspend(NULL);  // Suspend self
                }

                if (count2 == 5){  // Task only runs if count == 5 OR 1 anyways, no need to check for count value twice
                    xSemaphoreTake(xMutexUart, portMAX_DELAY);
                    UART0_SendString("Turning Passenger Heater OFF\r\n");
                    xSemaphoreGive(xMutexUart);
                    count2 =0;
                    // Suspend all other tasks
                    vTaskSuspend(xTempSetterHandle2);
                    vTaskSuspend(xTempReaderHandle2);
                    vTaskSuspend(xHeatControlHandle2);
                    if (!on_off) vTaskSuspend(xDisplayHandle);
                    vTaskSuspend(xDiagnosticHandle2);
                    GPIO_ExternGreenOff();
                    GPIO_ExternBlueOff();
                    on_off2 = 0;
                    xEventGroupClearBits(xTempSetEventGroup2, LowTempBit2 | MedTempBit2 | HighTempBit2);
                    currentMode2 = NONE;
                    desiredTempDisplay2 = 0;
                    currentTempDisplay2 = 0;
                }

            }
        }

    }
}


void vTempSetterTask(void *pvParameters) {
    for(;;){
        if (count == 5){
            vTaskResume(xHeaterOnOffHandle);
        }
        if ((xSemaphoreTake(xButtonSemaphore, portMAX_DELAY) == pdTRUE)) { // Button event
            if (xSemaphoreTake(xMutexCount, portMAX_DELAY) == pdTRUE) { // Resource protection of count variable
                // Debounce delay: ignore any further bounces for 120 ms
                vTaskDelay(pdMS_TO_TICKS(200));

                // Drain any extra bounces that happened during the 120 ms
                while (xSemaphoreTake(xButtonSemaphore, 0) == pdTRUE) { /* nothing */ }
                count++;
                switch (count){
                    case 2: xEventGroupSetBits(xTempSetEventGroup, LowTempBit);
                    xSemaphoreTake(xMutexUart, portMAX_DELAY);
                    UART0_SendString("\r\n25\r\n");
                    xSemaphoreGive(xMutexUart);
                            break;
                    case 3: xEventGroupSetBits(xTempSetEventGroup, MedTempBit);
                    xSemaphoreTake(xMutexUart, portMAX_DELAY);
                    UART0_SendString("\r\n30\r\n");
                    xSemaphoreGive(xMutexUart);
                            break;
                    case 4: xEventGroupSetBits(xTempSetEventGroup, HighTempBit);
                    xSemaphoreTake(xMutexUart, portMAX_DELAY);
                    UART0_SendString("\r\n35\r\n");
                    xSemaphoreGive(xMutexUart);
                        break;
                    default: break;
                }
                xSemaphoreGive(xMutexCount);
            }
        }
    }
}




void vTempSetterTask2(void *pvParameters) {
    for(;;){
        if (count2 == 5){
            vTaskResume(xHeaterOnOffHandle2);
        }
        if ((xSemaphoreTake(xButtonSemaphoreSw2, portMAX_DELAY) == pdTRUE)) { // Button event
            if (xSemaphoreTake(xMutexCount2, portMAX_DELAY) == pdTRUE) { // Resource protection of count variable
                // Debounce delay: ignore any further bounces for 120 ms
                vTaskDelay(pdMS_TO_TICKS(200));

                // Drain any extra bounces that happened during the 120 ms
                while (xSemaphoreTake(xButtonSemaphoreSw2, 0) == pdTRUE) { /* nothing */ }
                count2++;
                switch (count2){
                    case 2: xEventGroupSetBits(xTempSetEventGroup2, LowTempBit2);
                    xSemaphoreTake(xMutexUart, portMAX_DELAY);
                    UART0_SendString("\r\n25P\r\n");
                    xSemaphoreGive(xMutexUart);
                            break;
                    case 3: xEventGroupSetBits(xTempSetEventGroup2, MedTempBit2);
                    xSemaphoreTake(xMutexUart, portMAX_DELAY);
                    UART0_SendString("\r\n30P\r\n");
                    xSemaphoreGive(xMutexUart);
                            break;
                    case 4: xEventGroupSetBits(xTempSetEventGroup2, HighTempBit2);
                    xSemaphoreTake(xMutexUart, portMAX_DELAY);
                    UART0_SendString("\r\n35P\r\n");
                    xSemaphoreGive(xMutexUart);
                        break;
                    default: break;
                }
                xSemaphoreGive(xMutexCount2);
            }
        }
    }
}

void vTempReaderTask(void *pvParameters) {

    TickType_t xLastWakeTime = xTaskGetTickCount();

    for(;;){
        if (on_off){ // Driver is on
    //        uint16_t currentTemp1 = ADC_to_Temperature(ADC0_Read());
            uint16_t currentTemp1 = 20;
            if (currentTemp1< 40 && currentTemp1> 5){
                GPIO_RedLedOff();
                error = 0;
                vTaskPrioritySet( xDiagnosticHandle, 3);
                xQueueSend(xCurrentTempQueue, &currentTemp1, portMAX_DELAY);
            }
            else{
                GPIO_RedLedOn();
                error = 1;
                vTaskPrioritySet( xDiagnosticHandle, 4);
                xQueueSend(xInvalidTempQueue, &currentTemp1, portMAX_DELAY);
            }
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000)); // Works periodically every 10s, Only delay that works with potentiometer most accurately
    }
}

void vTempReaderTask2(void *pvParameters) {

    TickType_t xLastWakeTime = xTaskGetTickCount();

    for(;;){

        if (on_off2){ // Passenger is on
                //uint16_t currentTemp2 = ADC_to_Temperature(ADC0_Read());
                 uint16_t currentTemp2 = 27;
                 if (currentTemp2< 40 && currentTemp2> 5){
                     GPIO_ExternRedOff();
                     error1 = 0;
                     vTaskPrioritySet( xDiagnosticHandle2, 2);
                     xQueueSend(xCurrentTempQueue2, &currentTemp2, portMAX_DELAY);
                 }
                 else{
                     GPIO_ExternRedOn();
                     error1 = 1;
                     vTaskPrioritySet( xDiagnosticHandle2, 4);
                     xQueueSend(xInvalidTempQueue2, &currentTemp2, portMAX_DELAY);
                 }
            }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000)); // Works periodically every 10s, Only delay that works with potentiometer most accurately
    }
}

void vHeatControlTask(void *pvParameters) {
    EventBits_t xEventGroupValue;
    const EventBits_t xBitsToWaitFor = ( LowTempBit | MedTempBit | HighTempBit);
    uint16_t currentTemp = 0;
    uint16_t desiredTemp = 0;
    int16_t tempGap;

    for(;;){
        /* Block to wait for event bits to become set within the event group. */
        xEventGroupValue = xEventGroupWaitBits( xTempSetEventGroup,     /* The event group to read. */
                                                xBitsToWaitFor,  /* Bits to test. */
                                                pdTRUE,          /* Clear bits on exit if the unblock condition is met. */
                                                pdFALSE,         /* Don't Wait for all bits. */
                                                pdMS_TO_TICKS(portMAX_DELAY));  /* timeout of 15sec. */


        if (xQueueReceive(xCurrentTempQueue, &currentTemp, portMAX_DELAY) == pdTRUE && count!=1) {
            currentTempDisplay = currentTemp;
            /* Check which events are set and take an action based on it. */
            if (xEventGroupValue & LowTempBit) // Enable green, and low intensity mode
            {
                desiredTemp = 25;
            }
            else if (xEventGroupValue & MedTempBit)
            {
                desiredTemp = 30;
            }
            else if (xEventGroupValue & HighTempBit)
            {
                desiredTemp = 35;
            }
            else{
                // Code to handle no event (Do nothing)
            }

            if (desiredTemp){
                tempGap = desiredTemp - currentTemp;
                desiredTempDisplay = desiredTemp;
            }

            if (tempGap || desiredTemp==currentTemp){
                if (tempGap >= 10) { // High intensity heater
                    GPIO_RedLedOff();
                    GPIO_GreenLedOn();
                    GPIO_BlueLedOn();
                    currentMode = MODE_HIGH;
                }
                else if (tempGap > 5) { // Medium heater
                    GPIO_RedLedOff();
                    GPIO_GreenLedOff();
                    GPIO_BlueLedOn();
                    currentMode = MODE_MEDIUM;
                }
                else if (tempGap > 2) { // Low intensity heater
                    GPIO_RedLedOff();
                    GPIO_BlueLedOff();
                    GPIO_GreenLedOn();
                    currentMode = MODE_LOW;
                }
                else { // Disable heater
                    if (xSemaphoreTake(xMutexCount, portMAX_DELAY) == pdTRUE) { // Resource protection of count variable
//                        count = 5;
                        //New, trying to let user enter a higher temperature if desired
//                        count++;
                        GPIO_RedLedOff();
                        GPIO_BlueLedOff();
                        GPIO_GreenLedOff();
                        xSemaphoreTake(xMutexUart, portMAX_DELAY);
                        UART0_SendString("\r\nCurrent temperature surpasses desired temperature.\r\n");
                        xSemaphoreGive(xMutexUart);
                        xSemaphoreGive(xMutexCount);
//                        vTaskResume(xHeaterOnOffHandle);
                    }
                }
                tempGap = 0;
                desiredTemp = 0;
            }
        }

    }
}



void vHeatControlTask2(void *pvParameters) {
    EventBits_t xEventGroupValue;
    const EventBits_t xBitsToWaitFor = ( LowTempBit2 | MedTempBit2 | HighTempBit2);
    uint16_t currentTemp = 0;
    uint16_t desiredTemp = 0;
    int16_t tempGap;

    for(;;){
        /* Block to wait for event bits to become set within the event group. */
        xEventGroupValue = xEventGroupWaitBits( xTempSetEventGroup2,     /* The event group to read. */
                                                xBitsToWaitFor,  /* Bits to test. */
                                                pdTRUE,          /* Clear bits on exit if the unblock condition is met. */
                                                pdFALSE,         /* Don't Wait for all bits. */
                                                pdMS_TO_TICKS(portMAX_DELAY));  /* timeout of 15sec. */


        if (xQueueReceive(xCurrentTempQueue2, &currentTemp, portMAX_DELAY) == pdTRUE) {
            currentTempDisplay2 = currentTemp;
            /* Check which events are set and take an action based on it. */
            if (xEventGroupValue & LowTempBit2) // Enable green, and low intensity mode
            {
                desiredTemp = 25;
            }
            else if (xEventGroupValue & MedTempBit2)
            {
                desiredTemp = 30;
            }
            else if (xEventGroupValue & HighTempBit2)
            {
                desiredTemp = 35;
            }
            else{
                // Code to handle no event (Do nothing)
            }

            if (desiredTemp){
                tempGap = desiredTemp - currentTemp;
                desiredTempDisplay2 = desiredTemp;
            }

            if (tempGap || desiredTemp == currentTemp){
                if (tempGap >= 10) { // High intensity heater
                    GPIO_ExternRedOff();
                    GPIO_ExternGreenOn();
                    GPIO_ExternBlueOn();
                    currentMode2 = MODE_HIGH;
                }
                else if (tempGap > 5) { // Medium heater
                    GPIO_ExternRedOff();
                    GPIO_ExternGreenOff();
                    GPIO_ExternBlueOn();
                    currentMode2 = MODE_MEDIUM;
                }
                else if (tempGap > 2) { // Low intensity heater
                    GPIO_ExternRedOff();
                    GPIO_ExternGreenOn();
                    GPIO_ExternBlueOn();
                    currentMode2 = MODE_LOW;
                }
                else { // Disable heater
                    if (xSemaphoreTake(xMutexCount2, portMAX_DELAY) == pdTRUE) { // Resource protection of count variable
//                        count2 = 5;
//                        count2++;
                        GPIO_ExternRedOff();
                        GPIO_ExternBlueOff();
                        GPIO_ExternGreenOff();
                        xSemaphoreTake(xMutexUart, portMAX_DELAY);
                        UART0_SendString("\r\nCurrent temperature surpasses desired temperature.\r\n");
                        xSemaphoreGive(xMutexUart);
                        xSemaphoreGive(xMutexCount2);
//                        vTaskResume(xHeaterOnOffHandle2);
                    }
                }
                tempGap = 0;
                desiredTemp = 0;
            }
        }

    }
}

void vDisplayTask(void *pvParameters) {

    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint16_t lastTemp = 0;
    uint16_t lastMode = NONE;

    uint16_t lastTemp2 = 0;
    uint16_t lastMode2 = NONE;

    for(;;){
        if(count == 1) {
            /* Take the UART mutex, print, then give it back before delaying */
            if( xSemaphoreTake(xMutexUart, portMAX_DELAY) == pdTRUE ) {
                UART0_SendString("Waiting...\r\n");
                xSemaphoreGive(xMutexUart);
            }
        vTaskDelay(pdMS_TO_TICKS(6000));
        }
        if(currentTempDisplay>5 && currentTempDisplay<40 && count!=1){
            if (xSemaphoreTake(xMutexUart, portMAX_DELAY) == pdTRUE) {
                if (on_off){ //on
                    if (desiredTempDisplay && currentTempDisplay && currentMode!= NONE){
                        UART0_SendString("\r\n********************** Displaying ********************** \r\n" );
                        UART0_SendString("Heater is ON. \r\n");
                        if(lastTemp != currentTempDisplay){
                            lastTemp = currentTempDisplay;
                            UART0_SendString("UPDATE--> Current temperature is: " );
                            UART0_SendInteger(currentTempDisplay);
                            UART0_SendString(".\r\n" );
                        }
                        if(lastMode != currentMode){
                            lastMode = currentMode;
                            UART0_SendString("UPDATE--> Current mode is: " );
                            switch (currentMode){
                            case 1: UART0_SendString("Low Intensity Heating.\r\n" );
                            break;
                            case 2: UART0_SendString("Medium Intensity Heating.\r\n" );
                            break;
                            case 3: UART0_SendString("High Intensity Heating.\r\n" );
                            break;
                        }
                        }
                        UART0_SendString("Desired temperature to be achieved is: " );
                        UART0_SendInteger(desiredTempDisplay);
                        UART0_SendString(".\r\n" );
                    }
                }
                else if (!on_off){
                    UART0_SendString("Driver Heater is currently turned OFF. \r\n");
                }
            }
            xSemaphoreGive(xMutexUart);
          vTaskDelay(pdMS_TO_TICKS(5000)); // Works periodically every 5s
        }




        /* Passenger */

        if(count2 == 1) {
            if( xSemaphoreTake(xMutexUart, portMAX_DELAY) == pdTRUE ) {
//                if (currentTempDisplay2<5 || currentTempDisplay2>40) {
//                    xQueueSend(xInvalidTempQueue2, &currentTempDisplay2, portMAX_DELAY);
//                    xSemaphoreGive(xMutexUart);
//                }
//                else{
                    UART0_SendString("\r\n\r\nPassenger Heater Waiting...\r\n");
                    xSemaphoreGive(xMutexUart);
//                }
            }
            vTaskDelay(pdMS_TO_TICKS(6000));
        }
        if(currentTempDisplay2>5 && currentTempDisplay2<40 && count2!=1){
            if (xSemaphoreTake(xMutexUart, portMAX_DELAY) == pdTRUE) {
                if (on_off2){ //on
                    if (desiredTempDisplay2 && currentTempDisplay2 && currentMode2!= NONE){
                        UART0_SendString("\r\n \r\n");
                        UART0_SendString("\r\n********************** Displaying Passenger Seat ********************** \r\n" );
                        UART0_SendString("Passenger Heater is ON. \r\n");
                        if(lastTemp2 != currentTempDisplay2){
                            lastTemp2 = currentTempDisplay2;
                            UART0_SendString("UPDATE--> Passenger Current temperature is: " );
                            UART0_SendInteger(currentTempDisplay2);
                            UART0_SendString(".\r\n" );
                        }
                        if(lastMode2 != currentMode2){
                            lastMode2 = currentMode2;
                            UART0_SendString("UPDATE--> Passenger Current mode is: " );
                            switch (currentMode2){
                            case 1: UART0_SendString("Low Intensity Heating.\r\n" );
                            break;
                            case 2: UART0_SendString("Medium Intensity Heating.\r\n" );
                            break;
                            case 3: UART0_SendString("High Intensity Heating.\r\n" );
                            break;
                        }
                        }
                        UART0_SendString("Desired temperature to be achieved is: " );
                        UART0_SendInteger(desiredTempDisplay2);
                        UART0_SendString(".\r\n" );
                    }
                }
                else if (!on_off2){
                    UART0_SendString("\r\n \r\n");
                    UART0_SendString("Passenger Heater is currently turned OFF. \r\n");
                }
            }
            xSemaphoreGive(xMutexUart);
            vTaskDelay(pdMS_TO_TICKS(5000)); // Works periodically every 5s
        }

    }
}


void vDiagnosticTask(void *pvParameters) {
    uint16_t invalidTemp;

    for(;;){
        // Check driver first
            if( xQueueReceive(xInvalidTempQueue, &invalidTemp, portMAX_DELAY)==pdTRUE )
            {
                // Driver‐seat error
                xSemaphoreTake(xMutexUart, portMAX_DELAY);
                UART0_SendString("------------------ERROR------------------\r\n");
                UART0_SendString("Driver Seat INVALID TEMPERATURE: Not within range of 5-40\r\n");
                xSemaphoreGive(xMutexUart);

                count = 5;
                vTaskResume(xHeaterOnOffHandle);
            }
    }
}

void vDiagnosticTask2(void *pvParameters) {
    uint16_t invalidTemp2;

    for(;;){
        if( xQueueReceive(xInvalidTempQueue2, &invalidTemp2, portMAX_DELAY)==pdTRUE )
            {
            error1 = 1;
            // Passenger‐seat error
            xSemaphoreTake(xMutexUart, portMAX_DELAY);
            UART0_SendString("------------------ERROR------------------\r\n");
            UART0_SendString("Passenger Seat INVALID TEMPERATURE: Not within range of 5-40\r\n");
            xSemaphoreGive(xMutexUart);

            count2 = 5;
            vTaskResume(xHeaterOnOffHandle2);
            }
    }
}






