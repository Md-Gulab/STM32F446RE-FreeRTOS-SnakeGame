#ifndef USART_DMA_H
#define USART_DMA_H

#include "FreeRTOS.h"
#include "semphr.h"
#include <stdint.h>

extern SemaphoreHandle_t dmaTxSemaphore;

#define BAUD 115200U
#define USART_TX_BUFFER_SIZE 2048U

void usart_init(void);
uint8_t usart_dma_send(const char *data, uint16_t length);
uint8_t usart_dma_send_packet(const char *packet);
uint8_t usart_dma_tx_busy(void);

#endif