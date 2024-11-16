#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SPIFFS.h>
#include <Wire.h>
#include <BH1750.h>
#include <HTTPClient.h>

const char* ssid = "Vivo 11 pro max";
const char* password = "boom1514";
#define DHTPIN 4    
#define DHTTYPE DHT22
BH1750 lightMeter;
const char* scriptUrl = "https://script.google.com/macros/s/AKfycbw0PQADYWhKyAFcIkJuvkAL3zlN9H-MUoEt95Vph4nV0ZFt3qcREqAo4tkqt-y9AV9d/exec";
float brightness = 150;
float temperature = 24.5;
float humidity = 60.0;

AsyncWebServer server(80);

DHT dht(DHTPIN, DHTTYPE);

#define I2C_SDA 33
#define I2C_SCL 32

void setup() {
    Serial.begin(115200);
    Wire.begin(I2C_SDA, I2C_SCL);

    // Connect to Wi-Fi
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS!");
        return;
    }

    // List SPIFFS contents
    Serial.println("SPIFFS contents:");
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
        Serial.println(file.name());
        file = root.openNextFile();
    }

    if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
        Serial.println("Error initializing BH1750 sensor!");
        while (1);
    }
    Serial.println("BH1750 sensor initialized!");

    // Serve index.html
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (SPIFFS.exists("/index.html")) {
            request->send(SPIFFS, "/index.html", "text/html");
        } else {
            request->send(404, "text/plain", "index.html not found");
        }
    });

    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/script.js", "application/javascript");
    });

    // Start the server
    server.begin();
    dht.begin();
}

void sendDataToGoogleSheets() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(scriptUrl) + "?brightness=" + brightness + "&temperature=" + temperature + "&humidity=" + humidity;
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Success");
    } else {
      Serial.println("Error on sending request");
    }
    http.end();
  }
}

void loop() {
  sendDataToGoogleSheets();

    delay(2000);
    float lux = lightMeter.readLightLevel();
    Serial.print("Light Intensity: ");
    Serial.print(lux);
    Serial.println(" lx");

    // Read temperature and humidity
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    // Check if any reading failed
    if (isnan(humidity) || isnan(temperature)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    // Print readings to the Serial Monitor
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("%  Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");
}
