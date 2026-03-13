#include "../include/bme280-driver.h"
#include "../include/bme280-regs.h"
#include "stdio.h"

typedef struct
{
	bme280_i2c_read i2c_read;
	bme280_i2c_write i2c_write;
} bme280_ctx_t;

static bme280_ctx_t bme280_ctx = {0};

static uint16_t dig_T1;
static int16_t dig_T2, dig_T3;
static uint16_t dig_P1;
static int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
static int32_t t_fine;




void bme280_init(bme280_i2c_read i2c_read, bme280_i2c_write i2c_write){
    bme280_ctx.i2c_read = i2c_read;
    bme280_ctx.i2c_write = i2c_write;
    uint8_t id_reg_buf[1] = {0};
    bme280_read_regs(BME280_REG_id, id_reg_buf, sizeof(id_reg_buf));
    if(id_reg_buf[0] != 0x60){
        printf("Sensor's id isn't 0x60. Probably the sensor isn't connected properly");
    }

    uint8_t ctrl_hum_reg_value = 0;
    ctrl_hum_reg_value |= (0b001 << 0); // osrs_h[2:0] = oversampling 1
    bme280_write_reg(BME280_REG_ctrl_hum, ctrl_hum_reg_value);

    uint8_t config_reg_value = 0;
    config_reg_value |= (0b0 << 0); // spi3w_en[0:0] = false
    config_reg_value |= (0b000 << 2); // filter[4:2] = Filter off
    config_reg_value |= (0b001 << 5); // t_sb[7:5] = 62.5 ms
    bme280_write_reg(BME280_REG_config, config_reg_value);

    uint8_t ctrl_meas_check;
bme280_read_regs(BME280_REG_ctrl_meas, &ctrl_meas_check, 1);
printf("ctrl_meas after init: 0x%02X\n", ctrl_meas_check);

    uint8_t ctrl_meas_reg_value = 0;
    ctrl_meas_reg_value |= (0b11 << 0); // mode[1:0] = Normal mode
    ctrl_meas_reg_value |= (0b001 << 2); // osrs_p[4:2] = oversampling 1
    ctrl_meas_reg_value |= (0b001 << 5); // osrs_t[7:5] = oversampling 1
    bme280_write_reg(BME280_REG_ctrl_meas, ctrl_meas_reg_value);

    uint8_t calib_data[24];
    bme280_read_regs(BME280_REG_calib00, calib_data, sizeof(calib_data));
    
    dig_T1 = (uint16_t)(calib_data[1] << 8 | calib_data[0]);
    dig_T2 = (int16_t)(calib_data[3] << 8 | calib_data[2]);
    dig_T3 = (int16_t)(calib_data[5] << 8 | calib_data[4]);
    dig_P1 = (uint16_t)(calib_data[7] << 8 | calib_data[6]);
    dig_P2 = (int16_t)(calib_data[9] << 8 | calib_data[8]);
    dig_P3 = (int16_t)(calib_data[11] << 8 | calib_data[10]);
    dig_P4 = (int16_t)(calib_data[13] << 8 | calib_data[12]);
    dig_P5 = (int16_t)(calib_data[15] << 8 | calib_data[14]);
    dig_P6 = (int16_t)(calib_data[17] << 8 | calib_data[16]);
    dig_P7 = (int16_t)(calib_data[19] << 8 | calib_data[18]);
    dig_P8 = (int16_t)(calib_data[21] << 8 | calib_data[20]);
    dig_P9 = (int16_t)(calib_data[23] << 8 | calib_data[22]);
}

void bme280_read_regs(uint8_t start_reg_address, uint8_t* buffer, uint8_t length){
    uint8_t data[1] = {start_reg_address};
    bme280_ctx.i2c_write(data, sizeof(data));
    bme280_ctx.i2c_read(buffer, length);
}

void bme280_write_reg(uint8_t reg_address, uint8_t value){
    uint8_t data[2] = {reg_address, value};
    bme280_ctx.i2c_write(data, sizeof(data));
}

uint32_t bme280_read_temp_raw(){
    uint8_t status;
bme280_read_regs(0xF3, &status, 1);
printf("status reg: 0x%02X\n", status);
    uint8_t read[3] = {0};
    bme280_read_regs(BME280_REG_temp_msb, read, sizeof(read));
    
    uint32_t value = ((uint32_t)read[0] << 12) | ((uint32_t)read[1] << 4) | ((uint32_t)read[2] >> 4);
    uint8_t read[3] = {0};
bme280_read_regs(0xFA, read, 3);  // читаем с 0xFA, 0xFB, 0xFC

printf("temp bytes: [0x%02X] [0x%02X] [0x%02X]\n", read[0], read[1], read[2]);
    return value;
}

uint32_t bme280_read_pres_raw(){
    uint8_t read[3] = {0};
	bme280_read_regs(BME280_REG_press_msb, read, sizeof(read));
	uint32_t value = ((uint32_t)read[0] << 12) | ((uint32_t)read[1]<<4) | ((uint32_t)read[2]>>4);
	return value;
}

uint16_t bme280_read_hum_raw(){
    uint8_t read[2] = {0};
	bme280_read_regs(BME280_REG_hum_msb, read, sizeof(read));
	uint16_t value = ((uint16_t)read[0] << 8) | ((uint16_t)read[1]);
	return value;
}


// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.  
// t_fine carries fine temperature as global value 
int32_t bmp280_compensate_temperature(int32_t adc_T)
{
    int32_t var1, var2, T;
    
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    
    return T;
}


// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits). 
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa 
uint32_t bmp280_compensate_pressure(int32_t adc_P)
{
    int64_t var1, var2, p;
    
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
    
    if (var1 == 0) {
        return 0;
    }
    
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    printf("DEBUG: var1=%lld, p_before=%lld, p_after=%lld\n", var1, (p << 31) - var2, p);
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    
    return (uint32_t)p;
}

// Получить температуру в градусах Цельсия
float bmp280_get_temperature_celsius(void)
{
    uint32_t raw = bme280_read_temp_raw();
    
    int32_t temp_raw = bmp280_compensate_temperature((int32_t)raw);
    return temp_raw / 100.0f;
}

float bmp280_get_pressure_pa(void)
{
    printf("sizeof(int64_t) = %zu\n", sizeof(int64_t));
    uint32_t raw_temp = bme280_read_temp_raw();
    uint32_t raw_pres = bme280_read_pres_raw();
    
    
    // Обновляем t_fine
    bmp280_compensate_temperature((int32_t)raw_temp);
    printf("t_fine before pressure: %ld\n", t_fine);
    
    uint32_t pressure_raw = bmp280_compensate_pressure((int32_t)raw_pres);
    return pressure_raw / 256.0f;
}