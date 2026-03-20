from PIL import Image
import time
import serial
import numpy as np

image = Image.open("/home/andrew0808/Загрузки/get.jpg")
width, height = image.size
image = image.resize((320,240))
width, height = image.size
img = np.array(image)
def main():
    ser = serial.Serial(port='/dev/ttyACM0', baudrate=115200, timeout=0.0)
    if ser.is_open:
        print(f"Port {ser.name} opened")
    else:
        print(f"Port {ser.name} closed")

    try:
         for i in range(len(img)):
            for j in range(len(img[i])):
                cmd = "disp_px " + hex(img[i][j][0]) + hex(img[i][j][1])[2:] + hex(img[i][j][2])[2:] + " " + str(j) + " " + str(i) + "\n"
                
                ser.write(cmd.encode('ascii'))
    finally:
        time.sleep(0.1)
        ser.close()
        print("Port closed")

if __name__ == "__main__":
	main()
