# 🚗 Seat Heater Control System – FreeRTOS on Tiva C

This project implements a real-time seat heater control system for the driver and passenger seats in a car using the Tiva C (TM4C123GH6PM) microcontroller and FreeRTOS. It features dynamic temperature regulation, hardware abstraction, real-time diagnostics, and UART-based user feedback.
🔧 Features

Heating Levels:

Off, Low (25°C), Medium (30°C), High (35°C)

Maintains ±2°C around the set temperature

Heater Intensity Simulation:

Cyan LED: High (≥10°C below target)

Blue LED: Medium (5–10°C below target)

Green LED: Low (2–5°C below target)

Off: <2°C below or above target

User Input:

2 console buttons (1 per seat)

1 steering wheel button (driver seat only)

Each button cycles through heating levels

Sensor Diagnostics:

Valid range: 5°C–40°C

Red LED indicates sensor fault

Heater disables on error; resumes on recovery

ADC Input:

LM35 sensor or potentiometer

0V–3.3V → 0°C–45°C

Only 5°C–40°C considered valid

UART Output:

Real-time status: temperature, heating level, heater state

🧠 Software Architecture
RTOS Configuration

Kernel: FreeRTOS

Total Tasks: 12

Used APIs:

vTaskDelay, vTaskDelayUntil

vTaskResume, vTaskSuspend, vTaskPrioritySet

xSemaphoreGive, xSemaphoreTake

Event groups, queues, and mutexes for resource and task management


Inter-Task Communication:
Method	-->  Purpose:
 Queues	-->  Transmit temperature readings.
 Semaphores	-->  Signal button press events for heater turn ON and OFF.
 Mutexes	   -->   Protect UART and shared data.
 Events	   -->   Manage heater logic, buttons set the desired temperature using events.

Responsiveness

Buttons use edge-triggered GPIO interrupts to minimize latency and CPU usage



🔌 Hardware Setup

Microcontroller: Tiva C TM4C123GH6PM

Temperature Input: LM35 sensor or 10k potentiometer

Output LEDs: Indicate heater intensity and diagnostic status

UART Terminal: Display current system status

📊 Diagnostics and CPU Monitoring

Sensor Fault Detection:

Out-of-range readings trigger fault state and red LED

Recovery auto-detected when sensor becomes valid again

CPU Load Measurement:

Manual runtime measurement via GPTM

Accuracy: 0.1 ms

Compared against FreeRTOS statistics (for validation only)




🧑‍💻 Author

Developed by Youssef Tamer as part of the Edges Advanced Embedded Diploma.
