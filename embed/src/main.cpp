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
#include <esp_task_wdt.h>
#include <HTTPClient.h>
#include <GP2YDustSensor.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <Firebase_ESP_Client.h>
// Insert Firebase project API Key
#define API_KEY "AIzaSyCvi4o1NE3QGjF-OdLKfCpYiOs7hKksoNY"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://embeded-953a1-default-rtdb.firebaseio.com/"

#define FIREBASE_DISABLE_FCM
#define FIREBASE_DISABLE_FIRESTORE
#define FIREBASE_DISABLE_STORAGE

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

LiquidCrystal_I2C lcd(0x27, 16, 2); // 16x2 LCD

float lux = 0;
float humidity = 0;
float temperature = 0;
float dustDensity = 0;

const char* scriptUrl = "https://script.google.com/macros/s/AKfycbw0PQADYWhKyAFcIkJuvkAL3zlN9H-MUoEt95Vph4nV0ZFt3qcREqAo4tkqt-y9AV9d/exec";

// Wi-Fi credentials
const char* ssid = "Vivo 11 pro max";
const char* password = "boom1514";

const char* sensorNodeIP = "172.20.10.6";
const char* sensorNodeEndpoint = "/get-data";

// Pin Definitions
#define DHTPIN 4
#define DHTTYPE DHT22
#define I2C_SDA 33
#define I2C_SCL 32




// Sensor Initialization
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

// Web Server Initialization
AsyncWebServer server(80);


TaskHandle_t loopTask = NULL;

// Function Prototypes

StaticJsonDocument<200> sendRequestToSensorNode() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        // Construct the full URL
        String url = String("http://");
        url += sensorNodeIP;
        url += sensorNodeEndpoint;

        Serial.println(url);

        http.begin(url); // Start the HTTP request
        int httpResponseCode = http.GET(); // Perform GET request

        // Check response
        if (httpResponseCode > 0) {
            Serial.printf("HTTP Response code: %d\n", httpResponseCode);
            String responseBody = http.getString();
            Serial.println("Response from sensor node:");
            Serial.println(responseBody);
            StaticJsonDocument<200> doc;
            deserializeJson(doc, responseBody);
            return doc;
            // Parse or process the JSON response here if needed
            // Example: Extract sensor values from JSON
        } else {
            Serial.printf("Error on HTTP request: %s\n", http.errorToString(httpResponseCode).c_str());
        }

        http.end(); // End the HTTP connection
    } else {
        Serial.println("WiFi is not connected. Cannot send request.");
    }
}

void reconnectWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Reconnecting to Wi-Fi...");
        WiFi.disconnect();
        WiFi.begin(ssid, password);

        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            delay(500);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nReconnected to Wi-Fi!");
        } else {
            Serial.println("\nFailed to reconnect to Wi-Fi.");
        }
    }
}

TaskHandle_t emailTaskHandle = NULL;

void sendDataToGoogleSheets(float humidity,float temperature,float brightness) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(scriptUrl);
        url += "?brightness=";
        url += brightness;
        url += "&temperature=";
        url += temperature;
        url += "&humidity=";
        url += humidity;
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


// Setup Function
void setup() {
    Serial.begin(115200);

    // Initialize I2C
    Wire.begin(I2C_SDA, I2C_SCL);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("LCD Initialized");

    loopTask = xTaskGetHandle("loopTask");
    if (loopTask != NULL) {
        esp_task_wdt_delete(loopTask);
    }

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
    if (!SPIFFS.begin(true)) {  // Force format on failure
    Serial.println("SPIFFS Mount Failed! Formatting...");
    if (!SPIFFS.format()) {
        Serial.println("SPIFFS Formatting Failed!");
    }
    } else {
    Serial.println("SPIFFS Mounted Successfully!");
    }

    // List SPIFFS contents for debugging
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
    Serial.println(file.name());
    file = root.openNextFile();
    file.close(); // Make sure to close the file handle
    }
    // Configure web server endpoints
    // server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    //     request->send(SPIFFS, "/index.html", "text/html");
    // });
    // server.on("/scan", HTTP_GET, [](AsyncWebServerRequest* request) {
    //     StaticJsonDocument<200> sensornode = sendRequestToSensorNode();
    //     lux = sensornode["brightness"];
    //     humidity = dht.readHumidity();
    //     temperature = dht.readTemperature();
    //     dustDensity = sensornode["dustDensity"];
    //     Serial.printf("Humidity: %.2f%%  Temperature: %.2f°C\n lux: %.2flux\n dustDensity: %.2fu\n", humidity, temperature,lux,dustDensity);

    //     // Create JSON response
    //     String jsonResponse = "{";
    //     jsonResponse += "\"humidity\":";
    //     jsonResponse += String(humidity);
    //     jsonResponse += ",";
    //     jsonResponse += "\"temperature\":" ;
    //     jsonResponse += String(temperature);
    //     jsonResponse += ",";
    //     jsonResponse += "\"dust\":"; 
    //     jsonResponse += String(dustDensity);
    //     jsonResponse += ",";
    //     jsonResponse += "\"brightness\":" ;
    //     jsonResponse += String(lux);
    //     jsonResponse += "}";
    //     request->send(200, "application/json", jsonResponse); // Respond with JSON
    // });
    // server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest* request) {
    //     request->send(SPIFFS, "/script.js", "application/javascript");
    // });

    // server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest* request) {
    //     request->send(SPIFFS, "/styles.css", "text/css");
    // });
//     server.on("/process-voice", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
//     StaticJsonDocument<200> jsonDoc;
//     DeserializationError error = deserializeJson(jsonDoc, data);

//     if (error) {
//       Serial.println("Failed to parse JSON");
//       request->send(400, "application/json", "{\"message\":\"Invalid JSON\"}");
//       return;
//     }

//     const char* word = jsonDoc["word"];
//     String key = String(word);
//     if (key == "humidity") {
//         key+=" : " ;
//         key  += String(humidity);
//         key +="% RH";
//     }
//     else if(key == "dust") {
//         key+=" : " ;
//         key  += String(dustDensity);
//         key+="µg/m³";
//     }
//     else if(key == "temperature") {
//         key+=" : " ;
//         key  += String(temperature);
//         key +="°C";
//     }
//     else if(key == "brightness") {
//         key+=" : " ;
//         key  += String(lux);
//         key+="lx";
//     }
//     lcd.print(key);
//   });

    // Start the server
    server.begin();
    dht.begin();

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

      /* Sign up */
    if (Firebase.signUp(&config, &auth, "", "")){
        Serial.println("ok");
        signupOK = true;
    }
    else{
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }


    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

// Main Loop
void loop() {
    // Use a timer to read sensors and perform actions
    static unsigned long lastReadTime = 0;
    const unsigned long readInterval = 30000; // 5 seconds

    if (millis() - lastReadTime > readInterval) {
        lastReadTime = millis();
        reconnectWiFi();     // Reconnect Wi-Fi if disconnected
    }

    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    StaticJsonDocument<200> sensornode = sendRequestToSensorNode();
    lux = sensornode["brightness"];
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    dustDensity = sensornode["dustDensity"];
    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setFloat(&fbdo, "/sensors/brightness", lux)){
      Serial.println("PASSED");
      Serial.println(fbdo.dataPath());
      Serial.println( fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println(fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "/sensors/dust", dustDensity)){
      Serial.println("PASSED");
      Serial.println(fbdo.dataPath());
      Serial.println( fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println(fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "/sensors/humidity", humidity)){
      Serial.println("PASSED");
      Serial.println(fbdo.dataPath());
      Serial.println( fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println(fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "/sensors/temperature", temperature)){
      Serial.println("PASSED");
      Serial.println(fbdo.dataPath());
      Serial.println( fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println(fbdo.errorReason());
    }
    if (Firebase.RTDB.getString(&fbdo, "/sensors/AI")) {
        String stringValue = fbdo.stringData();
        String key = String(stringValue);
        if (key == "humidity") {
            key+=" : " ;
            String keys  = String(humidity);
            keys +=" RH";
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(key);
            lcd.setCursor(0,1);
            lcd.print(keys);
        }
        else if(key == "dust") {
            key+=" : " ;
            String keys  = String(dustDensity);
            keys+=" ug/m^3";
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(key);
            lcd.setCursor(0,1);
            lcd.print(keys);
        }
        else if(key == "temperature") {
            key+=" : " ;
            String keys  = String(temperature);
            keys +=" C";
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(key);
            lcd.setCursor(0,1);
            lcd.print(keys);
        }
        else if(key == "brightness") {
            key+=" : " ;
            String keys  = String(lux);
            keys+=" lx";
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(key);
            lcd.setCursor(0,1);
            lcd.print(keys);
        }
        else if(key == "reset") {
            lcd.clear();
        }
            Serial.println(stringValue);
        }
    else {
      Serial.println(fbdo.errorReason());
    }
  }
}
