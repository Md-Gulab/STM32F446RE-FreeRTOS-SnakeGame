# ==============================================================================
# Toolchain & Tools
# ==============================================================================

CC       = arm-none-eabi-gcc
OBJCOPY  = arm-none-eabi-objcopy
SIZE     = arm-none-eabi-size
OPENOCD  = openocd

# ==============================================================================
# Target & MCU Configuration
# ==============================================================================

TARGET   = main
MCU      = -mcpu=cortex-m4 -mthumb
FLOAT    = -mfloat-abi=soft

BUILD_DIR = Builds

# ==============================================================================
# Directory & Include Paths
# ==============================================================================

INC_DIR  = -I. \
           -I./Inc \
           -I./CMSIS/Include \
           -I./CMSIS/Device/ST/STM32F4xx/Include

# ==============================================================================
# Compiler & Linker Flags
# ==============================================================================

CFLAGS   = $(MCU) \
           $(FLOAT) \
           -DSTM32F446xx \
           -Wall \
           -O0 \
           -g \
           $(INC_DIR)

LDFLAGS  = -T linker.ld \
           -nostartfiles \
           --specs=nano.specs \
           --specs=nosys.specs

# ==============================================================================
# Source Files & Object Generation
# ==============================================================================

# C sources from Src/ directory
SRCS_C   = Src/main.c \
           Src/gpio.c \
           Src/isr.c \
           Src/usart_DMA.c \
           Src/SnakeLogic.c \
           Src/timer.c

# Startup assembly file from root directory
SRCS_S   = startup_stm32f446xx.s

# Object files mapped to Builds/ directory
OBJS     = $(BUILD_DIR)/startup_stm32f446xx.o \
           $(patsubst Src/%.c, $(BUILD_DIR)/%.o, $(SRCS_C))

# Output file paths
ELF_FILE = $(BUILD_DIR)/$(TARGET).elf
BIN_FILE = $(BUILD_DIR)/$(TARGET).bin

# ==============================================================================
# OpenOCD Settings
# ==============================================================================

OPENOCD_INTERFACE = interface/stlink.cfg
OPENOCD_TARGET    = target/stm32f4x.cfg

OPENOCD_FLAGS     = -f $(OPENOCD_INTERFACE) \
                    -f $(OPENOCD_TARGET)

# ==============================================================================
# Serial Monitor Settings (PuTTY)
# ==============================================================================

PORT ?= COM3
BAUD ?= 115200

# ==============================================================================
# Build Rules
# ==============================================================================

.PHONY: all clean flash erase monitor flash_monitor

all: $(BIN_FILE)

# Ensure the output directory exists
$(BUILD_DIR):
	@mkdir -p $@

# Link objects into ELF executable inside Builds/
$(ELF_FILE): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

# Convert ELF to Binary inside Builds/
$(BIN_FILE): $(ELF_FILE)
	$(OBJCOPY) -O binary $< $@
	$(SIZE) $<

# Compile C source files from Src/ into Builds/
$(BUILD_DIR)/%.o: Src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile startup assembly file from root into Builds/
$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(CC) $(MCU) $(FLOAT) $(INC_DIR) -c $< -o $@

# Flash the target device
flash: $(ELF_FILE)
	@echo "Flashing STM32F446RE..."
	$(OPENOCD) $(OPENOCD_FLAGS) -c "program $(ELF_FILE) verify reset exit"

# Erase flash memory
erase:
	@echo "Erasing STM32F446RE Flash..."
	$(OPENOCD) $(OPENOCD_FLAGS) -c "init" -c "reset halt" -c "stm32f4x mass_erase 0" -c "reset" -c "shutdown"

# Launch PuTTY in serial mode
monitor:
	putty -serial $(PORT) -sercfg $(BAUD),8,n,1,N &

# Delay PuTTY launch by 1 second to give ST-LINK time to release COM3
flash_monitor: flash
	powershell -Command "Start-Sleep -Seconds 1"
	$(MAKE) monitor

# Clean build artifacts and remove the Builds folder
clean:
	@echo "Cleaning project..."
	rm -rf $(BUILD_DIR)
	@echo "Clean complete."