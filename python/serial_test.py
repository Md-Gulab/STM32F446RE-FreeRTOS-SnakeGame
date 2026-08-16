import serial

ser = serial.Serial(
    port="COM3",
    baudrate=115200,
    timeout=1
)

print("Connected to STM32 on COM3")

while True:

    data = ser.readline()

    if data:
        print("Received:", data.decode("utf-8").strip())