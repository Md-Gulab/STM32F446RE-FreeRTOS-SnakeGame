import pygame
import serial


# ============================================================
# SERIAL
# ============================================================

PORT = "COM3"
BAUD = 115200


# ============================================================
# BOARD
# ============================================================

BOARD_WIDTH = 120
BOARD_HEIGHT = 90


# ============================================================
# DISPLAY
# ============================================================

CELL_SIZE = 8

BOARD_PIXEL_WIDTH = BOARD_WIDTH * CELL_SIZE
BOARD_PIXEL_HEIGHT = BOARD_HEIGHT * CELL_SIZE

HUD_HEIGHT = 36

SCREEN_WIDTH = BOARD_PIXEL_WIDTH
SCREEN_HEIGHT = BOARD_PIXEL_HEIGHT + HUD_HEIGHT


# ============================================================
# COLORS
# ============================================================

BACKGROUND_COLOR = (20, 20, 20)

BOARD_COLOR = (8, 8, 8)

GRID_COLOR = (45, 45, 45)

BORDER_COLOR = (220, 220, 220)

SNAKE_COLOR = (0, 200, 0)

HEAD_COLOR = (0, 255, 80)

FOOD_COLOR = (255, 50, 50)

TEXT_COLOR = (255, 255, 255)


# ============================================================
# SERIAL CONNECTION
# ============================================================

try:

    ser = serial.Serial(
        PORT,
        BAUD,
        timeout=0
    )

    print(f"Connected to STM32 on {PORT}")

except serial.SerialException as e:

    print("Could not open COM3:")
    print(e)

    raise SystemExit


# ============================================================
# PYGAME
# ============================================================

pygame.init()

screen = pygame.display.set_mode(
    (SCREEN_WIDTH, SCREEN_HEIGHT)
)

pygame.display.set_caption(
    "STM32 Snake Game"
)

clock = pygame.time.Clock()

font = pygame.font.SysFont(
    "Arial",
    20
)


# ============================================================
# GAME STATE
# ============================================================

snake = []

food = (0, 0)

score = 0

sequence = 0

game_status = 0


# ============================================================
# SERIAL BUFFER
# ============================================================

rx_buffer = b""


# ============================================================
# PACKET PARSER
# ============================================================

def parse_packet(line):

    global snake
    global food
    global score
    global sequence
    global game_status

    try:

        fields = line.split(",")


        if len(fields) < 4:
            return


        if fields[0] != "@SNAKE":
            return


        new_sequence = int(fields[1])

        snake_length = int(fields[2])


        expected_fields = (
            3
            + (snake_length * 2)
            + 4
        )


        if len(fields) < expected_fields:
            return


        new_snake = []

        index = 3


        for _ in range(snake_length):

            x = int(fields[index])

            y = int(fields[index + 1])

            new_snake.append((x, y))

            index += 2


        food_x = int(fields[index])

        food_y = int(fields[index + 1])

        index += 2


        new_score = int(fields[index])

        index += 1


        new_status = int(fields[index])


        sequence = new_sequence

        snake = new_snake

        food = (
            food_x,
            food_y
        )

        score = new_score

        game_status = new_status


    except (ValueError, IndexError):

        pass


# ============================================================
# READ SERIAL
# ============================================================

def read_serial():

    global rx_buffer

    waiting = ser.in_waiting


    if waiting == 0:
        return


    rx_buffer += ser.read(waiting)


    while b"\r\n" in rx_buffer:

        packet, rx_buffer = \
            rx_buffer.split(
                b"\r\n",
                1
            )


        line = packet.decode(
            "utf-8",
            errors="replace"
        ).strip()


        if line.startswith("@SNAKE"):

            parse_packet(line)


# ============================================================
# DRAW BOARD
# ============================================================

def draw_board():

    pygame.draw.rect(
        screen,
        BOARD_COLOR,
        (
            0,
            0,
            BOARD_PIXEL_WIDTH,
            BOARD_PIXEL_HEIGHT
        )
    )


    # Grid

    for x in range(
        0,
        BOARD_PIXEL_WIDTH + 1,
        CELL_SIZE
    ):

        pygame.draw.line(
            screen,
            GRID_COLOR,
            (x, 0),
            (x, BOARD_PIXEL_HEIGHT)
        )


    for y in range(
        0,
        BOARD_PIXEL_HEIGHT + 1,
        CELL_SIZE
    ):

        pygame.draw.line(
            screen,
            GRID_COLOR,
            (0, y),
            (BOARD_PIXEL_WIDTH, y)
        )


    # Outer boundary

    pygame.draw.rect(
        screen,
        BORDER_COLOR,
        (
            0,
            0,
            BOARD_PIXEL_WIDTH - 1,
            BOARD_PIXEL_HEIGHT - 1
        ),
        2
    )


# ============================================================
# DRAW FOOD
# ============================================================

def draw_food():

    x, y = food

    if not (0 <= x < BOARD_WIDTH and 0 <= y < BOARD_HEIGHT):
        return

    center_x = x * CELL_SIZE + CELL_SIZE // 2
    center_y = y * CELL_SIZE + CELL_SIZE // 2

    radius = max(3, CELL_SIZE // 2 - 1)

    pygame.draw.circle(
        screen,
        FOOD_COLOR,
        (center_x, center_y),
        radius
    )

    pygame.draw.circle(
        screen,
        (255, 255, 255),
        (center_x, center_y),
        radius,
        1
    )


# ============================================================
# DRAW SNAKE
# ============================================================

def draw_snake():

    for index, (x, y) in enumerate(snake):

        if not (
            0 <= x < BOARD_WIDTH and
            0 <= y < BOARD_HEIGHT
        ):
            continue


        rect = pygame.Rect(
            x * CELL_SIZE,
            y * CELL_SIZE,
            CELL_SIZE,
            CELL_SIZE
        )


        if index == 0:

            pygame.draw.rect(
                screen,
                HEAD_COLOR,
                rect
            )

        else:

            pygame.draw.rect(
                screen,
                SNAKE_COLOR,
                rect
            )


# ============================================================
# DRAW HUD
# ============================================================

def draw_information():

    if game_status == 0:
        status = "RUNNING"
    elif game_status == 1:
        status = "GAME OVER - WALL"
    else:
        status = "GAME OVER - SELF"


    text = font.render(
        f"Score: {score}   "
        f"Length: {len(snake)}   "
        f"Food: {food}   "
        f"Status: {status}",
        True,
        TEXT_COLOR
    )


    screen.blit(
        text,
        (
            10,
            BOARD_PIXEL_HEIGHT + 6
        )
    )


# ============================================================
# MAIN LOOP
# ============================================================

running = True


while running:

    for event in pygame.event.get():

        if event.type == pygame.QUIT:

            running = False


    read_serial()


    screen.fill(
        BACKGROUND_COLOR
    )


    draw_board()

    draw_food()

    draw_snake()

    draw_information()


    pygame.display.flip()


    clock.tick(60)


# ============================================================
# CLEANUP
# ============================================================

ser.close()

pygame.quit()