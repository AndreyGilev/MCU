#include "stdio-task/stdio-task.h"
#include "protocol-task.h"
#include "led-task/led-task.h"
#include "hardware/gpio.h" 
#include "stdio.h"
#include "stdint.h"
#include "bme280-driver.h"
#include "hardware/i2c.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

void version_callback(const char* args);
void led_on_callback(const char* args);
void led_off_callback(const char* args);
void led_blink_set_period_ms_callback(const char* args);
void led_blink_callback(const char* args);
void help_callback(const char* args);
void mem_callback(const char* args);
void wmem_callback(const char* args);
void read_regs_callback(const char* args);
void write_reg_callback(const char* args);
void temp_raw_callback(const char* args);
void pres_raw_callback(const char* args);
void hum_raw_callback(const char* args);
void temp_callback(const char* args);
void pres_callback(const char* args);
void hum_callback(const char* args);
void tm_callback(const char* args);

uint32_t mem(uint32_t addr)
{
    return *(volatile uint32_t*)addr;
}

void wmem(uint32_t addr, uint32_t data)
{
    *(volatile uint32_t*)addr = data;
}

api_t device_api[] =
{
	{"version", version_callback, "get device name and firmware version"},
    {"on", led_on_callback, "turn on the led"},
    {"off", led_off_callback, "turn off the led"},
    {"blink", led_blink_callback, "blink the led"},
    {"set_period", led_blink_set_period_ms_callback, "set period of blinkings"},
    {"help", help_callback, "print command description"},
    {"mem", mem_callback, "get data att address"},
    {"wmem", wmem_callback, "write data to address"},
    {"read_regs", read_regs_callback, "read registers from the sensor"},
    {"write_reg", write_reg_callback, "write to the sensor's register"},
    {"temp_raw", temp_raw_callback, "get raw temperature data"},
    {"pres_raw", pres_raw_callback, "get raw pressure data"},
    {"hum_raw", hum_raw_callback, "get raw humidity data"},
    {"temp", temp_callback, "get real temperature"},
    {"pres", pres_callback, "get real pressure"},
    {"hum", hum_callback, "get real humidity"},
    {"get_tm", tm_callback, "get temperature, pressure and humidity"},
	{NULL, NULL, NULL},
};

void version_callback(const char* args)
{
	printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void led_on_callback(const char* args){
    led_task_state_set(LED_STATE_ON);
}

void led_off_callback(const char* args){
    led_task_state_set(LED_STATE_OFF);
}

void led_blink_callback(const char* args){
    led_task_state_set(LED_STATE_BLINK);
}

void led_blink_set_period_ms_callback(const char* args){
    uint32_t period_ms = 0;
    sscanf(args, "%u", &period_ms);
    if(period_ms == 0){
        printf("entered unacceptable period \n");
        return;
    }
    else{
        led_task_set_blink_period_ms(period_ms);
    }
}

void read_regs_callback(const char* args){
    unsigned int start_reg_address;
    unsigned int N;
    if(sscanf(args, "%x %x", &start_reg_address, &N) != 2){
        return;
    }
    
    uint8_t buffer[256] = {0};
    if((start_reg_address > 0xFF) || (N > 0xFF) || (start_reg_address + N > 0x100)){
        return;
    }
    bme280_read_regs(start_reg_address, buffer, N);
    for (int i = 0; i < N; i++){
        printf("bme280 register [0x%X] = 0x%X\n", start_reg_address + i, buffer[i]);
    }
}

void write_reg_callback(const char* args){
    unsigned int addr;
    unsigned int value;
    if(sscanf(args, "%x %x", &addr, &value) != 2){
        return;
    }
    if((addr > 0xFF) || (value > 0xFF)){
        return;
    }
    bme280_write_reg(addr, value);
}

void help_callback(const char* args)
{
    for (int i = 0; device_api[i].command_name != NULL; i++)
    {
        printf("%s - %s\n", device_api[i].command_name, device_api[i].command_help);
    }
}

void mem_callback(const char* args)
{
    uint32_t addr;
    sscanf(args, "%u", &addr);
    if (addr == 0 || addr % 4 != 0)
    {
        printf("invalid address\n");
        return;
    }
    printf("%u\n", mem(addr));
}

void wmem_callback(const char* args)
{
    uint32_t addr, value;
    sscanf(args, "%u %u", &addr, &value);
    if (addr == 0 || addr % 4 != 0)
    {
        printf("invalid address\n");
        return;
    }
    wmem(addr, value);
}

void temp_raw_callback(const char* args){
    uint32_t raw_data = 0;
    raw_data = bme280_read_temp_raw();
    printf("%d\n", raw_data);
}

void pres_raw_callback(const char* args){
    uint32_t raw_data = 0;
    raw_data = bme280_read_pres_raw();
    printf("%d\n", raw_data);
}

void hum_raw_callback(const char* args){
    uint16_t raw_data = 0;
    raw_data = bme280_read_hum_raw();
    printf("%d\n", raw_data);
}

void temp_callback(const char* args){
    float real_data = 0.0;
    real_data = bmp280_get_temperature_celsius();
    printf("%f C\n", real_data);
}

void pres_callback(const char* args){
    float real_data = 0.0;
    real_data = bmp280_get_pressure_pa();
    printf("%f Pa\n", real_data);
}

float get_hum(){
    uint16_t raw_data = 0;
    raw_data = bme280_read_hum_raw();
    float real_data;
    real_data = raw_data/1024.0;
    return real_data;
}

void hum_callback(const char* args){
    float real_data;
    real_data = get_hum();
    printf("%f %%\n", real_data);
}

void tm_callback(const char* args){
    float temp = 0.0;
    float pres = 0.0;
    float hum = 0.0;
    temp = bmp280_get_temperature_celsius();
    pres = bmp280_get_temperature_celsius();
    hum = get_hum();
    printf("%f %f %f\n", pres, temp, hum);
}

void rp2040_i2c_read(uint8_t* buffer, uint16_t length){
	i2c_read_timeout_us(i2c1, 0x76, buffer, length, false, 100000);
}

void rp2040_i2c_write(uint8_t* data, uint16_t size){
	i2c_write_timeout_us(i2c1, 0x76, data, size, false, 100000);
}


int main(){
    i2c_init(i2c1, 100000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);

    bme280_init(rp2040_i2c_read, rp2040_i2c_write);
    stdio_init_all();
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();
    while(1){
        char* command = stdio_task_handle();
        if (command != NULL)
        {
            protocol_task_handle(command);
        }
        led_task_handle();
    }
}