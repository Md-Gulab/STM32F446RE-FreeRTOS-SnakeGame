#ifndef APP_TASKS_H
#define APP_TASKS_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"

#define MIN_SNAKE_SPEED_MS    100U
#define SPEED_INCREMENT_MS    25U

typedef enum
{
    INPUT_RIGHT,
    INPUT_LEFT,
    INPUT_UP,
    INPUT_DOWN
} InputEvent_t;


extern QueueHandle_t inputQueue;
void App_Tasks_Init(void);
void SnakeTimer_SetSpeed(uint32_t speed_ms);
void Game_SetRunning(void);
void Game_SetGameOver(void);
void Game_SetFoodEaten(void);
uint32_t SnakeTimer_GetSpeed(void);

#endif