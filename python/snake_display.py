import pygame
import serial


# SERIAL
PORT = "COM3"
BAUD = 115200

# BOARD
BOARD_WIDTH = 120
BOARD_HEIGHT = 90

# DISPLAY
CELL_SIZE = 8
BOARD_PIXEL_WIDTH = BOARD_WIDTH * CELL_SIZE
BOARD_PIXEL_HEIGHT = BOARD_HEIGHT * CELL_SIZE
HUD_HEIGHT = 36

# RTOS panel on the LEFT
RTOS_PANEL_WIDTH = 360
SCREEN_WIDTH = RTOS_PANEL_WIDTH + BOARD_PIXEL_WIDTH
SCREEN_HEIGHT = BOARD_PIXEL_HEIGHT + HUD_HEIGHT


# COLORS
BACKGROUND_COLOR = (20, 20, 20)
BOARD_COLOR = (8, 8, 8)
GRID_COLOR = (45, 45, 45)
BORDER_COLOR = (220, 220, 220)
SNAKE_COLOR = (0, 200, 0)
HEAD_COLOR = (0, 255, 80)
FOOD_COLOR = (255, 50, 50)
TEXT_COLOR = (255, 255, 255)
RTOS_PANEL_COLOR = (15, 15, 15)
RTOS_HEADER_COLOR = (100, 180, 255)
SEPARATOR_COLOR = (80, 80, 80)

# SERIAL CONNECTION
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


# PYGAME
pygame.init()
screen = pygame.display.set_mode(
    (SCREEN_WIDTH, SCREEN_HEIGHT)
)

pygame.display.set_caption(
    "STM32 Snake Game - FreeRTOS Monitor"
)
clock = pygame.time.Clock()


# FONTS
font = pygame.font.SysFont(
    "Consolas",
    18
)

small_font = pygame.font.SysFont(
    "Consolas",
    16
)

header_font = pygame.font.SysFont(
    "Consolas",
    20,
    bold=True
)

# GAME STATE
snake = []
food = (0, 0)
score = 0
sequence = 0
game_status = 0


# RTOS STATE
rtos_tasks = {}


# SERIAL BUFFER
rx_buffer = b""


# PARSE SNAKE PACKET
def parse_snake_packet(fields):
    global snake
    global food
    global score
    global sequence
    global game_status

    try:
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


# PARSE RTOS PACKET
# STM32 FORMAT:
# @RTOS,TASK,RUNTIME,PRIORITY,CPU_X100,HWM
# Example:
# @RTOS,Snake,275480,3,31,481
# TASK     = Snake
# RUNTIME  = 275480
# PRIORITY = 3
# CPU      = 31 / 100 = 0.31%
# HWM      = 481
#
def parse_rtos_packet(fields):
    try:
        if len(fields) < 6:
            return

        task_name = fields[1]
        runtime_counter = int(fields[2])
        priority = int(fields[3])
        cpu_percent_x100 = int(fields[4])
        stack_hwm = int(fields[5])
        rtos_tasks[task_name] = {
            "runtime": runtime_counter,
            "priority": priority,
            "cpu_x100": cpu_percent_x100,
            "stack_hwm": stack_hwm
        }

    except (ValueError, IndexError):
        pass


# PACKET PARSER
def parse_packet(line):

    try:
        fields = line.split(",")
        if len(fields) == 0:
            return

        if fields[0] == "@SNAKE":
            parse_snake_packet(fields)

        elif fields[0] == "@RTOS":
            parse_rtos_packet(fields)

    except (ValueError, IndexError):
        pass


# READ SERIAL
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

        if line:
            parse_packet(line)


# DRAW RTOS PANEL
def draw_rtos_panel():
    pygame.draw.rect(
        screen,
        RTOS_PANEL_COLOR,
        (
            0,
            0,
            RTOS_PANEL_WIDTH,
            BOARD_PIXEL_HEIGHT
        )
    )

    # Header
    header = header_font.render(
        "FreeRTOS MONITOR",
        True,
        RTOS_HEADER_COLOR
    )

    screen.blit(
        header,
        (15, 15)
    )

    # TOP SECTION
    #
    # TASK        PRI        RUNTIME        HWM
    # --------------------------------------------------------
    y = 55

    columns = [
        ("TASK", 15),
        ("PRI", 105),
        ("RUNTIME", 160),
        ("HWM", 310)
    ]

    for text_value, x_position in columns:

        header_text = small_font.render(
            text_value,
            True,
            TEXT_COLOR
        )

        screen.blit(
            header_text,
            (x_position, y)
        )

    # Separator below header
    pygame.draw.line(
        screen,
        SEPARATOR_COLOR,
        (10, 82),
        (RTOS_PANEL_WIDTH - 10, 82)
    )

    # Task order
    preferred_order = [
        "Snake",
        "Game",
        "UART",
        "Tmr Svc",
        "IDLE"
    ]

    y = 92

    for task_name in preferred_order:
        if task_name not in rtos_tasks:
            continue

        task = rtos_tasks[task_name]
        priority = task["priority"]
        runtime = task["runtime"]
        hwm = task["stack_hwm"]

        # TASK
        text = small_font.render(
            task_name,
            True,
            TEXT_COLOR
        )

        screen.blit(
            text,
            (15, y)
        )

        # PRIORITY
        text = small_font.render(
            f"{priority:>3}",
            True,
            TEXT_COLOR
        )

        screen.blit(
            text,
            (105, y)
        )

        # RUNTIME
        text = small_font.render(
            f"{runtime:>9}",
            True,
            TEXT_COLOR
        )

        screen.blit(
            text,
            (160, y)
        )

        # HWM
        text = small_font.render(
            f"{hwm:>4}",
            True,
            TEXT_COLOR
        )

        screen.blit(
            text,
            (310, y)
        )

        y += 32

    # CPU SECTION
    cpu_section_y = y + 5
    pygame.draw.line(
        screen,
        SEPARATOR_COLOR,
        (10, cpu_section_y),
        (RTOS_PANEL_WIDTH - 10, cpu_section_y)
    )

    # CPU HEADER
    cpu_header = header_font.render(
        "CPU UTILIZATION",
        True,
        RTOS_HEADER_COLOR
    )

    screen.blit(
        cpu_header,
        (15, cpu_section_y + 10)
    )

    # CPU COLUMNS
    cpu_y = cpu_section_y + 45
    for task_name in preferred_order:
        if task_name not in rtos_tasks:
            continue

        task = rtos_tasks[task_name]
        cpu_x100 = task["cpu_x100"]
        cpu_percent = cpu_x100 / 100.0

        # Task name
        text = small_font.render(
            task_name,
            True,
            TEXT_COLOR
        )

        screen.blit(
            text,
            (15, cpu_y)
        )

        # CPU percentage
        text = small_font.render(
            f"{cpu_percent:6.2f}%",
            True,
            TEXT_COLOR
        )

        screen.blit(
            text,
            (105, cpu_y)
        )
        cpu_y += 28

    # Right separator
    pygame.draw.line(
        screen,
        SEPARATOR_COLOR,
        (RTOS_PANEL_WIDTH - 1, 0),
        (
            RTOS_PANEL_WIDTH - 1,
            BOARD_PIXEL_HEIGHT
        )
    )


# DRAW BOARD
def draw_board():

    board_x = RTOS_PANEL_WIDTH

    pygame.draw.rect(
        screen,
        BOARD_COLOR,
        (
            board_x,
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
            (
                board_x + x,
                0
            ),
            (
                board_x + x,
                BOARD_PIXEL_HEIGHT
            )
        )

    for y in range(
        0,
        BOARD_PIXEL_HEIGHT + 1,
        CELL_SIZE
    ):

        pygame.draw.line(
            screen,
            GRID_COLOR,
            (
                board_x,
                y
            ),
            (
                board_x + BOARD_PIXEL_WIDTH,
                y
            )
        )

    # Outer boundary
    pygame.draw.rect(
        screen,
        BORDER_COLOR,
        (
            board_x,
            0,
            BOARD_PIXEL_WIDTH - 1,
            BOARD_PIXEL_HEIGHT - 1
        ),
        2
    )


# DRAW FOOD
def draw_food():
    x, y = food
    if not (
        0 <= x < BOARD_WIDTH and
        0 <= y < BOARD_HEIGHT
    ):
        return

    board_x = RTOS_PANEL_WIDTH
    center_x = (
        board_x
        + x * CELL_SIZE
        + CELL_SIZE // 2
    )
    center_y = (
        y * CELL_SIZE
        + CELL_SIZE // 2
    )
    radius = max(
        3,
        CELL_SIZE // 2 - 1
    )
    pygame.draw.circle(
        screen,
        FOOD_COLOR,
        (
            center_x,
            center_y
        ),
        radius
    )
    pygame.draw.circle(
        screen,
        (255, 255, 255),
        (
            center_x,
            center_y
        ),
        radius,
        1
    )

# DRAW SNAKE
def draw_snake():
    board_x = RTOS_PANEL_WIDTH
    for index, (x, y) in enumerate(snake):
        if not (
            0 <= x < BOARD_WIDTH and
            0 <= y < BOARD_HEIGHT
        ):
            continue

        rect = pygame.Rect(
            board_x + x * CELL_SIZE,
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


# DRAW GAME INFORMATION
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
            RTOS_PANEL_WIDTH + 10,
            BOARD_PIXEL_HEIGHT + 6
        )
    )


# MAIN LOOP
running = True

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    read_serial()
    screen.fill(
        BACKGROUND_COLOR
    )

    draw_rtos_panel()
    draw_board()
    draw_food()
    draw_snake()
    draw_information()
    pygame.display.flip()
    clock.tick(60)


# CLEANUP
ser.close()
pygame.quit()