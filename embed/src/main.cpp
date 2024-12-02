#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <esp_task_wdt.h>
#include <HTTPClient.h>
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

StaticJsonDocument<200> sensornode;

float lux = 0;
float humidity = 0;
float temperature = 0;
float dustDensity = 0;


static unsigned long lastReadTime;

unsigned long startTime;

const char* scriptUrl = "https://script.google.com/macros/s/AKfycbyJjGmHs3FkzUM2DNj37QLqiZgetZMX-Gt6bURzYZ5v-9f2mmkk9cVZobij08M7X8DX/exec";

// Wi-Fi credentials
const char* ssid = "Vivo 11 pro max";
const char* password = "boom1514";


// Pin Definitions
#define DHTPIN 4
#define FAN_PIN 5
#define DHTTYPE DHT22
#define I2C_SDA 33
#define I2C_SCL 32

// Sensor Initialization
DHT dht(DHTPIN, DHTTYPE);


TaskHandle_t loopTask = NULL;

unsigned long startTime;

// Function Prototypes


// Setup Function
void setup() {
    Serial.begin(115200);

    // Initialize I2C
    pinMode(FAN_PIN, OUTPUT);
    digitalWrite(FAN_PIN, LOW);
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
    
    static unsigned long lastReadTime = 0;

    startTime=millis();
}

void controlFan(bool state) {
    digitalWrite(FAN_PIN, state ? HIGH : LOW);
    Serial.printf("Fan turned %s\n", state ? "ON" : "OFF");
}

StaticJsonDocument<200> sendRequestToSensorNode() {
  StaticJsonDocument<200> doc;
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        // Construct the full URL
        String url = String("http://172.20.10.6/get-data");

        Serial.println(url);

        http.begin(url); // Start the HTTP request
        int httpResponseCode = http.GET(); // Perform GET request

        // Check response
        if (httpResponseCode > 0) {
            Serial.printf("HTTP Response code: %d\n", httpResponseCode);
            String responseBody = http.getString();
            Serial.println("Response from sensor node:");
            Serial.println(responseBody);
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
    return doc;
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

void sendDataToGoogleSheets(float brightness, float dust, float humidity, float temperature) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(scriptUrl);
        url += "?brightness=";
        url += brightness;
        url += "&dust=";
        url += dust;
        url += "&humidity=";
        url += humidity;
        url += "&temperature=";
        url += temperature;
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Success");
      Serial.println(url);
    } else {
      Serial.println("Error on sending request");
    }
    http.end();
  }
}

// Main Loop
void loop() {
    // Use a timer to read sensors and perform actions
const unsigned long readInterval=30000;

    if (millis() - lastReadTime > readInterval) {
        lastReadTime = millis();
        reconnectWiFi();     // Reconnect Wi-Fi if disconnected
    }

    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    sensornode = sendRequestToSensorNode();
    lux = sensornode["brightness"];
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    dustDensity = sensornode["dustDensity"];
    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setFloat(&fbdo, "/sensors/brightness", lux)){
      Serial.println("PASSED1");
      Serial.println(fbdo.dataPath());
      Serial.println( fbdo.dataType());
    }
    else {
      Serial.println("FAILED1");
      Serial.println(fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "/sensors/dust", dustDensity)){
      Serial.println("PASSED2");
      Serial.println(fbdo.dataPath());
      Serial.println( fbdo.dataType());
    }
    else {
      Serial.println("FAILED2");
      Serial.println(fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "/sensors/humidity", humidity)){
      Serial.println("PASSED3");
      Serial.println(fbdo.dataPath());
      Serial.println( fbdo.dataType());
    }
    else {
      Serial.println("FAILED3");
      Serial.println(fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "/sensors/temperature", temperature)){
      Serial.println("PASSED4");
      Serial.println(fbdo.dataPath());
      Serial.println( fbdo.dataType());
    }
    else {
      Serial.println("FAILED4");
      Serial.println(fbdo.errorReason());
    }
    if (Firebase.RTDB.getString(&fbdo, "/sensors/fan")) {
        controlFan(fbdo.boolData());
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
            Serial.println(key);
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
            Serial.println(key);

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
            Serial.println(key);

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
            Serial.println(key);
        }
        else if(key == "reset") {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Embedded Project");
        }
            Serial.println(stringValue);
        }
    else {
      Serial.println(fbdo.errorReason());
    }
    if (millis()>startTime+10000){
    sendDataToGoogleSheets(lux, dustDensity, humidity, temperature);
    startTime=millis(); // Reset timer
  }
  }
}
