#ifndef __NEO_BLINKY__
#define __NEO_BLINKY__
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "global.h"

#define NEO_PIN 45
#define LED_COUNT 1 

// Humidity thresholds for different color patterns
#define HUMIDITY_LOW_THRESHOLD 30.0      // Below 40%: DRY (RED)
#define HUMIDITY_COMFORTABLE_THRESHOLD 70.0  // 40-60%: COMFORTABLE (GREEN)
// Above 60%: HUMID (BLUE)

// Color definitions (RGB values)
#define COLOR_DRY_R 59         // RED for dry
#define COLOR_DRY_G 59
#define COLOR_DRY_B 59

#define COLOR_COMFORTABLE_R 0    // GREEN for comfortable
#define COLOR_COMFORTABLE_G 255
#define COLOR_COMFORTABLE_B 0

#define COLOR_HUMID_R 0         // BLUE for humid
#define COLOR_HUMID_G 0
#define COLOR_HUMID_B 255

void neo_blinky(void *pvParameters);

#endif