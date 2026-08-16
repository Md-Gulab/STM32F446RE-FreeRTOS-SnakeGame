#include "usart_DMA.h"
#include "stm32f446xx.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


/* ============================================================
 * Configuration
 * ============================================================
 */

#define USART2_CLOCK_HZ    16000000U


/*
 * STM32F446:
 *
 * USART2_TX
 * DMA1
 * Stream 6
 * Channel 4
 */
#define USART2_TX_DMA_STREAM    DMA1_Stream6


/*
 * Persistent DMA TX buffer.
 *
 * DMA must continue accessing this buffer after
 * usart_dma_send() returns.
 */
static char tx_buffer[USART_TX_BUFFER_SIZE];


/*
 * Indicates whether DMA is currently transmitting.
 */
static volatile uint8_t tx_busy = 0;


/* ============================================================
 * Baud rate
 * ============================================================
 */

void set_baud_rate(uint32_t periph_clk_hz,
                   uint32_t baud_rate)
{
    uint32_t value;

    value = (periph_clk_hz + (baud_rate / 2U))
            / baud_rate;

    USART2->BRR = value;
}


/* ============================================================
 * USART2 initialization
 * ============================================================
 */

void usart_init(void)
{
    /*
     * --------------------------------------------------------
     * GPIOA clock
     * --------------------------------------------------------
     */

    RCC->AHB1ENR |= (1U << 0);


    /*
     * --------------------------------------------------------
     * PA5 LED
     *
     * Keeping your existing LED setup.
     * --------------------------------------------------------
     */

    GPIOA->MODER &= ~(3U << (2U * 5U));

    GPIOA->MODER |=
        (1U << (2U * 5U));

    GPIOA->ODR |=
        (1U << 5);


    /*
     * --------------------------------------------------------
     * PA2 = USART2_TX
     * PA3 = USART2_RX
     *
     * Alternate function mode = 10
     * --------------------------------------------------------
     */

    GPIOA->MODER &=
        ~((3U << (2U * 2U)) |
          (3U << (2U * 3U)));

    GPIOA->MODER |=
        ((2U << (2U * 2U)) |
         (2U << (2U * 3U)));


    /*
     * --------------------------------------------------------
     * AF7 = USART2
     * --------------------------------------------------------
     */

    GPIOA->AFR[0] &=
        ~((0xFU << 8U) |
          (0xFU << 12U));

    GPIOA->AFR[0] |=
        ((7U << 8U) |
         (7U << 12U));


    /*
     * --------------------------------------------------------
     * USART2 peripheral clock
     *
     * APB1ENR bit 17
     * --------------------------------------------------------
     */

    RCC->APB1ENR |=
        (1U << 17);


    /*
     * --------------------------------------------------------
     * Baud rate
     * --------------------------------------------------------
     */

    set_baud_rate(
        USART2_CLOCK_HZ,
        BAUD
    );


    /*
     * --------------------------------------------------------
     * USART2
     *
     * UE = bit 13
     * TE = bit 3
     * RE = bit 2
     * --------------------------------------------------------
     */

    USART2->CR1 |=
        ((1U << 13) |
         (1U << 3) |
         (1U << 2));


    /*
     * --------------------------------------------------------
     * DMA1 clock
     *
     * AHB1ENR bit 21
     * --------------------------------------------------------
     */

    RCC->AHB1ENR |=
        (1U << 21);


    /*
     * --------------------------------------------------------
     * Enable USART2 TX DMA request.
     *
     * USART_CR3:
     *
     * DMAT = bit 7
     * --------------------------------------------------------
     */

    USART2->CR3 |=
        (1U << 7);
}


/* ============================================================
 * DMA TX
 * ============================================================
 */

uint8_t usart_dma_send(const char *data,
                       uint16_t length)
{
    /*
     * Don't overwrite the buffer while DMA is
     * still transmitting it.
     */
    if (tx_busy)
    {
        return 0;
    }


    /*
     * Invalid length.
     */
    if (length == 0 ||
        length > USART_TX_BUFFER_SIZE)
    {
        return 0;
    }


    /*
     * Copy data into persistent DMA buffer.
     */
    memcpy(
        tx_buffer,
        data,
        length
    );


    /*
     * Mark DMA busy.
     */
    tx_busy = 1;


    /*
     * --------------------------------------------------------
     * Disable Stream 6
     * --------------------------------------------------------
     */

    USART2_TX_DMA_STREAM->CR &=
        ~(1U << 0);


    /*
     * Wait until hardware has disabled the stream.
     */

    while (USART2_TX_DMA_STREAM->CR &
           (1U << 0))
    {
    }


    /*
     * --------------------------------------------------------
     * Clear DMA Stream 6 flags.
     *
     * Stream 6 is controlled through HIFCR.
     *
     * Clear:
     *
     * CFEIF6
     * CDMEIF6
     * CTEIF6
     * CHTIF6
     * CTCIF6
     * --------------------------------------------------------
     */

    DMA1->HIFCR =
          (1U << 16)
        | (1U << 18)
        | (1U << 19)
        | (1U << 20)
        | (1U << 21);


    /*
     * --------------------------------------------------------
     * Peripheral address
     * --------------------------------------------------------
     */

    USART2_TX_DMA_STREAM->PAR =
        (uint32_t)&USART2->DR;


    /*
     * --------------------------------------------------------
     * Memory address
     * --------------------------------------------------------
     */

    USART2_TX_DMA_STREAM->M0AR =
        (uint32_t)tx_buffer;


    /*
     * --------------------------------------------------------
     * Number of bytes
     * --------------------------------------------------------
     */

    USART2_TX_DMA_STREAM->NDTR =
        length;


    /*
     * --------------------------------------------------------
     * Configure Stream 6
     *
     * Channel 4
     * DIR = memory -> peripheral
     * MINC = enabled
     * TCIE = enabled
     *
     * 8-bit peripheral
     * 8-bit memory
     * --------------------------------------------------------
     */

    USART2_TX_DMA_STREAM->CR = 0;


    /*
     * Channel selection:
     *
     * CHSEL = 100
     */
    USART2_TX_DMA_STREAM->CR |=
        (4U << 25);


    /*
     * Direction:
     *
     * DIR = 01
     *
     * Memory -> peripheral
     */
    USART2_TX_DMA_STREAM->CR |=
        (1U << 6);


    /*
     * Memory increment.
     *
     * M0AR advances after every byte.
     */
    USART2_TX_DMA_STREAM->CR |=
        (1U << 10);


    /*
     * Transfer complete interrupt.
     */
    USART2_TX_DMA_STREAM->CR |=
        (1U << 4);


    /*
     * --------------------------------------------------------
     * Enable DMA1 Stream6 interrupt in NVIC.
     * --------------------------------------------------------
     */

    NVIC_EnableIRQ(DMA1_Stream6_IRQn);


    /*
     * --------------------------------------------------------
     * Start DMA.
     * --------------------------------------------------------
     */

    USART2_TX_DMA_STREAM->CR |=
        (1U << 0);


    return 1;
}


/* ============================================================
 * Send null-terminated packet
 * ============================================================
 */

uint8_t usart_dma_send_packet(const char *packet)
{
    uint16_t length;

    length = (uint16_t)strlen(packet);

    return usart_dma_send(
        packet,
        length
    );
}


/* ============================================================
 * DMA TX busy
 * ============================================================
 */

uint8_t usart_dma_tx_busy(void)
{
    return tx_busy;
}


/* ============================================================
 * DMA1 Stream6 interrupt
 * ============================================================
 */

void DMA1_Stream6_IRQHandler(void)
{
    /*
     * Transfer complete flag for Stream6.
     *
     * HIFSR / HISR bit 21.
     */
    if (DMA1->HISR & (1U << 21))
    {
        /*
         * Clear TCIF6.
         */
        DMA1->HIFCR =
            (1U << 21);


        /*
         * Disable DMA stream.
         */
        USART2_TX_DMA_STREAM->CR &=
            ~(1U << 0);


        /*
         * DMA is now available.
         */
        tx_busy = 0;
    }
}


/* ============================================================
 * Blocking UART functions
 *
 * These are retained for debugging.
 * ============================================================
 */

void usart_write(char ch)
{
    while (!(USART2->SR & (1U << 7)))
    {
    }

    USART2->DR =
        ((uint8_t)ch);
}


void usart_write_string(char *str)
{
    while (*str != '\0')
    {
        usart_write(*str);
        str++;
    }

    /*
     * Wait for TC.
     */
    while (!(USART2->SR & (1U << 6)))
    {
    }
}


void usart_printf(const char *format, ...)
{
    char buffer[256];

    va_list args;

    va_start(args, format);

    vsnprintf(
        buffer,
        sizeof(buffer),
        format,
        args
    );

    va_end(args);

    usart_write_string(buffer);
}


char usart_read(void)
{
    while (!(USART2->SR & (1U << 5)))
    {
    }

    return (char)(USART2->DR & 0xFF);
}


/* ============================================================
 * Delay
 * ============================================================
 */

void delay(int n)
{
    for (; n > 0; n--)
    {
        for (volatile int i = 0;
             i < 3195;
             i++)
        {
        }
    }
}