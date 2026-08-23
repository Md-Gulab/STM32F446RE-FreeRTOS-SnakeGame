//FreeRTOS includes
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <semphr.h>

// Standard includes 
#include <stdio.h>
#include <stm32f446xx.h>
#include "usart_DMA.h"
#include <stdarg.h> // for usart_printf()
#include"timer.h"
#include "AppTasks.h"
#include "usart_DMA.h"
#include "SnakeLOgic.h"

//function declarations
void gpio_dma_init(void) ;

int main(void){   
    usart_init();
    timer2_init();
    gpio_dma_init();
    Snake_Init();
    Food_Init();
    App_Tasks_Init();
    vTaskStartScheduler();
    //execution should never reach here

    while (1){
    }
}