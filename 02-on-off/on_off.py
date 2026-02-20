import serial, sys
serialName = '/dev/ttyACM0'
serialBaudRate = 115200
ser = serial.Serial(serialName, baudrate=serialBaudRate)

print('start')

try:
    #ser.open()
    while True:
        symbol = input()
        if symbol == "e" or symbol == "d":
            ser.write(symbol.encode())
        elif symbol == "v":
            ser.write(symbol.encode())
            mes1 = ser.readline()
            mes2 = ser.readline()
            print(mes1 + mes2)
        else:
            ser.write(symbol.encode())
            mes = ser.readline()
            print(mes)

finally:
    ser.close()
