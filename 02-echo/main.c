#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/stdio.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

const uint LED_PIN = 25;

int main(){
    stdio_init_all();
    while(1){
        char symbol = getchar();
        printf("ASCII код: %d\n", symbol);
        sleep_ms(10);
        
    }
}