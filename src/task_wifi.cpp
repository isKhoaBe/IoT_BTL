#include "task_wifi.h"

void startAP()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(String(SSID_AP), String(PASS_AP));
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

void startSTA()
{
    String ssid = "";
    String pass = "";
    
    if (g_wifiConfig != NULL && g_wifiConfig->mutex != NULL) {
        if (xSemaphoreTake(g_wifiConfig->mutex, portMAX_DELAY) == pdTRUE) {
            ssid = g_wifiConfig->WIFI_SSID;
            pass = g_wifiConfig->WIFI_PASS;
            xSemaphoreGive(g_wifiConfig->mutex);
        }
    }
    
    if (ssid.isEmpty())
    {
        vTaskDelete(NULL);
    }

    WiFi.mode(WIFI_STA);

    if (pass.isEmpty())
    {
        WiFi.begin(ssid.c_str());
    }
    else
    {
        WiFi.begin(ssid.c_str(), pass.c_str());
    }

    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    //Give a semaphore here
    if (g_wifiConfig != NULL && g_wifiConfig->xBinarySemaphoreInternet != NULL) {
        xSemaphoreGive(g_wifiConfig->xBinarySemaphoreInternet);
    }
}

bool Wifi_reconnect()
{
    const wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED)
    {
        return true;
    }
    startSTA();
    return false;
}
