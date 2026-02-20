#include "main.h"
#include "led/led.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/stdio.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"


int main(){
    stdio_init_all();
    led_init();
    while(1){
        char symbol = getchar();
        switch(symbol)
        {
            case 'e':
	            led_enable();
                break;
            case 'd':
				led_disable();
                break;
            case 'v':
                printf("Device name: '%s'\n", DEVICE_NAME);
                printf("Version: '%s'\n",  DEVICE_VRSN);
                break;
            default:
                printf("Введен неподдерживаемый символ: %d\n", symbol);
                break;
        }
        
    }
}