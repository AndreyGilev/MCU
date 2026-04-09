#include "string.h"
#include "../include/bme280-driver.h"
#include "../include/bme280-regs.h"
#include "stdio.h"  // Для работы с вводом/выводом данных standart input output


typedef struct
{
	bme280_i2c_read i2c_read;
	bme280_i2c_write i2c_write;
} bme280_ctx_t;

static bme280_ctx_t bme280_ctx = {0};
/*
Переменные, хранящие состояние библиотеки 
или других программных модулей принято называть контексом.
Т.е. bme280_ctx - это контекст драйвера BME280
*/

static device_type_t device_type = DEVICE_UNKNOWN;
// Глобальные переменные для калибровочных коэффициентов
static uint16_t dig_T1;
static int16_t dig_T2, dig_T3;
static uint16_t dig_P1;
static int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
static int32_t t_fine;


void bme280_init(bme280_i2c_read i2c_read, bme280_i2c_write i2c_write)
{

    bme280_ctx.i2c_read = i2c_read;
    bme280_ctx.i2c_write = i2c_write;
//--------------------------------------------------------------------------	
	uint8_t id_reg_buf[1] = {0}; // В памяти выделен 1 байт [0x00]
	bme280_read_regs(BME280_REG_id, id_reg_buf, sizeof(id_reg_buf));

	if (id_reg_buf[0] == 0x60)
	{
		device_type = DEVICE_BME280;
		printf("BME280 finded! ID = 0x%X\n", id_reg_buf[0]);


		uint8_t ctrl_hum_reg_value = 0;
		ctrl_hum_reg_value |= (0b001 << 0); // osrs_h[2:0] = oversampling 1
		bme280_write_reg(BME280_REG_ctrl_hum, ctrl_hum_reg_value);
	}
	else if (id_reg_buf[0] == 0x58)
	{
		device_type = DEVICE_BMP280;
		printf("BMP280 finded! ID = 0x%X\n", id_reg_buf[0]);
	}
	else
	{
		printf("Error: taken 0x%X\n", id_reg_buf[0]);
	}
//--------------------------------------------------------------------------	
	uint8_t config_reg_value = 0; //  1 байт 00000000 
	// <<X - перемещает на X байтов влево наяччинае с конца; | - объединить
	// 00AB << 2 -> AB00
	// (00AB << 2) | 00CD -> ABCD
	config_reg_value |= (0b0 << 0); // spi3w_en[0:0] = false
	config_reg_value |= (0b000 << 2); // filter[4:2] = Filter off
	config_reg_value |= (0b001 << 5); // t_sb[7:5] = 62.5 ms 
	/*
	t_sb[1:0] | tstandby [ms] 
	000 		0.5 
	001 		62.5 
	010 		125 
	011 		250 
	100 		500 
	101 		1000 
	110 		2000 
	111 		4000 
	*/
	bme280_write_reg(BME280_REG_config, config_reg_value);

//--------------------------------------------------------------------------	
	uint8_t ctrl_meas_reg_value = 0;
	ctrl_meas_reg_value |= (0b11 << 0);
	ctrl_meas_reg_value |= (0b001 << 2);
	ctrl_meas_reg_value |= (0b001 << 5);
/*
	Биты |	   Поле   |	Назначение
	7:5	 |osrs_t[2:0] |	Оверсемплинг температуры
	4:2	 |osrs_p[2:0] |	Оверсемплинг давления
	1:0	 |mode[1:0]	  | Режим работы (00 = sleep, 01/10 = forced, 11 = normal, подробнее на стр 15 data sheet)
*/
	bme280_write_reg(BME280_REG_ctrl_meas, ctrl_meas_reg_value);
//----------------------------------------------------------------
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

//  Функция чтения регистров принимает "откуда куда и сколько"
void bme280_read_regs(uint8_t start_reg_address, uint8_t* buffer, uint8_t length)
{
	uint8_t start[1] = {start_reg_address}; // содержит адрес регистра, с которого должно начаться чтение
	
	bme280_ctx.i2c_write(start, sizeof(start)); // отправляем  адрес регистра, с которого мы хотим начать чтение:
	bme280_ctx.i2c_read(buffer, length); //прочитать  желаемое количество регистров сразу в буффер пользователя:
}
// принимает "откуда и куда"
void bme280_write_reg(uint8_t reg_address, uint8_t value)
{
	// отправляет байт адреса регистра и после него байт значения регистра
    uint8_t data[2] = {reg_address, value};
    bme280_ctx.i2c_write(data, sizeof(data));
}

//Функции возвращающие значения измерения в отсчетах
uint16_t bme280_read_temp_raw() 
{
	uint8_t read[2] = {0};
	bme280_read_regs(BME280_REG_temp_msb, read, sizeof(read));
	uint16_t value = ((uint16_t)read[0] << 8) | ((uint16_t)read[1]);   
	return value;
// Результаты измерений BME280 имеют размер 16 бит. Регистры BME280 всего 8 бит.
// Поэтому нужно их объеденить (3 строчка)
}
uint16_t bme280_read_pres_raw()
{
	uint8_t read[2] = {0};
	bme280_read_regs(BME280_REG_press_msb, read, sizeof(read));
	uint16_t value = ((uint16_t)read[0] << 8) | ((uint16_t)read[1]);   
	return value;
}
uint16_t bme280_read_hum_raw()
{
	if (device_type == DEVICE_BMP280)
	{
		printf("This function not working on your divice\n");
		return 0;
	}
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
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    
    return (uint32_t)p;
}

// Получить температуру в градусах Цельсия
float bmp280_get_temperature_celsius(void)
{
    uint16_t raw = bme280_read_temp_raw();
    // BME280/BMP280 используют 20-битные значения, а у вас 16 бит
    // Нужно преобразовать в 20-битный формат
    int32_t adc_T = (int32_t)raw << 4;  // Сдвиг для 20 бит
    
    int32_t temp_raw = bmp280_compensate_temperature(adc_T);
    return temp_raw / 100.0f;
}

// Получить давление в гектопаскалях (гПа)
float bmp280_get_pressure_hpa(void)
{
    uint16_t raw_temp = bme280_read_temp_raw();
    uint16_t raw_pres = bme280_read_pres_raw();
    
    // Преобразование в 20-битный формат
    int32_t adc_T = (int32_t)raw_temp << 4;
    int32_t adc_P = (int32_t)raw_pres << 4;
    
    // Обновляем t_fine
    bmp280_compensate_temperature(adc_T);
    
    uint32_t pressure_raw = bmp280_compensate_pressure(adc_P);
    return pressure_raw / 256.0f / 100.0f;  // Па / 100.0f -> гПа
}