# 🚗 Seat Heater Control System – FreeRTOS on Tiva C

This project implements a **real-time seat heater control system** for the **driver and passenger seats** in a car using the **Tiva C (TM4C123GH6PM)** microcontroller and **FreeRTOS**.  
It features dynamic temperature regulation, hardware abstraction, real-time diagnostics, and UART-based user feedback.

---

## 🔧 Features

### Heating Levels
- Off  
- Low: **25°C**  
- Medium: **30°C**  
- High: **35°C**  
- Maintains **±2°C** around the selected level

### Heater Intensity Simulation
- 🟩 **Green LED**: Low (2–5°C below target)  
- 🔵 **Blue LED**: Medium (5–10°C below target)  
- 🔷 **Cyan LED**: High (≥10°C below target)  
- ❌ Heater turns **Off** if within 2°C or above target

### User Input
- 2 console buttons (1 per seat)  
- 1 steering wheel button (driver seat only)  
- Each button cycles: **Off → Low → Medium → High → On → Off**

### Sensor Diagnostics
- Valid sensor range: **5°C to 40°C**  
- ❗ **Red LED** indicates sensor fault  
- Heater disables on error, resumes automatically on recovery

### ADC Input
- LM35 or 10k potentiometer  
- 0V–3.3V → 0°C–45°C  
- Only **5°C–40°C** considered valid

### UART Output
- Displays real-time:
  - Current temperature
  - Heating level
  - Heater intensity/state

---

## 🧠 Software Architecture

### RTOS Configuration
- **Kernel**: FreeRTOS  
- **Total Tasks**: 12  
- **APIs Used**:
  - `vTaskDelay`, `vTaskDelayUntil`  
  - `vTaskResume`, `vTaskSuspend`, `vTaskPrioritySet`  
  - `xSemaphoreGive`, `xSemaphoreTake`  
  - Queues, mutexes, event groups  

### Inter-Task Communication

| Method      | Purpose                                       |
|-------------|-----------------------------------------------|
| Queues      | Transmit temperature readings                 |
| Semaphores  | Signal button press events                    |
| Mutexes     | Protect UART and shared resources             |
| EventGroups | Manage heater state and temperature changes   |

### Responsiveness
- Buttons handled via **edge-triggered GPIO interrupts**  
- Minimizes latency and reduces CPU usage

---

## 🔌 Hardware Setup

- **MCU**: Tiva C TM4C123GH6PM  
- **Temp Input**: LM35 sensor or 10k potentiometer  
- **LEDs**: Show heater intensity + diagnostic state  
- **UART Terminal**: Displays live system info  

---

## 📊 Diagnostics and CPU Monitoring

### Sensor Fault Detection
- Invalid temperature → red LED on + heater disabled  
- Auto-recovery once sensor gives valid readings again

### CPU Load Measurement
- Measured via GPTM manually (0.1 ms resolution)  
- Compared against FreeRTOS runtime stats (for validation)

---

## 👨‍💻 Author

Developed by **Youssef Tamer**  
Part of the **Edges Advanced Embedded Diploma**
