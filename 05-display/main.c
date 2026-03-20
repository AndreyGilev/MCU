#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"
#include "pico/stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "ili9341-driver.h"
#include "ili9341-display.h"
#include "ili9341-font.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

#define ILI9341_PIN_MISO 4
#define ILI9341_PIN_CS 10
#define ILI9341_PIN_SCK 6
#define ILI9341_PIN_MOSI 7
#define ILI9341_PIN_DC 8
#define ILI9341_PIN_RESET 9
// #define PIN_LED -> 3.3V

void version_callback(const char* args);
void led_on_callback(const char* args);
void led_off_callback(const char* args);
void led_blink_set_period_ms_callback(const char* args);
void led_blink_callback(const char* args);
void help_callback(const char* args);
void mem_callback(const char* args);
void wmem_callback(const char* args);
void disp_screen_callback(const char* args);
void disp_px_callback(const char* args);
void disp_line_callback(const char* args);
void disp_rect_callback(const char* args);
void disp_frect_callback(const char* args);
void disp_text_callback(const char* args);

static ili9341_display_t ili9341_display = {0};

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
    {"set_period", led_blink_set_period_ms_callback, "set period of blinkings, args: T in ms"},
    {"help", help_callback, "print command description"},
    {"mem", mem_callback, "get data att address"},
    {"wmem", wmem_callback, "write data to address"},
    {"disp_screen", disp_screen_callback, "fill the screen with the color, args: color in 0x"},
    {"disp_px", disp_px_callback, "fill the pixel with the color, args: color in 0x, x, y"},
	{"disp_line", disp_line_callback, "draw the line with the color, args: color in 0x, xl, yl,  xr, yr"},
    {"disp_rect", disp_rect_callback, "draw the rectangle with the color, args: color in 0x, x, y, width, height"},
    {"disp_frect", disp_frect_callback, "draw and fill the rectangle with the color, args: color in 0x, x, y, width, height"},
    {"disp_text", disp_text_callback, "draw the text, args: font color in 0x, back color in 0x, x, y, text"},
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

void rp2040_spi_write(const uint8_t *data, uint32_t size)
{
	spi_write_blocking(spi0, data, size);
}

void rp2040_spi_read(uint8_t *buffer, uint32_t length)
{
	spi_read_blocking(spi0, 0, buffer, length);
}

void rp2040_gpio_cs_write(bool level)
{
	gpio_put(ILI9341_PIN_CS, level);
}

void rp2040_gpio_dc_write(bool level)
{
	gpio_put(ILI9341_PIN_DC, level);
}

void rp2040_gpio_reset_write(bool level)
{
	gpio_put(ILI9341_PIN_RESET, level);
}

void rp2040_delay_ms(int sec)
{
	sleep_ms(sec);
}

void disp_screen_callback(const char* args){
    uint32_t c = 0;
	int result = sscanf(args, "%x", &c);
	
	uint16_t color = COLOR_BLACK;
	
	if (result == 1)
	{
		color = RGB888_2_RGB565(c);
	}
	
	ili9341_fill_screen(&ili9341_display, color);
}

void disp_px_callback(const char* args){
    uint32_t c = 0;
    uint32_t x = 0;
    uint32_t y = 0;
	int result = sscanf(args, "%x %u %u", &c, &x, &y);
	
	uint16_t color = COLOR_BLACK;
	
	if (result == 3)
	{
		color = RGB888_2_RGB565(c);
	}
    ili9341_draw_pixel(&ili9341_display, x, y, color);
}

void disp_line_callback(const char* args){
    uint32_t c = 0;
    uint32_t xl = 0;
    uint32_t yl = 0;
    uint32_t xr = 0;
    uint32_t yr = 0;
	int result = sscanf(args, "%x %u %u %u %u", &c, &xl, &yl, &xr, &yr);
	
	uint16_t color = COLOR_BLACK;
	
	if (result == 5)
	{
		color = RGB888_2_RGB565(c);
	}
    ili9341_draw_line(&ili9341_display, xl, yl, xr, yr, color);
}

void disp_rect_callback(const char* args){
    uint32_t c = 0;
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t h = 0;
    uint32_t w = 0;
	int result = sscanf(args, "%x %u %u %u %u", &c, &x, &y, &h, &w);
	
	uint16_t color = COLOR_BLACK;
	
	if (result == 5)
	{
		color = RGB888_2_RGB565(c);
	}
    ili9341_draw_rect(&ili9341_display, x, y, h, w, color);
}

void disp_frect_callback(const char* args){
    uint32_t c = 0;
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t h = 0;
    uint32_t w = 0;
	int result = sscanf(args, "%x %u %u %u %u", &c, &x, &y, &h, &w);
	
	uint16_t color = COLOR_BLACK;
	
	if (result == 5)
	{
		color = RGB888_2_RGB565(c);
	}
    ili9341_draw_filled_rect(&ili9341_display, x, y, h, w, color);
}

void disp_text_callback(const char* args){
    uint32_t ct = 0;
    uint32_t cb = 0;
    uint32_t x = 0;
    uint32_t y = 0;
    char str[100];
	int result = sscanf(args, "%x %x %u %u %99s", &ct, &cb, &x, &y, &str);
	
	uint16_t colorText = RGB888_2_RGB565(ct);
    uint16_t colorBack = RGB888_2_RGB565(cb);
	
	ili9341_draw_text(&ili9341_display, x, y, str, &jetbrains_font, colorText, colorBack);
}



int main(){
    stdio_init_all();
    stdio_task_init();
    spi_init(spi0, 62500000);

    gpio_set_function(ILI9341_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_SCK, GPIO_FUNC_SPI);

    gpio_init(ILI9341_PIN_CS);
    gpio_set_dir(ILI9341_PIN_CS, GPIO_OUT);

    gpio_init(ILI9341_PIN_DC);
    gpio_set_dir(ILI9341_PIN_DC, GPIO_OUT);

    gpio_init(ILI9341_PIN_RESET);
    gpio_set_dir(ILI9341_PIN_RESET, GPIO_OUT);

    gpio_put(ILI9341_PIN_CS, 1);
    gpio_put(ILI9341_PIN_DC, 0);
    gpio_put(ILI9341_PIN_RESET, 0);

    ili9341_hal_t ili9341_hal = {0};
    ili9341_hal.spi_write = rp2040_spi_write;
    ili9341_hal.spi_read = rp2040_spi_read;
    ili9341_hal.gpio_cs_write = rp2040_gpio_cs_write;
    ili9341_hal.gpio_dc_write = rp2040_gpio_dc_write;

    ili9341_hal.gpio_reset_write = rp2040_gpio_reset_write;
    ili9341_hal.delay_ms = rp2040_delay_ms;

    ili9341_init(&ili9341_display, &ili9341_hal);
    ili9341_set_rotation(&ili9341_display, ILI9341_ROTATION_90);
    ili9341_fill_screen(&ili9341_display, COLOR_BLACK);
    sleep_ms(300);

    /*ili9341_draw_filled_rect(&ili9341_display, 10, 10, 100, 60, COLOR_RED);
    ili9341_draw_filled_rect(&ili9341_display, 120, 10, 100, 60, COLOR_GREEN);
    ili9341_draw_filled_rect(&ili9341_display, 230, 10, 80, 60, COLOR_BLUE);
    ili9341_draw_rect(&ili9341_display, 10, 90, 300, 80, COLOR_WHITE);

    ili9341_draw_line(&ili9341_display, 0, 0, 319, 239, COLOR_YELLOW);
    ili9341_draw_line(&ili9341_display, 319, 0, 0, 239, COLOR_CYAN);

    ili9341_draw_text(&ili9341_display, 20, 100, "Hello, ILI9341!", &jetbrains_font, COLOR_WHITE, COLOR_BLACK);

    ili9341_draw_text(&ili9341_display, 20, 116, "RP2040 / Pico SDK", &jetbrains_font, COLOR_GREEN, COLOR_BLACK);*/


    protocol_task_init(device_api);
    led_task_init();
    while(1){
        char* command = stdio_task_handle();ili9341_draw_text(&ili9341_display, 20, 116, "RP2040 / Pico SDK", &jetbrains_font, COLOR_YELLOW, COLOR_BLACK);
        if (command != NULL)
        {
            protocol_task_handle(command);
        }
        led_task_handle();
    }
}