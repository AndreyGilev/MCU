import time
import serial
import matplotlib.pyplot as plt

def read_value(ser):
    while True:
        try:
            line = ser.readline().decode('ascii')
            p, t, h = map(float, line.split())
            return p, t, h
        except ValueError:
            continue


def main():
    ser = serial.Serial(port='/dev/ttyACM0', baudrate=115200, timeout=0.0)
    if ser.is_open:
        print(f"Port {ser.name} opened")
    else:
        print(f"Port {ser.name} closed")

    measure_temperature_C = []
    measure_pressure_Pa = []
    measure_humidity_percent = []
    measure_ts = []
    start_ts = time.time()

    try:
        while True:
            ser.write("get_tm\n".encode('ascii'))
            ts = time.time() - start_ts
            pressure_Pa, temp_C, humidity_percent = read_value(ser)
            measure_ts.append(ts)
            measure_pressure_Pa.append(pressure_Pa)
            measure_temperature_C.append(temp_C)
            measure_humidity_percent.append(humidity_percent)
            print(f'{pressure_Pa:.0f} Pa - {temp_C:.1f}C - {humidity_percent:.1f}% -{ts:.2f}s')
            time.sleep(0.1)

    finally:
        ser.close()
        print("Port closed")
        plt.subplot(3, 1, 1)
        plt.plot(measure_ts, measure_pressure_Pa)
        plt.title('График зависимости давления от времени')
        plt.xlabel('время, с')
        plt.ylabel('давление, Па')

        plt.subplot(3, 1, 2)
        plt.plot(measure_ts, measure_temperature_C)
        plt.title('График зависимости температуры от времени')
        plt.xlabel('время, с')
        plt.ylabel('температура, C')
        
        plt.subplot(3, 1, 3)
        plt.plot(measure_ts, measure_humidity_percent)
        plt.title('График зависимости влажности от времени')
        plt.xlabel('время, с')
        plt.ylabel('влажность, %')

        plt.tight_layout()
        plt.show()


if __name__ == "__main__":
	main()
