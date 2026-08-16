#include "stm32f446xx.h"
#include "SnakeLogic.h"


// ============================================================
// RIGHT
// ============================================================

void EXTI0_IRQHandler(void)
{
    if (EXTI->PR & (1U << 0))
    {
        /*
         * Clear interrupt.
         */

        EXTI->PR = (1U << 0);


        /*
         * RIGHT is illegal only when currently moving LEFT.
         */

        if (directionOfSnake != DIR_LEFT)
        {
            directionOfSnake = DIR_RIGHT;
            LastISR = ISR_right;
        }
    }
}


// ============================================================
// LEFT
// ============================================================

void EXTI1_IRQHandler(void)
{
    if (EXTI->PR & (1U << 1))
    {
        /*
         * Clear interrupt.
         */

        EXTI->PR = (1U << 1);


        /*
         * LEFT is illegal only when currently moving RIGHT.
         */

        if (directionOfSnake != DIR_RIGHT)
        {
            directionOfSnake = DIR_LEFT;
            LastISR = ISR_left;
        }
    }
}


// ============================================================
// UP
// ============================================================

void EXTI4_IRQHandler(void)
{
    if (EXTI->PR & (1U << 4))
    {
        /*
         * Clear interrupt.
         */

        EXTI->PR = (1U << 4);


        /*
         * UP is illegal only when currently moving DOWN.
         */

        if (directionOfSnake != DIR_DOWN)
        {
            directionOfSnake = DIR_UP;
            LastISR = ISR_up;
        }
    }
}


// ============================================================
// DOWN
// ============================================================

void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR & (1U << 5))
    {
        /*
         * Clear interrupt.
         */

        EXTI->PR = (1U << 5);


        /*
         * DOWN is illegal only when currently moving UP.
         */

        if (directionOfSnake != DIR_UP)
        {
            directionOfSnake = DIR_DOWN;
            LastISR = ISR_down;
        }
    }
}