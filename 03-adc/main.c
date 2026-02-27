#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"
#include "adc-task/adc-task.h"
#include "stdio.h"
#include "stdint.h"


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
void get_adc_callback(const char* args);
void get_temp_callback(const char* args);
void tm_start_callback(const char* args);
void tm_stop_callback(const char* args);

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
    {"get_adc", get_adc_callback, "get analog value of voltage"},
    {"get_temp", get_temp_callback, "get value of temperature in celcius degrees"},
    {"tm_start", tm_start_callback, "start measures"},
    {"tm_stop", tm_stop_callback, "stop measures"},
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

void get_adc_callback(const char* args){
    float voltage_V = get_voltage();
    printf("%f\n", voltage_V);
}

void get_temp_callback(const char* args){
    float temp_C= get_temp();
    printf("%f\n", temp_C);
}

void tm_start_callback(const char* args){
    adc_task_set_state(1);
}

void tm_stop_callback(const char* args){
    adc_task_set_state(0);
}

int main(){
    stdio_init_all();
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();
    adc_task_init();
    while(1){
        char* command = stdio_task_handle();
        if (command != NULL)
        {
            protocol_task_handle(command);
        }
        led_task_handle();
        adc_task_handle();
    }
}