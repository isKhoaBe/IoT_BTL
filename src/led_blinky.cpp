#include "led_blinky.h"
#include "task_webserver.h"
#include "task_check_info.h"
#include "neo_blinky.h"
#include <ArduinoJson.h>
Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

bool isValidOutputPin(int gpio)
{
    const int forbidden[] = {0, 11, 12, 19, 20, 43, 44, 45, 48};
    for (int f : forbidden)
        if (gpio == f)
            return false;

    if (gpio < 0 || gpio > 49)
        return false;
    return true;
}

void led_blinky(void *pvParameters)
{
    vTaskDelay(pdMS_TO_TICKS(1000));

    pinMode(LED_GPIO, OUTPUT);

    float temperature = 0.0;
    float humidity = 0.0;

    // LED timing variables
    uint16_t onTime = LED_NORMAL_ON_TIME;
    uint16_t offTime = LED_NORMAL_OFF_TIME;

    Serial.println("[LED_BLINK] Task started - Temperature-based LED control");

    while (1)
    {
        bool override = false;
        if (xSemaphoreTake(g_wifiConfig->mutex, pdMS_TO_TICKS(50)) == pdTRUE)
        {
            override = g_wifiConfig->led1Override;
            xSemaphoreGive(g_wifiConfig->mutex);
        }

        if (override)
        {
            Serial.println("[LED_BLINK] Override active - Web control taking over");
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }
        getSensorData(&temperature, &humidity);

        if (temperature < TEMP_COLD_THRESHOLD)
        {
            onTime = LED_COLD_ON_TIME;
            offTime = LED_COLD_OFF_TIME;
            // Serial.println("[LED_BLINK] COLD - Slow ON, Fast OFF");
        }
        else if (temperature >= TEMP_COLD_THRESHOLD && temperature <= TEMP_NORMAL_THRESHOLD)
        {
            onTime = LED_NORMAL_ON_TIME;
            offTime = LED_NORMAL_OFF_TIME;
            // Serial.println("[LED_BLINK] NORMAL - 1s ON/OFF");
        }
        else
        {
            onTime = LED_HOT_ON_TIME;
            offTime = LED_HOT_OFF_TIME;
            // Serial.println("[LED_BLINK] HOT - Fast blink");
        }

        // Turn LED ON
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(onTime));

        // Turn LED OFF
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(pdMS_TO_TICKS(offTime));
    }
}

void Device_Control_Task(void *pvParameters)
{
    strip.begin();
    strip.show();
    pinMode(LED_GPIO, OUTPUT);

    DeviceControlCommand cmd;
    SettingsCommand settings;

    while (1)
    {
        // Check for settings updates first (higher priority)
        if (xQueueReceive(xQueueSettings, &settings, 0) == pdPASS)
        {
            Serial.println("💾 Processing Settings from Queue...");
            Serial.println("SSID: " + settings.ssid);
            Serial.println("Password: " + settings.password);
            Serial.println("Token: " + settings.token);
            Serial.println("Server: " + settings.server);
            Serial.print("Port: ");
            Serial.println(settings.port);
            
            // Save configuration to file and restart
            Save_info_File(settings.ssid, settings.password, settings.token, 
                          settings.server, String(settings.port));
        }
        
        if (xQueueReceive(xQueueRelayControl, &cmd, pdMS_TO_TICKS(100)) == pdPASS)
        {
            int pin = cmd.gpioPin;
            bool isWebOn = cmd.newState;

            if (pin == LED_GPIO)
            {
                if (g_wifiConfig != NULL && xSemaphoreTake(g_wifiConfig->mutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    // When toggle is OFF, enable override to force device OFF
                    // When toggle is ON, disable override to allow AUTO mode
                    g_wifiConfig->led1Override = !isWebOn;
                    xSemaphoreGive(g_wifiConfig->mutex);
                }
                if (isWebOn)
                {
                    // Toggle ON = AUTO mode, let sensor task handle it
                    Serial.println("✅ LED1 set to AUTO mode (sensor-controlled)");
                }
                else
                {
                    // Toggle OFF = Force OFF (override)
                    digitalWrite(LED_GPIO, LOW);
                    Serial.println("✅ LED1 forced OFF (override active)");
                }
            }

            else if (pin == NEO_PIN)
            {
                if (g_wifiConfig != NULL && xSemaphoreTake(g_wifiConfig->mutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    // When toggle is OFF, enable override to force device OFF
                    // When toggle is ON, disable override to allow AUTO mode
                    g_wifiConfig->neoOverride = !isWebOn;
                    xSemaphoreGive(g_wifiConfig->mutex);
                }

                if (isWebOn)
                {
                    // Toggle ON = AUTO mode, let sensor task handle it
                    Serial.println("✅ NeoPixel set to AUTO mode (humidity-controlled)");
                }
                else
                {
                    // Toggle OFF = Force OFF (override)
                    strip.setPixelColor(0, strip.Color(0, 0, 0));
                    strip.show();
                    Serial.println("✅ NeoPixel forced OFF (override active)");
                }
            }
            else
            {
                if (!isValidOutputPin(pin))
                {
                    Serial.printf("⚠️ Invalid GPIO %d received - ignoring\n", pin);
                }
                else
                {
                    bool alreadyConfigured = false;
                    for (int p : g_userPins)
                    {
                        if (p == pin)
                        {
                            alreadyConfigured = true;
                            break;
                        }
                    }
                    if (!alreadyConfigured)
                    {
                        pinMode(pin, OUTPUT);
                        digitalWrite(pin, LOW); // default safe state
                        g_userPins.push_back(pin);
                        Serial.printf("ℹ️ Configured GPIO %d as OUTPUT and saved to userPins\n", pin);
                    }
                    if (g_wifiConfig != NULL && xSemaphoreTake(g_wifiConfig->mutex, pdMS_TO_TICKS(100)) == pdTRUE)
                    {
                        g_wifiConfig->relayOverride = isWebOn;
                        xSemaphoreGive(g_wifiConfig->mutex);
                    }
                    digitalWrite(pin, isWebOn ? HIGH : LOW);
                    Serial.printf("✅ External Relay (GPIO %d) set to %s\n", pin, isWebOn ? "ON" : "OFF");
                }
            }

            StaticJsonDocument<256> jsonDoc;
            jsonDoc["page"] = "device_update";
            jsonDoc["value"]["gpio"] = pin;
            jsonDoc["value"]["status"] = isWebOn ? "ON" : "OFF";

            String jsonString;
            serializeJson(jsonDoc, jsonString);
            Webserver_sendata(jsonString);
        }
    }
}