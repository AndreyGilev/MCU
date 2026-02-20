import serial, sys, select
import time
serialName = '/dev/ttyACM0'
serialBaudRate = 115200
ser = serial.Serial(serialName, baudrate=serialBaudRate)

print('start')
T = 1
e = 'e'
d = 'd'
try:
    #ser.open()
    while True:
        print("Введите период мигания: ")
        rlist, _, _ = select.select([sys.stdin], [], [], 0)
        if rlist:
            T = sys.stdin.readline().strip()
        ser.write(e.encode())
        time.sleep(T/2)
        ser.write(d.encode())
        time.sleep(T/2)

finally:
    ser.write(d.encode())
    ser.close()