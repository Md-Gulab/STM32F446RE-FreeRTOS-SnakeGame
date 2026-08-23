#include "AppTasks.h"
#include "SnakeLogic.h"
#include "usart_DMA.h"
#include "timer.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#include <stdint.h>
#include <stdio.h>

// CONFIGURATION
#define INPUT_QUEUE_LENGTH        10U
#define SNAKE_TASK_STACK_SIZE     512U
#define GAME_TASK_STACK_SIZE      256U
#define UART_TASK_STACK_SIZE      512U
#define INITIAL_SNAKE_SPEED_MS    250U

// INPUT QUEUE
static StaticQueue_t xInputQueue;
static uint8_t inputQueueStorage[INPUT_QUEUE_LENGTH * sizeof(InputEvent_t)];
QueueHandle_t inputQueue = NULL;


// TASK OBJECTS
static StaticTask_t xSnakeTaskTCB;
static StaticTask_t xGameTaskTCB;
static StaticTask_t xUARTTaskTCB;
static StackType_t uxSnakeTaskStack[SNAKE_TASK_STACK_SIZE];
static StackType_t uxGameTaskStack[GAME_TASK_STACK_SIZE];
static StackType_t uxUARTTaskStack[UART_TASK_STACK_SIZE];


// SOFTWARE TIMER
static StaticTimer_t xSnakeTimerBuffer;
static TimerHandle_t snakeTimer = NULL;
static volatile uint32_t requestedSnakeSpeedMs = 0U;
static volatile uint32_t currentSnakeSpeedMs = INITIAL_SNAKE_SPEED_MS;


// TASK HANDLES
static TaskHandle_t snakeTaskHandle = NULL;
static TaskHandle_t gameTaskHandle  = NULL;
static TaskHandle_t uartTaskHandle  = NULL;



// UART
static uint32_t packet_sequence = 0U;
static char uartPacket[1024];
static SnakeGameSnapshot_t uartSnapshot;
static char runtimeStats[1024];


// FREERTOS RUNTIME STATISTICS
#define MAX_RUNTIME_TASKS    8U
static TaskStatus_t runtimeTaskStatus[MAX_RUNTIME_TASKS];
static UBaseType_t runtimeTaskCount = 0U;
static uint32_t runtimeTotalTime = 0U;


// FUNCTION DECLARATIONS
static void SnakeTask(void *argument);
static void GameTask(void *argument);
static void UARTTask(void *argument);
static void SnakeTimerCallback(TimerHandle_t xTimer);
static UBaseType_t FreeRTOS_GetTaskStats( TaskStatus_t *statusArray, UBaseType_t arraySize, uint32_t *totalRuntime ) ;
static void CollectRuntimeStats(void);



// STACK OVERFLOW HOOK

void vApplicationStackOverflowHook(TaskHandle_t xTask,char *pcTaskName){
    (void)xTask;
    (void)pcTaskName;

    taskDISABLE_INTERRUPTS();

    while (1){
        //If stack overflow detected , stay here so the problem can be debugged
    }
}


// COLLECT FREERTOS TASK STATISTICS
static UBaseType_t FreeRTOS_GetTaskStats(TaskStatus_t *statusArray,UBaseType_t arraySize,uint32_t *totalRuntime){
    UBaseType_t taskCount;

    taskCount = uxTaskGetSystemState( statusArray, arraySize, totalRuntime );

    return taskCount;
}


// APPLICATION INITIALIZATION
void App_Tasks_Init(void){
    
    // CREATE INPUT QUEUE
    inputQueue = xQueueCreateStatic( INPUT_QUEUE_LENGTH, sizeof(InputEvent_t), inputQueueStorage, &xInputQueue );
    if (inputQueue == NULL){
        while (1){
        }
    }

    // CREATE PERIODIC SNAKE TIMER
    snakeTimer = xTimerCreateStatic( "SnakeTimer", pdMS_TO_TICKS(INITIAL_SNAKE_SPEED_MS), pdTRUE, NULL, SnakeTimerCallback, &xSnakeTimerBuffer);
    if (snakeTimer == NULL){
        while (1){
        }
    }

    // CREATE SNAKE TASK
    snakeTaskHandle = xTaskCreateStatic( SnakeTask, "Snake", SNAKE_TASK_STACK_SIZE, NULL, 3, uxSnakeTaskStack, &xSnakeTaskTCB );
    if (snakeTaskHandle == NULL){
        while (1){
        }
    }

    // CREATE GAME INPUT TASK
    gameTaskHandle = xTaskCreateStatic( GameTask, "Game", GAME_TASK_STACK_SIZE, NULL, 3, uxGameTaskStack, &xGameTaskTCB );
    if (gameTaskHandle == NULL){
        while (1){
        }
    }

    // CREATE UART TASK
    uartTaskHandle = xTaskCreateStatic(UARTTask,"UART", UART_TASK_STACK_SIZE, NULL, 2, uxUARTTaskStack, &xUARTTaskTCB );
    if (uartTaskHandle == NULL){
        while (1){
        }
    }

    // START SOFTWARE TIMER
    if (xTimerStart(snakeTimer, 0U) != pdPASS){
        while (1){
        }
    }
}



// PERIODIC TIMER CALLBACK
static void SnakeTimerCallback(TimerHandle_t xTimer){
    (void)xTimer;
    Snake_Move();
}


// CHANGE SNAKE SPEED
void SnakeTimer_SetSpeed(uint32_t speed_ms)
{
    if (speed_ms == 0U)
    {
        return;
    }

    if (speed_ms < MIN_SNAKE_SPEED_MS)
    {
        speed_ms = MIN_SNAKE_SPEED_MS;
    }

    currentSnakeSpeedMs = speed_ms;
    requestedSnakeSpeedMs = speed_ms;
}

uint32_t SnakeTimer_GetSpeed(void)
{
    return currentSnakeSpeedMs;
}

// SNAKE TASK
static void SnakeTask(void *argument)
{
    (void)argument;
    uint32_t new_speed;

    while (1)
    {
        new_speed = requestedSnakeSpeedMs;
        if (new_speed != 0U)
        {
            requestedSnakeSpeedMs = 0U;
            xTimerChangePeriod(
                snakeTimer,
                pdMS_TO_TICKS(new_speed),
                0U
            );
        }
        vTaskDelay(pdMS_TO_TICKS(10U));
    }
}


// GAME INPUT TASK
static void GameTask(void *argument)
{
    (void)argument;
    InputEvent_t event;

    while (1)
    {
        if (xQueueReceive(
                inputQueue,
                &event,
                portMAX_DELAY
            ) == pdPASS)
        {
            Snake_Lock();
            switch (event)
            {
                case INPUT_RIGHT:
                    if (directionOfSnake != DIR_LEFT)
                    {
                        directionOfSnake = DIR_RIGHT;
                        LastISR = ISR_right;
                    }
                    break;

                case INPUT_LEFT:
                    if (directionOfSnake != DIR_RIGHT)
                    {
                        directionOfSnake = DIR_LEFT;
                        LastISR = ISR_left;
                    }
                    break;

                case INPUT_UP:
                    if (directionOfSnake != DIR_DOWN)
                    {
                        directionOfSnake = DIR_UP;
                        LastISR = ISR_up;
                    }
                    break;

                case INPUT_DOWN:
                    if (directionOfSnake != DIR_UP)
                    {
                        directionOfSnake = DIR_DOWN;
                        LastISR = ISR_down;
                    }
                    break;

                default:
                    break;
            }
            Snake_Unlock();
        }
    }
}


// COLLECT FREERTOS RUNTIME STATISTICS
static void CollectRuntimeStats(void)
{
    runtimeTaskCount = uxTaskGetSystemState(
        runtimeTaskStatus,
        MAX_RUNTIME_TASKS,
        &runtimeTotalTime
    );
}

// UART TASK
static void UARTTask(void *argument)
{
    (void)argument;

    while (1)
    {
        int offset = 0;
        // Take safe snapshot
        Snake_GetSnapshot(&uartSnapshot);
        CollectRuntimeStats();

        // Packet header
        offset += snprintf(
            uartPacket + offset,
            sizeof(uartPacket) - offset,
            "@SNAKE,%lu,%u",
            (unsigned long)packet_sequence++,
            (unsigned int)uartSnapshot.snake_length
        );

        // Snake coordinates
        for (uint16_t i = 0U;
             i < uartSnapshot.snake_length;
             i++)
        {
            offset += snprintf(
                uartPacket + offset,
                sizeof(uartPacket) - offset,
                ",%u,%u",
                (unsigned int)uartSnapshot.snake[i].x_pos,
                (unsigned int)uartSnapshot.snake[i].y_pos
            );
        }

        // Food
        offset += snprintf(
            uartPacket + offset,
            sizeof(uartPacket) - offset,
            ",%u,%u",
            (unsigned int)
                uartSnapshot.food.x_pos,
            (unsigned int)
                uartSnapshot.food.y_pos
        );

        // Score
        offset += snprintf(
            uartPacket + offset,
            sizeof(uartPacket) - offset,
            ",%lu",
            (unsigned long)uartSnapshot.score
        );

        // Game status
        offset += snprintf(
            uartPacket + offset,
            sizeof(uartPacket) - offset,
            ",%u\r\n",
            (unsigned int)uartSnapshot.game_status
        );

        // FreeRTOS runtime statistics
        for (UBaseType_t i = 0U;
        i < runtimeTaskCount;
         i++)
        {
        uint32_t cpu_percent_x100 = 0U;
        if (runtimeTotalTime > 0U)
        {
        /* CPU usage in hundredths of a percent.
         * Example:
         *     206  = 2.06%
         *     315  = 3.15%
               9231 = 92.31% */
        cpu_percent_x100 = (uint32_t)((runtimeTaskStatus[i].ulRunTimeCounter * 10000ULL ) / runtimeTotalTime );
        }   

        offset += snprintf(
        uartPacket + offset,
        sizeof(uartPacket) - offset,
        "@RTOS,%s,%lu,%u,%lu,%u\r\n",
        runtimeTaskStatus[i].pcTaskName,
        (unsigned long) runtimeTaskStatus[i].ulRunTimeCounter,
        (unsigned int)runtimeTaskStatus[i].uxCurrentPriority,
        (unsigned long)cpu_percent_x100,
        (unsigned int)runtimeTaskStatus[i].usStackHighWaterMark
        );
        }

        // Send UART packet
        if (!usart_dma_tx_busy()){
        if (usart_dma_send( uartPacket, (uint16_t)offset )){
            xSemaphoreTake( dmaTxSemaphore, portMAX_DELAY ) ;
        }
        }


        // UART update rate
        vTaskDelay(pdMS_TO_TICKS(100U));
    }
}