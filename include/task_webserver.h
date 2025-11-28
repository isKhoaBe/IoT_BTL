
#ifndef __TASK_WEBSERVER_H__
#define __TASK_WEBSERVER_H__

#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>
#include <task_handler.h>
#include <freertos/queue.h>

typedef struct
{
    int gpioPin;
    bool newState;
} DeviceControlCommand;

typedef struct
{
    String ssid;
    String password;
    String token;
    String server;
    int port;
} SettingsCommand;

extern AsyncWebServer server;
extern AsyncWebSocket ws;

void Webserver_stop();
void Webserver_reconnect();
void Webserver_sendata(String data);

extern QueueHandle_t xQueueRelayControl;
extern QueueHandle_t xQueueSettings;

void connnectWSV();
void handleWebSocketMessage(String message);
void Webserver_RTOS_Task(void *pvParameters);

#endif