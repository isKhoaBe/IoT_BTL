#ifndef __TEMP_HUMI_MONITOR__
#define __TEMP_HUMI_MONITOR__
#include <Arduino.h>
#include "LiquidCrystal_I2C.h"
#include "DHT20.h"
#include "global.h"

// LCD I2C address and dimensions
#define LCD_ADDRESS 33
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// Display state thresholds
#define TEMP_CRITICAL_LOW 18.0
#define TEMP_CRITICAL_HIGH 32.0
#define HUMIDITY_CRITICAL_LOW 25.0
#define HUMIDITY_CRITICAL_HIGH 80.0

#define TEMP_WARNING_LOW 20.0
#define TEMP_WARNING_HIGH 29.0
#define HUMIDITY_WARNING_LOW 30.0
#define HUMIDITY_WARNING_HIGH 70.0

// Display states
typedef enum {
    DISPLAY_NORMAL,
    DISPLAY_WARNING,
    DISPLAY_CRITICAL
} DisplayState_t;

// Sensor reading task
void temp_humi_monitor(void *pvParameters);

// LCD display task
void lcd_display_task(void *pvParameters);

#endif