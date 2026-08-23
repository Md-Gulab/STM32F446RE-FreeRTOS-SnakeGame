# STM32 FreeRTOS Snake Game

A real-time Snake Game implemented on the **STM32F446RE** using **FreeRTOS**, with a Python/Pygame-based GUI for game visualization and real-time RTOS monitoring.

This project was developed to explore practical **Embedded C, STM32, FreeRTOS, RTOS task management, inter-task communication, interrupts, timers, UART/DMA communication, and runtime analysis**.

---

## Features

* Snake game running on STM32F446RE
* FreeRTOS-based multitasking
* Automatic snake movement using a FreeRTOS software timer
* GPIO interrupts for:

  * Up
  * Down
  * Left
  * Right
* Random food generation
* Snake growth and speed increase after eating food
* Wall wrapping
* Collision detection
* Score tracking
* UART communication using DMA
* Real-time game telemetry sent to PC
* Python/Pygame graphical interface
* Real-time FreeRTOS task monitoring
* Task priority monitoring
* CPU utilization monitoring
* Runtime counter monitoring
* Stack High Water Mark monitoring

---

## System Architecture

```text
                 STM32F446RE
                     │
             ┌───────┴────────┐
             │    FreeRTOS    │
             └───────┬────────┘
                     │
       ┌─────────────┼─────────────┐
       │             │             │
    Snake Task    Game Task    UART Task
       │             │             │
       └─────────────┼─────────────┘
                     │
                 UART + DMA
                     │
                     ▼
                PC / COM Port
                     │
                     ▼
              Python + Pygame
                     │
          ┌──────────┴──────────┐
          │                     │
      Snake GUI           RTOS Monitor
```

---

## FreeRTOS Tasks

The project uses multiple FreeRTOS components to demonstrate RTOS-based application design.

| Task          | Purpose                                          |
| ------------- | ------------------------------------------------ |
| Snake         | Controls automatic snake movement and game logic |
| Game          | Handles game-related processing                  |
| UART          | Handles telemetry communication with the PC      |
| Timer Service | FreeRTOS software timer management               |
| IDLE          | FreeRTOS idle task                               |

The Python GUI displays runtime information for these tasks, including:

* Task priority
* CPU utilization
* Runtime counter
* Stack High Water Mark

---

## Communication Protocol

The STM32 sends structured packets over UART.

### Snake telemetry

```text
@SNAKE,SEQ,LENGTH,X1,Y1,X2,Y2,...,FOOD_X,FOOD_Y,SCORE,STATUS
```

### RTOS telemetry

```text
@RTOS,TASK,RUNTIME,PRIORITY,CPU_X100,HWM
```

Example:

```text
@RTOS,Snake,275480,3,31,481
```

The Python application parses these packets and updates the GUI in real time.

---

## Hardware

* **Microcontroller:** STM32F446RE
* GPIO push buttons for directional control
* USB/UART connection to PC
* UART with DMA for telemetry transmission

---

## Software

* Embedded C
* STM32
* FreeRTOS
* Python
* Pygame
* PySerial
* UART
* DMA
* GPIO interrupts

---

## RTOS Monitoring

The Python GUI provides a real-time FreeRTOS monitoring panel.

Example:

```text
TASK        PRI       RUNTIME       HWM
-----------------------------------------
Snake        3         275480       481
Game         3           1129       221
UART         2        3698075       398
Tmr Svc      2          56980       203
IDLE         0       82370620       105

-----------------------------------------
CPU UTILIZATION

Snake        0.31%
Game         0.00%
UART         4.27%
Tmr Svc      0.06%
IDLE        95.28%
```

This provides a simple way to observe **task scheduling and system runtime behavior** while the application is running.

---

## How to Run

### 1. Build and flash the STM32 project

Build the STM32 project using your preferred STM32 development environment and flash it to the STM32F446RE.

### 2. Connect the STM32 to the PC

Connect the board through the configured UART interface.

Connect the 4 GPIO buttons

Update the COM port in the Python application if required:

```python
PORT = "COM3"
BAUD = 115200
```

### 3. Install Python dependencies

```bash
pip install pygame pyserial
```

### 4. Run the GUI

```bash
python .\python\snake_display.py    AFTER running this :-  .\.venv\Scripts\Activate.ps1  if dependencies are installed
```

The Pygame window will display the Snake game along with the FreeRTOS monitoring panel.

---

## What This Project Demonstrates

This project was primarily developed as a practical exercise in:

* Embedded C programming
* STM32 peripheral programming
* FreeRTOS task scheduling
* Task priorities
* Software timers
* Inter-task communication
* Interrupt handling
* UART communication
* DMA
* Real-time telemetry
* CPU/runtime analysis
* Stack usage monitoring
* PC-to-microcontroller integration

---

## Author

**MD GULAB**

Embedded Systems | STM32 | FreeRTOS | Embedded C
