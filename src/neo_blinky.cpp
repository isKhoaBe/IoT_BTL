#include "neo_blinky.h"

/**
 * Task 2: NeoPixel LED Control Based on Humidity
 *
 * This task implements three different color patterns based on humidity:
 * 1. DRY (< 40%): RED - Indicates low humidity
 * 2. COMFORTABLE (40-60%): GREEN - Ideal humidity range
 * 3. HUMID (> 60%): BLUE - Indicates high humidity
 *
 * Uses semaphore-protected shared data access to read humidity values
 */

void neo_blinky(void *pvParameters)
{
    // Wait for shared data structures to be ready
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Initialize NeoPixel LED
    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.clear();
    strip.show();

    // Local variables to store sensor readings
    float temperature = 0.0;
    float humidity = 0.0;

    // Color variables
    uint8_t red = 0, green = 0, blue = 0;

    Serial.println("[NEO_LED] Task started - Humidity-based color control");

    while (1)
    {
        bool isOverride = false;
        if (xSemaphoreTake(g_wifiConfig->mutex, pdMS_TO_TICKS(50)) == pdTRUE && g_wifiConfig != NULL)
        {
            isOverride = g_wifiConfig->neoOverride;
            xSemaphoreGive(g_wifiConfig->mutex);
        }

        if (isOverride)
        {
            Serial.println("[NEO_LED] Override active - Web control taking over");
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }
        // Read sensor data using semaphore-protected function
        getSensorData(&temperature, &humidity);

        // Determine LED color based on humidity
        if (humidity < HUMIDITY_LOW_THRESHOLD)
        {
            // DRY state: RED
            red = COLOR_DRY_R;
            green = COLOR_DRY_G;
            blue = COLOR_DRY_B;
            Serial.print("[NEO_LED] DRY (");
            Serial.print(humidity);
            Serial.println("%) - RED");
        }
        else if (humidity >= HUMIDITY_LOW_THRESHOLD && humidity <= HUMIDITY_COMFORTABLE_THRESHOLD)
        {
            // COMFORTABLE state: GREEN
            red = COLOR_COMFORTABLE_R;
            green = COLOR_COMFORTABLE_G;
            blue = COLOR_COMFORTABLE_B;
            Serial.print("[NEO_LED] COMFORTABLE (");
            Serial.print(humidity);
            Serial.println("%) - GREEN");
        }
        else
        {
            // HUMID state: BLUE
            red = COLOR_HUMID_R;
            green = COLOR_HUMID_G;
            blue = COLOR_HUMID_B;
            Serial.print("[NEO_LED] HUMID (");
            Serial.print(humidity);
            Serial.println("%) - BLUE");
        }

        // Update NeoPixel color
        strip.setPixelColor(0, strip.Color(red, green, blue));
        strip.show();

        // Update every 2 seconds
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}