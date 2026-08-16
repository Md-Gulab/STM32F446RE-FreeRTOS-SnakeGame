#include "SnakeLogic.h"
#include "timer.h"
#include "stm32f446xx.h"
#include "usart_DMA.h"

#include <stdint.h>
#include <stdio.h>


// ============================================================
// SOFTWARE RANDOM NUMBER GENERATOR
// ============================================================

static uint32_t random_seed ;
static void RNG_Init(void)
{
    random_seed =
        0x12345678U ^
        TIM2->CNT;
}


// ============================================================
// PACKET SEQUENCE
// ============================================================

static uint32_t packet_sequence = 0U;


// ============================================================
// GLOBAL VARIABLES
// ============================================================

volatile POSITION Snake[MAX_SNAKE_LENGTH];

volatile LASTPOSITION snakelast[MAX_SNAKE_LENGTH];

uint16_t snake_length;

volatile DIRECTION directionOfSnake;

volatile ISR LastISR;

volatile FOOD Food;

uint32_t score;

volatile GAME_STATUS game_status;

// ============================================================
// SOFTWARE RANDOM NUMBER GENERATOR
// ============================================================

static uint32_t RNG_GetRandom(void)
{
    random_seed =
        (random_seed * 1664525U) + 1013904223U;

    return random_seed;
}


// ============================================================
// CHECK WHETHER FOOD IS ON SNAKE
// ============================================================

static uint8_t Food_Is_On_Snake(
    uint16_t x,
    uint16_t y
)
{
    for (uint16_t i = 0U; i < snake_length; i++)
    {
        if ((Snake[i].x_pos == x) &&
            (Snake[i].y_pos == y))
        {
            return 1U;
        }
    }

    return 0U;
}


// ============================================================
// GENERATE FOOD
// ============================================================

void Food_Generate(void)
{
    uint16_t x;
    uint16_t y;

    do
    {
        x = (uint16_t)
            (RNG_GetRandom() % BOARD_WIDTH);

        y = (uint16_t)
            (RNG_GetRandom() % BOARD_HEIGHT);

    } while (Food_Is_On_Snake(x, y));

    Food.x_pos = x;
    Food.y_pos = y;
}


// ============================================================
// FOOD INITIALIZATION
// ============================================================

void Food_Init(void)
{
    RNG_Init();

    score = 0U;

    game_status = GAME_RUNNING;

    Food_Generate();
}


// ============================================================
// SNAKE INITIALIZATION
// ============================================================

void Snake_Init(void)
{
    directionOfSnake = DIR_RIGHT;

    LastISR = ISR_right;


    Snake[0].x_pos = 40U;
    Snake[0].y_pos = 40U;

    Snake[1].x_pos = 39U;
    Snake[1].y_pos = 40U;

    Snake[2].x_pos = 38U;
    Snake[2].y_pos = 40U;

    Snake[3].x_pos = 37U;
    Snake[3].y_pos = 40U;


    snake_length = 4U;


    for (uint16_t i = 0U;
         i < snake_length;
         i++)
    {
        snakelast[i].x_pos_last =
            Snake[i].x_pos;

        snakelast[i].y_pos_last =
            Snake[i].y_pos;
    }
}


// ============================================================
// WALL COLLISION
//
// Wrap-around is enabled, therefore boundaries are not
// treated as collisions.
// ============================================================

uint8_t Snake_Hit_Wall(void)
{
    return 0U;
}


// ============================================================
// SELF COLLISION
// ============================================================

uint8_t Snake_Hit_Self(void)
{
    for (uint16_t i = 1U;
         i < snake_length;
         i++)
    {
        if ((Snake[0].x_pos == Snake[i].x_pos) &&
            (Snake[0].y_pos == Snake[i].y_pos))
        {
            return 1U;
        }
    }

    return 0U;
}


// ============================================================
// FOOD COLLISION
// ============================================================

uint8_t Snake_Ate_Food(void)
{
    return (
        (Snake[0].x_pos == Food.x_pos) &&
        (Snake[0].y_pos == Food.y_pos)
    );
}


// ============================================================
// SEND COMPLETE GAME PACKET
// ============================================================

void Snake_SendPacket(void)
{
    char packet[2048];

    int offset = 0;


    offset += sprintf(
        &packet[offset],
        "@SNAKE,%lu,%u",
        (unsigned long)packet_sequence++,
        (unsigned int)snake_length
    );


    for (uint16_t i = 0U;
         i < snake_length;
         i++)
    {
        offset += sprintf(
            &packet[offset],
            ",%u,%u",
            (unsigned int)Snake[i].x_pos,
            (unsigned int)Snake[i].y_pos
        );
    }


    offset += sprintf(
        &packet[offset],
        ",%u,%u",
        (unsigned int)Food.x_pos,
        (unsigned int)Food.y_pos
    );


    offset += sprintf(
        &packet[offset],
        ",%lu",
        (unsigned long)score
    );


    offset += sprintf(
        &packet[offset],
        ",%u",
        (unsigned int)game_status
    );


    offset += sprintf(
        &packet[offset],
        "\r\n"
    );


    if (!usart_dma_tx_busy())
    {
        usart_dma_send(
            packet,
            (uint16_t)offset
        );
    }
}






// ============================================================
// SNAKE MOVEMENT
// ============================================================

void Snake_Move(void)
{
    POSITION old_tail;


    if (game_status != GAME_RUNNING)
    {
        return;
    }


    // --------------------------------------------------------
    // Save old tail
    // --------------------------------------------------------

    old_tail.x_pos =
        Snake[snake_length - 1U].x_pos;

    old_tail.y_pos =
        Snake[snake_length - 1U].y_pos;


    // --------------------------------------------------------
    // Save previous positions
    // --------------------------------------------------------

    for (uint16_t i = 0U;
         i < snake_length;
         i++)
    {
        snakelast[i].x_pos_last =
            Snake[i].x_pos;

        snakelast[i].y_pos_last =
            Snake[i].y_pos;
    }


    // --------------------------------------------------------
    // Move body
    // --------------------------------------------------------

    for (int i = (int)snake_length - 1;
         i > 0;
         i--)
    {
        Snake[i] = Snake[i - 1];
    }


    // --------------------------------------------------------
    // Move HEAD
    //
    // Only the head wraps.
    // The body follows normally.
    // --------------------------------------------------------

    switch (directionOfSnake)
    {
        case DIR_RIGHT:

            if (Snake[0].x_pos >= BOARD_WIDTH - 1U)
            {
                Snake[0].x_pos = 0U;
            }
            else
            {
                Snake[0].x_pos++;
            }

            break;


        case DIR_LEFT:

            if (Snake[0].x_pos == 0U)
            {
                Snake[0].x_pos = BOARD_WIDTH - 1U;
            }
            else
            {
                Snake[0].x_pos--;
            }

            break;


        case DIR_UP:

            if (Snake[0].y_pos == 0U)
            {
                Snake[0].y_pos = BOARD_HEIGHT - 1U;
            }
            else
            {
                Snake[0].y_pos--;
            }

            break;


        case DIR_DOWN:

            if (Snake[0].y_pos >= BOARD_HEIGHT - 1U)
            {
                Snake[0].y_pos = 0U;
            }
            else
            {
                Snake[0].y_pos++;
            }

            break;


        default:
            break;
    }


    // --------------------------------------------------------
    // Self collision
    // --------------------------------------------------------

    if (Snake_Hit_Self())
    {
        game_status = GAME_OVER_SELF;

        Snake_SendPacket();

        return;
    }


    // --------------------------------------------------------
    // Food
    // --------------------------------------------------------

    if (Snake_Ate_Food())
    {
        if (snake_length < MAX_SNAKE_LENGTH)
        {
            Snake[snake_length] = old_tail;
            snake_length++;
        }

        score++;


        // ----------------------------------------------------
        // Increase speed
        // ----------------------------------------------------

        uint32_t current_speed =
            timer2_get_speed();


        if (current_speed > MIN_SNAKE_SPEED_MS)
        {
            uint32_t new_speed =
                current_speed - SPEED_INCREMENT_MS;


            if (new_speed < MIN_SNAKE_SPEED_MS)
            {
                new_speed = MIN_SNAKE_SPEED_MS;
            }
          
            //new_speed -- ;

            timer2_set_speed(new_speed);
        }


        // ----------------------------------------------------
        // Generate next food
        // ----------------------------------------------------

        Food_Generate();
    }


    // --------------------------------------------------------
    // Send state
    // --------------------------------------------------------

    Snake_SendPacket();
}