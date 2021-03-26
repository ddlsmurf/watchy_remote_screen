#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "mdns.h"
#include "all_fonts.h"

#define CS 5
#define DC 10
#define RESET 9
#define BUSY 19

#define WIFI_SSID "your wifi's name"
#define WIFI_PASS "and it's password"

#define MDNS_HOSTNAME "WRS"

// curl -v -F "data=@test.bin" http://192.168.83.46/raw
// curl -v -F "data=@white.bin" http://192.168.83.46/raw
// curl -v -F "data=@black.bin" http://192.168.83.46/raw
// curl -v http://192.168.83.46/fonts
// curl -v -d "f=FreeMonoBold9pt7b&t=hello&x=0&y=50&c=1&i=0" http://192.168.83.46/drawText

AsyncWebServer server(80);

GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(CS, DC, RESET, BUSY));

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.printf("Setup core ID: %i\n", xPortGetCoreID());
    Serial.println("Startup...");
    if (!SPIFFS.begin())
    {
        Serial.println("SPIFFS Error...");
        return;
    }
    display.init(0, false); //_initial_refresh to false to prevent full update on init
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);
    display.setFont(fonts["FreeSans9pt7b"]);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(30, 30);
    display.println("Connecting...");
    display.display(false); //partial refresh
    display.hibernate();
    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        if (WiFi.waitForConnectResult() != WL_CONNECTED)
        {
            Serial.println("WiFi Failed!");
            return;
        }
    }
    Serial.println("Wifi connnected");

    esp_err_t err = mdns_init();
    if (err) {
        printf("MDNS Init failed: %d\n", err);
    } else {
        mdns_hostname_set(MDNS_HOSTNAME);
        mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
    }

    display.fillScreen(GxEPD_WHITE);
    display.setCursor(30, 30);
    display.println("Ready");
    display.print("http://");
    display.println(WiFi.localIP());
    display.println("http://" MDNS_HOSTNAME ".local");
    display.display(true);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });
    server.on("/fonts", HTTP_GET, [](AsyncWebServerRequest *request) {
        std::string result = "";
        for (auto font : fonts)
        {
            if (result.length() > 0)
                result += ",";
            result += font.first;
        }
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", result.c_str());
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });
    server.on(
        "/raw", HTTP_POST, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response); },
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            if (len)
            {
                /* If the next line fails, you need to patch .pio/libdeps/esp32dev/GxEPD2/src/GxEPD2_BW.h
                    Add this to a public section of class GxEPD2_BW:
                    uint8_t *getBuffer() {
                        return _buffer;
                    }
                */
                uint8_t *buffer = display.getBuffer();
                memcpy(buffer + index, data, len);
            }
            if (final)
                display.display(true);
        });
    server.on("/drawText", HTTP_POST, [](AsyncWebServerRequest *request) {
        const String &text = request->arg("t");
        const String &fontName = request->arg("f");
        const int x = request->arg("x").toInt();
        const int y = request->arg("y").toInt();
        const int clear = request->arg("c").toInt();
        const int invert = request->arg("i").toInt();
        const GFXfont *font = fonts[fontName.c_str()];
        if (!font)
            return request->send(400, "text/plain", "Invalid font name");
        if (clear)
            display.fillScreen(invert ? GxEPD_BLACK : GxEPD_WHITE);
        display.setFont(font);
        display.setTextColor(invert ? GxEPD_WHITE : GxEPD_BLACK);
        display.setCursor(x, y);
        display.println(text);
        display.display(false);

        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });

    server.begin();
    Serial.println("Server started");
}

void loop()
{
}