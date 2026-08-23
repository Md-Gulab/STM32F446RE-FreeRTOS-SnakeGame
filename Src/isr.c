#include "stm32f446xx.h"
#include "SnakeLogic.h"
#include "AppTasks.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

// RIGHT
void EXTI0_IRQHandler(void){
    if (EXTI->PR & (1U << 0))
    {
        EXTI->PR = (1U << 0);
        InputEvent_t event = INPUT_RIGHT;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(
            inputQueue,
            &event,
            &xHigherPriorityTaskWoken
        );
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


// LEFT
void EXTI1_IRQHandler(void)
{
    if (EXTI->PR & (1U << 1))
    {
        EXTI->PR = (1U << 1);
        InputEvent_t event = INPUT_LEFT;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(
            inputQueue,
            &event,
            &xHigherPriorityTaskWoken
        );
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


// UP
void EXTI4_IRQHandler(void)
{
    if (EXTI->PR & (1U << 4))
    {
        EXTI->PR = (1U << 4);
        InputEvent_t event = INPUT_UP;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(
            inputQueue,
            &event,
            &xHigherPriorityTaskWoken
        );
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


// DOWN
void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR & (1U << 5))
    {
        EXTI->PR = (1U << 5);
        InputEvent_t event = INPUT_DOWN;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(
            inputQueue,
            &event,
            &xHigherPriorityTaskWoken
        );
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}