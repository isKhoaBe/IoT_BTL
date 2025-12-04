#include "task_toogle_boot.h"
#include "task_wifi.h"
#include "global.h"
#include <WiFi.h>

#define BOOT_PIN 0

void Task_Toogle_BOOT(void *pvParameters)
{
    pinMode(BOOT_PIN, INPUT_PULLUP);
    unsigned long buttonPressStartTime = 0;
    bool actionTaken = false;

    while (true)
    {
        if (digitalRead(BOOT_PIN) == LOW)
        {
            if (buttonPressStartTime == 0)
            {
                buttonPressStartTime = millis();
                actionTaken = false;
            }
            else if (millis() - buttonPressStartTime > 2000 && !actionTaken)
            {
                Serial.println("\n\n[BOOT BUTTON] Detected Long Press!");
                Serial.println("🔄 Switching to AP Mode...");

                Delete_info_File();

                if (g_wifiConfig != NULL && xSemaphoreTake(g_wifiConfig->mutex, pdMS_TO_TICKS(100)) == pdTRUE)
                {
                    g_wifiConfig->WIFI_SSID = "";
                    g_wifiConfig->WIFI_PASS = "";
                    g_wifiConfig->isWifiConnected = false;
                    xSemaphoreGive(g_wifiConfig->mutex);
                }
                WiFi.disconnect(true);
                WiFi.mode(WIFI_AP);
                WiFi.softAP(String(SSID_AP), String(PASS_AP));

                Serial.print("✅ AP Mode Started. IP: ");
                Serial.println(WiFi.softAPIP());
                actionTaken = true;
            }
        }
        else
        {
            buttonPressStartTime = 0;
            actionTaken = false;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}