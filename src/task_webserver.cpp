#include "task_webserver.h"
#include "global.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void Webserver_sendata(String data)
{
    if (ws.count() > 0)
    {
        ws.textAll(data); // Gửi đến tất cả client đang kết nối
        Serial.println("📤 Đã gửi dữ liệu qua WebSocket: " + data);
    }
    else
    {
        Serial.println("⚠️ Không có client WebSocket nào đang kết nối!");
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;

        if (info->opcode == WS_TEXT)
        {
            String message;
            message += String((char *)data).substring(0, len);
            // parseJson(message, true);
            handleWebSocketMessage(message);
        }
    }
}

void connnectWSV()
{
    ws.onEvent(onEvent);
    server.addHandler(&ws);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/script.js", "application/javascript"); });
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/styles.css", "text/css"); });
    server.begin();
    ElegantOTA.begin(&server);
    
    // Set webserver running state with mutex protection
    if (g_wifiConfig != NULL && g_wifiConfig->mutex != NULL) {
        if (xSemaphoreTake(g_wifiConfig->mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            g_wifiConfig->webserver_isrunning = true;
            xSemaphoreGive(g_wifiConfig->mutex);
        }
    }
}

void Webserver_stop()
{
    ws.closeAll();
    server.end();
    
    // Set webserver stopped state with mutex protection
    if (g_wifiConfig != NULL && g_wifiConfig->mutex != NULL) {
        if (xSemaphoreTake(g_wifiConfig->mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            g_wifiConfig->webserver_isrunning = false;
            xSemaphoreGive(g_wifiConfig->mutex);
        }
    }
}

void Webserver_reconnect()
{
    // Check webserver state with mutex protection
    bool isRunning = false;
    if (g_wifiConfig != NULL && g_wifiConfig->mutex != NULL) {
        if (xSemaphoreTake(g_wifiConfig->mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            isRunning = g_wifiConfig->webserver_isrunning;
            xSemaphoreGive(g_wifiConfig->mutex);
        }
    }
    
    if (!isRunning)
    {
        connnectWSV();
    }
    ElegantOTA.loop();
}
