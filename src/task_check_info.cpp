#include "task_check_info.h"

void Load_info_File()
{
  File file = LittleFS.open("/info.dat", "r");
  if (!file)
  {
    return;
  }
  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
  }
  else
  {
    if (g_wifiConfig != NULL && g_wifiConfig->mutex != NULL) {
      if (xSemaphoreTake(g_wifiConfig->mutex, portMAX_DELAY) == pdTRUE) {
        g_wifiConfig->WIFI_SSID = strdup(doc["WIFI_SSID"]);
        g_wifiConfig->WIFI_PASS = strdup(doc["WIFI_PASS"]);
        g_wifiConfig->CORE_IOT_TOKEN = strdup(doc["CORE_IOT_TOKEN"]);
        g_wifiConfig->CORE_IOT_SERVER = strdup(doc["CORE_IOT_SERVER"]);
        g_wifiConfig->CORE_IOT_PORT = strdup(doc["CORE_IOT_PORT"]);
        xSemaphoreGive(g_wifiConfig->mutex);
      }
    }
  }
  file.close();
}

void Delete_info_File()
{
  if (LittleFS.exists("/info.dat"))
  {
    LittleFS.remove("/info.dat");
  }
  ESP.restart();
}

void Save_info_File(String wifi_ssid, String wifi_pass, String CORE_IOT_TOKEN, String CORE_IOT_SERVER, String CORE_IOT_PORT)
{
  Serial.println(wifi_ssid);
  Serial.println(wifi_pass);

  DynamicJsonDocument doc(4096);
  doc["WIFI_SSID"] = wifi_ssid;
  doc["WIFI_PASS"] = wifi_pass;
  doc["CORE_IOT_TOKEN"] = CORE_IOT_TOKEN;
  doc["CORE_IOT_SERVER"] = CORE_IOT_SERVER;
  doc["CORE_IOT_PORT"] = CORE_IOT_PORT;

  File configFile = LittleFS.open("/info.dat", "w");
  if (configFile)
  {
    serializeJson(doc, configFile);
    configFile.close();
  }
  else
  {
    Serial.println("Unable to save the configuration.");
  }
  ESP.restart();
};

bool check_info_File(bool check)
{
  if (!check)
  {
    if (!LittleFS.begin(true))
    {
      Serial.println("❌ Lỗi khởi động LittleFS!");
      return false;
    }
    Load_info_File();
  }
  
  bool isEmpty = true;
  if (g_wifiConfig != NULL && g_wifiConfig->mutex != NULL) {
    if (xSemaphoreTake(g_wifiConfig->mutex, portMAX_DELAY) == pdTRUE) {
      isEmpty = g_wifiConfig->WIFI_SSID.isEmpty() && g_wifiConfig->WIFI_PASS.isEmpty();
      xSemaphoreGive(g_wifiConfig->mutex);
    }
  }
  
  if (isEmpty)
  {
    if (!check)
    {
      startAP();
    }
    return false;
  }
  return true;
}