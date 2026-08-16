# 🐍 Snake Game — STM32F446RE

A hardware-controlled Snake Game developed using the **STM32F446RE** microcontroller, where the snake is controlled using physical push buttons and the game is rendered on a PC using **Python**, **PySerial**, and **Pygame**.

The STM32 handles the core game logic, button inputs, snake movement, collision detection, food generation, score, and game state, while the PC provides the graphical interface.

---

## 🎬 Project Demonstration

Click below to watch the hardware & gameplay demo in action on YouTube:

[![Watch the Demo](https://img.youtube.com/vi/McwZJfSJrOo/maxresdefault.jpg)](https://youtu.be/McwZJfSJrOo)

---

## 📌 Project Status

### Version 1 — Bare-Metal Implementation (Current)
This is **Version 1** of the Snake Game.
The entire game is implemented using a bare-metal STM32 approach without FreeRTOS. Interrupts, timers, UART/DMA communication, GPIO, and game logic are handled directly by the application and peripheral drivers.

### Version 2 — FreeRTOS Implementation (Planned)
A future **Version 2** will integrate FreeRTOS into the project.
The objective of Version 2 is not only to improve the architecture and functionality of the game, but also to use the Snake Game as a practical platform for learning and demonstrating FreeRTOS concepts such as:
- Tasks and task scheduling
- Task priorities
- Queues
- Semaphores
- Mutexes
- Task notifications
- Software timers
- Interrupt-to-task communication
- Event groups
- Inter-task communication & Synchronization
- Tick management and delays

---

## 🎮 Features

- **Hardware-controlled Snake Game**
- **Four-directional movement:**
  - ⬆️ Up
  - ⬇️ Down
  - ⬅️ Left
  - ➡️ Right
- **Continuous snake movement**
- **Food generation & collision detection**
- **Snake growth after eating food**
- **Self-collision detection & Game Over state**
- **Score tracking & dynamic speed increase**
- **Screen boundary wrapping**
- **UART communication with DMA** (High-throughput, non-blocking telemetry transfer)
- **Python-based graphical interface** (Pygame rendering)
- **Interrupt-driven button inputs** (EXTI)

---

## 🧠 How It Works

┌──────────────────────────┐
             │       STM32F446RE        │
             │                          │
             │  GPIO Button Inputs      │
             │          │               │
             │          ▼               │
             │     EXTI Interrupts      │
             │          │               │
             │          ▼               │
             │     Snake Game Logic     │
             │          │               │
             │   ┌──────┴──────┐        │
             │   │             │        │
             │   ▼             ▼        │
             │ Snake        Food/Score  │
             │ Movement      Collision  │
             │   │                      │
             │   └──────────┐           │
             │              ▼           │
             │       USART2 + DMA       │
             └──────────────┬───────────┘
                            │
                            │ UART
                            ▼
             ┌──────────────────────────┐
             │           PC             │
             │                          │
             │       Python             │
             │      PySerial            │
             │        Pygame            │
             │                          │
             │     Game Rendering       │
             └──────────────────────────┘

1. **Input**: Button presses generate GPIO external interrupts (`EXTI`) on the STM32 to update the intended direction buffer.
2. **Game Loop**: A hardware timer updates snake coordinates periodically, tests collisions, handles food respawns, and updates scores.
3. **Telemetry via DMA**: Game frame data is streamed non-intrusively via `USART2` using DMA channels.
4. **PC Rendering**: The Python `pygame` script reads the serial packet stream and renders each game frame smoothly.

---

## 🔧 Hardware

- **Microcontroller:** STM32F446RE (Nucleo-64)
- **Input:** 4 Push Buttons connected to GPIOs with internal pull-up / pull-down and EXTI interrupts:
  | Button | Direction |
  |---|---|
  | Button 1 | ⬆️ Up |
  | Button 2 | ⬇️ Down |
  | Button 3 | ⬅️ Left |
  | Button 4 | ➡️ Right |
- **Communication:** USART2 over ST-LINK Virtual COM Port / USB-to-UART bridge.

---

## 💻 Software & Toolchain

### Embedded Side
- **Language:** Embedded C / ARM Assembly
- **Architecture:** ARM Cortex-M4 (STM32F446xx)
- **Toolchain:** `arm-none-eabi-gcc`, GNU Make, OpenOCD
- **Peripherals:** GPIO, EXTI, Timers (TIM), USART2, DMA, CMSIS Core/Device headers

### PC Side
- **Language:** Python 3
- **Libraries:** `pyserial`, `pygame`

---

## 📂 Project Structure
Snake_Game/
│
├── CMSIS/                          # ARM CMSIS Core & ST Device headers
│   ├── Device/
│   │   └── ST/
│   │       └── STM32F4xx/
│   │           └── Include/
│   └── Include/
│
├── Inc/                            # C Header files
│   ├── isr.h
│   ├── SnakeLogic.h
│   ├── timer.h
│   └── usart_DMA.h
│
├── Src/                            # C Source files
│   ├── gpio.c
│   ├── isr.c
│   ├── main.c
│   ├── SnakeLogic.c
│   ├── timer.c
│   └── usart_DMA.c
│
├── python/                         # PC Graphical & Test scripts
│   ├── dma_test.py
│   ├── serial_test.py
│   └── snake_display.py
│
├── linker.ld                       # Linker script for STM32F446RE
├── Makefile                        # Build and flash automation
├── startup_stm32f446xx.s           # Startup assembly file with vector table
└── README.md                       # Project documentation

---

## 🔄 Game Flow

Start Game
│
▼
Initialize STM32 Peripherals (GPIO, EXTI, USART2, DMA, Timer)
│
▼
Initialize Snake & Generate Food
│
▼
Snake Moves Automatically on Timer Tick
│
├── Button Press Interrupt? ──> Yes ──> Update Direction
│
├── Food Collision? ──────────> Yes ──> Increase Length & Score, Spawn Food
│
├── Self Collision? ──────────> Yes ──> Trigger Game Over State
│
└── Boundary Check ───────────> Yes ──> Wrap Around Screen
│
▼
Send Game State Packet via USART2 (DMA)
│
▼
PC (PySerial + Pygame) reads packet & updates screen display

---

## 📡 STM32 ↔ PC Communication

The STM32 periodically transmits serialized game packets through UART:
- Snake body coordinates & length
- Current movement direction
- Food coordinates
- Player score
- Game status flags (Alive, Game Over)

Using **DMA** relieves the CPU from per-byte TX overhead, allowing deterministic timing for the core logic loop.

---

## 🚀 Running the Project

### 1. Build and Flash STM32
Make sure `arm-none-eabi-gcc` and `openocd` are installed and added to your system `PATH`.

```bash
# Build the project (creates binary artifacts inside Builds/)
make all

# Flash to the connected STM32F446RE board
make flash

2. Launch Python Display
Install dependencies:

Bash
pip install pyserial pygame
Set your COM port inside python/snake_display.py (e.g., COM3), then launch:

Bash
python python/snake_display.py


🎯 Learning Objectives
STM32 Bare-Metal Register-level & HAL/CMSIS development

Nested Vector Interrupt Controller (NVIC) & EXTI line routing

Precise hardware timers configuration

USART communication using Direct Memory Access (DMA)

Circular buffers and state-machine-driven game logic

Inter-system protocol design (Embedded C ↔ Python serial parsing)

2D rendering with Pygame

🔮 Future Development — Version 2 (FreeRTOS)
Version 2 will transition from the bare-metal superloop/ISR design to a multi-tasking Real-Time Operating System (FreeRTOS):

                    FreeRTOS
                       │
        ┌──────────────┼──────────────┐
        │              │              │
        ▼              ▼              ▼
   Snake Task      Input Task     UART Task
        │              │              │
        ▼              ▼              ▼
   Game Logic      Button Data     PC Telemetry
        │
        ▼
   Game State (Food, Collision, Score, Speed)

📜 Version History
v1.0 (Current):
    Full bare-metal implementation on STM32F446RE
    EXTI push-button handling & Timer-driven movement
    DMA-driven USART2 communication
    Python Pygame frontend display

v2.0 (Planned):
    FreeRTOS multitasking architecture
    FreeRTOS Queues, Mutexes, Semaphores, and Software Timers

👨‍💻 Author
MD GULAB   