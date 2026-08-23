#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>


// SNAKE SPEED CONFIGURATION
//Initial speed: Snake moves every 200 ms.
#define INITIAL_SNAKE_SPEED_MS    200U

//Every food eaten makes the snake faster by this amount 
#define SPEED_INCREMENT_MS        10U


/*
 * Maximum speed.
 * Smaller period = faster snake.
 * We don't allow the snake to go below 60 ms.
 */
#define MIN_SNAKE_SPEED_MS        60U

// TIMER FUNCTIONS
void timer2_init(void);
void timer2_set_speed(uint32_t speed_ms);
uint32_t timer2_get_speed(void);


#endif