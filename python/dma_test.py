import serial
import time


PORT = "COM3"
BAUD = 115200


ser = serial.Serial(
    PORT,
    BAUD,
    timeout=1
)


print("Connected to STM32 on", PORT)

while True:

    line = ser.readline()

    if line:
        print("Received:", line.decode(
            "utf-8",
            errors="replace"
        ).rstrip())