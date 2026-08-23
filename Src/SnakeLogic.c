#include "SnakeLogic.h"
#include "AppTasks.h"
#include "stm32f446xx.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include <stdint.h>


//MUTEX
static StaticSemaphore_t snakeMutexBuffer;
static SemaphoreHandle_t snakeMutex = NULL;

//RANDOM NUMBER GENERATOR
static uint32_t random_seed;

//GAME VARIABLES
volatile POSITION Snake[MAX_SNAKE_LENGTH];
volatile LASTPOSITION snakelast[MAX_SNAKE_LENGTH];
uint16_t snake_length;
volatile DIRECTION directionOfSnake;
volatile ISR LastISR;
volatile FOOD Food;
uint32_t score;
volatile GAME_STATUS game_status;


//RANDOM INITIALIZATION
static void RNG_Init(void)
{
    random_seed =
        0x12345678U ^
        TIM2->CNT;
}


//RANDOM NUMBER GENERATOR FUNCTION
static uint32_t RNG_GetRandom(void)
{
    random_seed =
        (random_seed * 1664525U)
        + 1013904223U;
    return random_seed;
}


//CHECK FOOD AGAINST SNAKE
static uint8_t Food_Is_On_Snake(
    uint16_t x,
    uint16_t y
)
{
    for (
        uint16_t i = 0U;
        i < snake_length;
        i++
    )
    {
        if (
            (Snake[i].x_pos == x) &&
            (Snake[i].y_pos == y)
        )
        {
            return 1U;
        }
    }
    return 0U;
}


//GENERATE FOOD
void Food_Generate(void)
{
    uint16_t x;
    uint16_t y;
    do
    {
        x = (uint16_t)(
            RNG_GetRandom()
            % BOARD_WIDTH
        );
        y = (uint16_t)(
            RNG_GetRandom()
            % BOARD_HEIGHT
        );
    } 
    while (
        Food_Is_On_Snake(x, y)
    );

    Food.x_pos = x;
    Food.y_pos = y;
}


//FOOD INITIALIZATION
void Food_Init(void)
{
    RNG_Init();
    score = 0U;
    game_status = GAME_RUNNING;
    Food_Generate();
}


//SNAKE INITIALIZATION
void Snake_Init(void)
{
    //Create mutex
    snakeMutex =
        xSemaphoreCreateMutexStatic(
            &snakeMutexBuffer
        );

    if (snakeMutex == NULL)
    {
        while (1)
        {
        }
    }

    //Initial direction
    directionOfSnake = DIR_RIGHT;
    LastISR = ISR_right;

    //Initial snake
    Snake[0].x_pos = 40U;
    Snake[0].y_pos = 40U;

    Snake[1].x_pos = 39U;
    Snake[1].y_pos = 40U;

    Snake[2].x_pos = 38U;
    Snake[2].y_pos = 40U;

    Snake[3].x_pos = 37U;
    Snake[3].y_pos = 40U;

    snake_length = 4U;


    //Previous positions
    for (
        uint16_t i = 0U;
        i < snake_length;
        i++
    )
    {
        snakelast[i].x_pos_last =
            Snake[i].x_pos;

        snakelast[i].y_pos_last =
            Snake[i].y_pos;
    }
}


//LOCK
void Snake_Lock(void)
{
    if (snakeMutex != NULL)
    {
        xSemaphoreTake(
            snakeMutex,
            portMAX_DELAY
        );
    }
}


//UNLOCK
void Snake_Unlock(void)
{
    if (snakeMutex != NULL)
    {
        xSemaphoreGive(
            snakeMutex
        );
    }
}


//SNAPSHOT
void Snake_GetSnapshot(
    SnakeGameSnapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return;
    }
    Snake_Lock();

    snapshot->snake_length =
        snake_length;

    for (
        uint16_t i = 0U;
        i < snake_length;
        i++
    )
    {
        snapshot->snake[i] =
            Snake[i];
    }

    snapshot->food =
        Food;

    snapshot->score =
        score;

    snapshot->game_status =
        game_status;

    Snake_Unlock();
}


//WALL COLLISION
uint8_t Snake_Hit_Wall(void){
    return 0U;
}


//SELF COLLISION
uint8_t Snake_Hit_Self(void)
{
    for (
        uint16_t i = 1U;
        i < snake_length;
        i++
    )
    {
        if (
            (Snake[0].x_pos ==
             Snake[i].x_pos) &&

            (Snake[0].y_pos ==
             Snake[i].y_pos)
        )
        {
            return 1U;
        }
    }

    return 0U;
}


//FOOD COLLISION
uint8_t Snake_Ate_Food(void)
{
    return (
        (Snake[0].x_pos == Food.x_pos) &&
        (Snake[0].y_pos == Food.y_pos)
    );
}


//SNAKE MOVEMENT
void Snake_Move(void)
{
    POSITION old_tail;

    //Lock game state
    Snake_Lock();


    //Check game status
    if (game_status != GAME_RUNNING)
    {
        Snake_Unlock();
        return;
    }


    // Save old tail
    old_tail.x_pos =
        Snake[snake_length - 1U].x_pos;

    old_tail.y_pos =
        Snake[snake_length - 1U].y_pos;

    //Save previous positions
    for (
        uint16_t i = 0U;
        i < snake_length;
        i++
    )
    {
        snakelast[i].x_pos_last =
            Snake[i].x_pos;

        snakelast[i].y_pos_last =
            Snake[i].y_pos;
    }


    
    //Move body
    for (
        int i = (int)snake_length - 1;
        i > 0;
        i--
    )
    {
        Snake[i] =
            Snake[i - 1];
    }

    //Move head
    switch (directionOfSnake)
    {
        //-----------------------------------------
        case DIR_RIGHT:
            if (
                Snake[0].x_pos >=
                BOARD_WIDTH - 1U
            )
            {
                Snake[0].x_pos = 0U;
            }
            else
            {
                Snake[0].x_pos++;
            }
            break;

        //-----------------------------------------
        case DIR_LEFT:
            if (
                Snake[0].x_pos == 0U
            )
            {
                Snake[0].x_pos =
                    BOARD_WIDTH - 1U;
            }
            else
            {
                Snake[0].x_pos--;
            }
            break;

        //-----------------------------------------
        case DIR_UP:
            if (
                Snake[0].y_pos == 0U
            )
            {
                Snake[0].y_pos =
                    BOARD_HEIGHT - 1U;
            }
            else
            {
                Snake[0].y_pos--;
            }
            break;

        //------------------------------------------
        case DIR_DOWN:
            if (
                Snake[0].y_pos >=
                BOARD_HEIGHT - 1U
            )
            {
                Snake[0].y_pos = 0U;
            }
            else
            {
                Snake[0].y_pos++;
            }
            break;

        //--------------------------------------------
        default:
            break;
    }

    //Self collision
    if (Snake_Hit_Self())
    {
        game_status = GAME_OVER_SELF;
        Snake_Unlock();
        return;
    }


    
    //  Food ->score++ ,,,, speed++
    if (Snake_Ate_Food()){
       //Increase length
        if( snake_length < MAX_SNAKE_LENGTH ){
            Snake[snake_length] =
            old_tail;
            snake_length++;
        }

        //Increase score
        score++;

        //Increase snake speed
        uint32_t current_speed;
        uint32_t new_speed;
        current_speed = SnakeTimer_GetSpeed();
        if (current_speed > MIN_SNAKE_SPEED_MS)
        {
            new_speed = current_speed - SPEED_INCREMENT_MS;
            if (new_speed < MIN_SNAKE_SPEED_MS)
            {
                new_speed =MIN_SNAKE_SPEED_MS;
            }
            SnakeTimer_SetSpeed(new_speed);
        }

        //Generate new food
        Food_Generate();
    }

    //Unlock
    Snake_Unlock();
}