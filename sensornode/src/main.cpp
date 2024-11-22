#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <BH1750.h>
#include <GP2YDustSensor.h>

// Wi-Fi credentials
const char* ssid = "Vivo 11 pro max";
const char* password = "boom1514";

// Pin Definitions for GP2Y1014AU0F
const int ledPin = 2; // GP2Y1014 LED control pin
const int voPin = 36;

// Sensor Initialization
BH1750 lightMeter(0x23);
GP2YDustSensor dustSensor(GP2YDustSensorType::GP2Y1014AU0F, ledPin, voPin);
#define I2C_SDA 33
#define I2C_SCL 32

// Web Server Initialization
AsyncWebServer server(80);

void setup() {
    Serial.begin(115200);
    Wire.begin(I2C_SDA, I2C_SCL);

    // Initialize I2C for BH1750
    Wire.begin(18, 19); // Replace with your SDA and SCL pins
    if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
        Serial.println("Error initializing BH1750 sensor!");
    }
    Serial.println("BH1750 sensor initialized!");

    // Initialize GP2Y1014AU0F Dust Sensor
    dustSensor.begin();
    Serial.println("GP2Y1014AU0F Dust Sensor initialized!");

    // Connect to Wi-Fi
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi connected!");
    Serial.println("IP address: " + WiFi.localIP());

    // Set up server endpoints
    server.on("/get-data", HTTP_GET, [](AsyncWebServerRequest* request) {
        float lux = lightMeter.readLightLevel();
        float dustDensity = dustSensor.getDustDensity();

        // Create JSON response
        String jsonResponse = "{";
        jsonResponse += "\"brightness\":";
        jsonResponse += String(lux);
        jsonResponse += ",";
        jsonResponse += "\"dustDensity\":";
        jsonResponse += String(dustDensity);
        jsonResponse += "}";

        request->send(200, "application/json", jsonResponse);
        Serial.println("Data sent to gateway: " + jsonResponse);
    });

    server.begin();
}

void loop() {
    // You can implement any additional periodic tasks here if necessary
}
