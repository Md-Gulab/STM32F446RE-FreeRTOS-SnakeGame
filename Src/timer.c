#include "stm32f446xx.h"
#include "timer.h"
#include "SnakeLogic.h"


// CURRENT SNAKE SPEED
static volatile uint32_t current_speed_ms = INITIAL_SNAKE_SPEED_MS;

// TIM2 INITIALIZATION
void timer2_init(void)
{
    
    //Enable TIM2 clock.TIM2 is on APB1.
    RCC->APB1ENR |= (1U << 0);

    //Disable TIM2 during configuration.
    TIM2->CR1 &= ~(1U << 0);


    /*
     * Current project clock:
     * 16 MHz
     * 16,000,000 / 16,000 = 1,000 Hz
     * Therefore:
     * 1 timer count = 1 ms
     */
    TIM2->PSC = 15999U;

    //Initial snake speed.
    TIM2->ARR = current_speed_ms - 1U;
    
    //Reset counter
    TIM2->CNT = 0U;

    //Force update event.
    //This makes the PSC/ARR values take effect
    TIM2->EGR |= (1U << 0);

    //Clear update flag.
    TIM2->SR &= ~(1U << 0);

}

// SET SNAKE SPEED
void timer2_set_speed(uint32_t speed_ms)
{
    
    //Don't allow zero
    if (speed_ms == 0U)
    {
        return;
    }

    //Enforce minimum speed
    if (speed_ms < MIN_SNAKE_SPEED_MS)
    {
        speed_ms = MIN_SNAKE_SPEED_MS;
    }
    current_speed_ms = speed_ms;


    /*
     * ARR = period - 1
     * Since the timer counter is running at 1 kHz,
     * 1 count = 1 ms.
     */
    TIM2->ARR = current_speed_ms - 1U;

    /*
     * Reset counter so that the new speed
     * starts cleanly.
     */
    TIM2->CNT = 0U;

    //Force update event.
    TIM2->EGR |= (1U << 0);
}

// GET CURRENT SPEED
uint32_t timer2_get_speed(void)
{
    return current_speed_ms;
}

// TIM2 INTERRUPT HANDLER
void TIM2_IRQHandler(void)
{
    //Check update interrupt flag.
    if (TIM2->SR & (1U << 0))
    {
        //Clear UIF.
        TIM2->SR &= ~(1U << 0);
    }
}