#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define RELAY_PIN 5  // D5
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

const char* ssid = "Teacher's Home";
const char* password = "My_Home#@6110";

void connectWifi(){
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
        delay(500);
        Serial.print(".");
        retry++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } 
    else {
        Serial.println("\nFailed to connect to WiFi");
    }
}
void setup() {
    Serial.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, RELAY_OFF); // Ensure relay is off at startup
    connectWifi();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED){
        Serial.println("WiFi not connected, attempting to reconnect...");
        connectWifi();
    }
    Serial.println("Relay ON");
    digitalWrite(RELAY_PIN, RELAY_ON);
    delay(500);

    Serial.println("Relay OFF");
    digitalWrite(RELAY_PIN, RELAY_OFF);
    delay(500);
}