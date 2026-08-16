#ifndef SNAKE_LOGIC_H
#define SNAKE_LOGIC_H

#include <stdint.h>


// ============================================================
// BOARD CONFIGURATION
// ============================================================

#define BOARD_WIDTH        120U
#define BOARD_HEIGHT       90U

#define MAX_SNAKE_LENGTH   100U


// ============================================================
// DIRECTION
// ============================================================

typedef enum
{
    DIR_RIGHT,
    DIR_LEFT,
    DIR_UP,
    DIR_DOWN

} DIRECTION;


// ============================================================
// LAST ISR
// ============================================================

typedef enum
{
    ISR_right,
    ISR_left,
    ISR_up,
    ISR_down

} ISR;


// ============================================================
// POSITION
// ============================================================

typedef struct
{
    uint16_t x_pos;
    uint16_t y_pos;

} POSITION;


// ============================================================
// PREVIOUS POSITION
// ============================================================

typedef struct
{
    uint16_t x_pos_last;
    uint16_t y_pos_last;

} LASTPOSITION;


// ============================================================
// FOOD
// ============================================================

typedef struct
{
    uint16_t x_pos;
    uint16_t y_pos;

} FOOD;


// ============================================================
// GAME STATUS
// ============================================================

typedef enum
{
    GAME_RUNNING   = 0,
    GAME_OVER_WALL = 1,
    GAME_OVER_SELF = 2

} GAME_STATUS;


// ============================================================
// GLOBAL VARIABLES
// ============================================================

extern volatile POSITION Snake[MAX_SNAKE_LENGTH];

extern volatile LASTPOSITION snakelast[MAX_SNAKE_LENGTH];

extern uint16_t snake_length;

extern volatile DIRECTION directionOfSnake;

extern volatile ISR LastISR;

extern volatile FOOD Food;

extern uint32_t score;

extern volatile GAME_STATUS game_status;


// ============================================================
// INITIALIZATION
// ============================================================

void Snake_Init(void);

void Food_Init(void);


// ============================================================
// GAME MOVEMENT
// ============================================================

void Snake_Move(void);


// ============================================================
// FOOD
// ============================================================

void Food_Generate(void);


// ============================================================
// COLLISION
// ============================================================

uint8_t Snake_Hit_Wall(void);

uint8_t Snake_Hit_Self(void);

uint8_t Snake_Ate_Food(void);


// ============================================================
// COMMUNICATION
// ============================================================

void Snake_SendPacket(void);


#endif