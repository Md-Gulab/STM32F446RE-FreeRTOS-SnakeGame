// PA0->RIGHT
// PA1->LEFT
// PA4->UP
// PA5->DOWN

#include "stm32f446xx.h"

// this function will initialise all the user buttons as the interrupt input


void gpio_dma_init(void) {
    // 1. Enable GPIOA Clock
    RCC->AHB1ENR |= (1U << 0);

    // 2. Set PA0, PA1, PA4, PA5 as Input Mode (00b)
    GPIOA->MODER &= ~((3U << (0 * 2)) | (3U << (1 * 2)) | (3U << (4 * 2)) | (3U << (5 * 2)));

    // 3. Enable Internal Pull-Up Resistors (01b)
    GPIOA->PUPDR &= ~((3U << (0 * 2)) | (3U << (1 * 2)) | (3U << (4 * 2)) | (3U << (5 * 2)));
    GPIOA->PUPDR |=  ((1U << (0 * 2)) | (1U << (1 * 2)) | (1U << (4 * 2)) | (1U << (5 * 2)));

    // 4. Enable SYSCFG Clock for EXTI Mapping
    RCC->APB2ENR |= (1U << 14);

    // 5. Connect EXTI0, EXTI1, EXTI4, EXTI5 to Port A
    SYSCFG->EXTICR[0] &= ~((0xFU << 0) | (0xFU << 4)); // EXTI0 (PA0), EXTI1 (PA1)
    SYSCFG->EXTICR[1] &= ~((0xFU << 0) | (0xFU << 4)); // EXTI4 (PA4), EXTI5 (PA5)

    // 6. Trigger on Falling Edge (Button press grounds the pin)
    EXTI->FTSR |= ((1U << 0) | (1U << 1) | (1U << 4) | (1U << 5));
    EXTI->RTSR &= ~((1U << 0) | (1U << 1) | (1U << 4) | (1U << 5));

    // 7. Unmask Interrupt Lines
    EXTI->IMR |= ((1U << 0) | (1U << 1) | (1U << 4) | (1U << 5));
    // 8. Configure Interrupt Priorities
    NVIC_SetPriority(EXTI0_IRQn, 5);
    NVIC_SetPriority(EXTI1_IRQn, 5);
    NVIC_SetPriority(EXTI4_IRQn, 5);
    NVIC_SetPriority(EXTI9_5_IRQn, 5);

    // 9. Enable Interrupts in NVIC
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);
    NVIC_EnableIRQ(EXTI4_IRQn);
    NVIC_EnableIRQ(EXTI9_5_IRQn);

    // 9. Enable Global Interrupts (Cortex-M Primask)
    __asm volatile ("cpsie i" : : : "memory");

}