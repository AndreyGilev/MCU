#pragma once

#include <stdint.h> //библиотеки целочисленных типов:


// typedef - создаём новый тип данных;
typedef void (*bme280_i2c_read)(uint8_t* buffer, uint16_t length); // вводит новый тип данных bme280_i2c_read являющийся указателем на функцию принимающая 2 аргумента и ничего не выводит
typedef void (*bme280_i2c_write)(uint8_t* data, uint16_t size); // вводит новый тип данных bme280_i2c_write являющийся указателем на функцию принимающая 2 аргумента и ничего не выводит
// Нужны для того, чтобы драйвер не зависил от устролйства
typedef enum {
    DEVICE_UNKNOWN = 0,
    DEVICE_BMP280,
    DEVICE_BME280
} device_type_t;

void bme280_init(bme280_i2c_read i2c_read, bme280_i2c_write i2c_write); // Принимает функции чтения и записи в I2C зависящие от конкректного устройства

void bme280_read_regs(uint8_t start_reg_address, uint8_t* buffer, uint8_t length);
/*
Функция будет читать [ length ] регистров BME280, начиная с адреса [ start_reg_address ]. 
Считанные значения функция записывавает в указатель на буфер [ buffer ]. 
Размер buffer обязан быть не менее length байт.
*/
void bme280_write_reg(uint8_t reg_address, uint8_t value);

uint16_t bme280_read_temp_raw();
uint16_t bme280_read_pres_raw();
uint16_t bme280_read_hum_raw();
float bmp280_get_temperature_celsius(void);
float bmp280_get_pressure_hpa(void);