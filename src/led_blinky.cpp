#include "led_blinky.h"

/**
 * Task 1: Single LED Blink with Temperature Conditions
 * 
 * This task implements three different LED blinking behaviors based on temperature:
 * 1. HOT (> 29°C): Fast blink - 200ms ON, 200ms OFF
 * 2. NORMAL (22-29°C): Normal blink - 1s ON, 1s OFF
 * 3. COLD (< 22°C): Slow ON, fast OFF - 2s ON, 200ms OFF
 * 
 * Uses semaphore-protected shared data access to read temperature values
 */

void led_blinky(void *pvParameters){
    // Wait for shared data structures to be ready
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    pinMode(LED_GPIO, OUTPUT);
    
    // Local variables to store sensor readings
    float temperature = 0.0;
    float humidity = 0.0;
    
    // LED timing variables
    uint16_t onTime = LED_NORMAL_ON_TIME;
    uint16_t offTime = LED_NORMAL_OFF_TIME;
    
    Serial.println("[LED_BLINK] Task started - Temperature-based LED control");
    
    while(1) {
        // Read sensor data using semaphore-protected function
        getSensorData(&temperature, &humidity);
        
        // Determine LED blink pattern based on temperature
        if (temperature < TEMP_COLD_THRESHOLD) {
            // COLD state: Slow ON, fast OFF
            onTime = LED_COLD_ON_TIME;
            offTime = LED_COLD_OFF_TIME;
            Serial.println("[LED_BLINK] COLD - Slow ON, Fast OFF");
        } 
        else if (temperature >= TEMP_COLD_THRESHOLD && temperature <= TEMP_NORMAL_THRESHOLD) {
            // NORMAL state: 1s ON, 1s OFF
            onTime = LED_NORMAL_ON_TIME;
            offTime = LED_NORMAL_OFF_TIME;
            Serial.println("[LED_BLINK] NORMAL - 1s ON/OFF");
        } 
        else {
            // HOT state: Fast blink
            onTime = LED_HOT_ON_TIME;
            offTime = LED_HOT_OFF_TIME;
            Serial.println("[LED_BLINK] HOT - Fast blink");
        }
        
        // Turn LED ON
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(onTime));
        
        // Turn LED OFF
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(pdMS_TO_TICKS(offTime));
    }
}