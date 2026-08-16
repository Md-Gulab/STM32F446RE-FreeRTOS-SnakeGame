#include <stdint.h>

#include "stm32f446xx.h"
#include "usart_DMA.h"
#include "SnakeLogic.h"
#include "timer.h"


void gpio_dma_init(void);


void SystemInit(void)
{
    /*
     * Keep existing startup configuration.
     */
}


int main(void)
{
    /*
     * USART2 + DMA
     */

    usart_init();


    /*
     * Snake initialization
     */

    Snake_Init();


    /*
     * Food + score + PRNG
     */

    Food_Init();


    /*
     * GPIO + EXTI
     */

    gpio_dma_init();


    /*
     * Start automatic Snake movement.
     */

    timer2_init();


    /*
     * Send initial state.
     */

    Snake_SendPacket();


    /*
     * Sleep between interrupts.
     */

    while (1)
    {
        __asm volatile ("wfi");
    }
}