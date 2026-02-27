#include "hardware/adc.h"
#include "adc-task.h"
#include "stdint.h"
#include "stdio.h"
#include "pico/stdlib.h"

adc_task_state_t adc_state;
uint64_t time;
uint64_t ADC_TASK_MEAS_PERIOD_US = 100000;
static uint LED_PIN = 25;
const uint ADC_CH = 0; // номер канала АЦП
const uint TEMP_CH = 4; // номер канала, подключенного к датчику температуры


void adc_task_init(){
    adc_init();
    adc_gpio_init(LED_PIN);
    adc_set_temp_sensor_enabled(true);
    adc_state = ADC_TASK_STATE_IDLE;
    time = 0;
}

float get_voltage(){
    adc_select_input(ADC_CH);
    uint16_t voltage_counts = adc_read();
    float voltage_V = voltage_counts / 4096 * 3.3;
    return voltage_V;
}

float get_temp(){
    adc_select_input(TEMP_CH);
    uint16_t voltage_counts = adc_read();
    float voltage_V = voltage_counts / 4096 * 3.3;
    float temp_C = 27.0f - (voltage_V - 0.706f) / 0.001721f;
    return temp_C;
}

void adc_task_set_state(adc_task_state_t state){
    adc_state = state;
}

void adc_task_handle(){
    if(adc_state == ADC_TASK_STATE_RUN){
        if (time_us_64() > time)
        {
            time = time_us_64() + (ADC_TASK_MEAS_PERIOD_US / 2);
            float voltage_V = get_voltage();
            float temp_C = get_temp();
            printf("%f %f\n", voltage_V, temp_C);
        }
    }
}