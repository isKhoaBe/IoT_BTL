#ifndef __LED_BLINKY__
#define __LED_BLINKY__
#include <Arduino.h>
#include "global.h"

#define LED_GPIO 48

// Temperature thresholds
#define TEMP_COLD_THRESHOLD 22.0    // Below this: COLD
#define TEMP_NORMAL_THRESHOLD 29.0  // Between 22-29: NORMAL
// Above 29°C: HOT

// LED blink patterns (in milliseconds)
#define LED_HOT_ON_TIME 200          // Fast blink for hot
#define LED_HOT_OFF_TIME 200
#define LED_NORMAL_ON_TIME 1000      // 1 second ON for normal
#define LED_NORMAL_OFF_TIME 1000     // 1 second OFF for normal
#define LED_COLD_ON_TIME 2000        // Slow ON for cold
#define LED_COLD_OFF_TIME 200        // Fast OFF for cold

void led_blinky(void *pvParameters);

#endif