#ifndef USART_DMA_H
#define USART_DMA_H

#include <stdint.h>

#define BAUD 115200U

#define USART_TX_BUFFER_SIZE 2048U

void usart_init(void);

uint8_t usart_dma_send(const char *data, uint16_t length);

uint8_t usart_dma_send_packet(const char *packet);

uint8_t usart_dma_tx_busy(void);

void usart_write(char ch);
void usart_write_string(char *str);
void usart_printf(const char *format, ...);

char usart_read(void);

void delay(int n);

#endif