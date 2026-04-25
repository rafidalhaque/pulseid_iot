#include <HTTPClient.h>
#include <api_key.h>
#include <ArduinoJson.h>


void sendAttendance(const char* uuid, const char* timestamp, const int rssi, const char* door) {
    HTTPClient http;
    http.begin("http://192.168.0.151:5000/api/attendance");
    http.setTimeout(5000);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", api_key);
    StaticJsonDocument<256> payloadJson; 
    payloadJson["uuid"] = uuid;
    payloadJson["timestamp"] = timestamp;
    payloadJson["rssi"] = rssi;
    payloadJson["door"] = door;
    String body;
    serializeJson(payloadJson, body);
    int httpResponseCode = http.POST(body);
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response:");
        Serial.println(response);
    } 
    else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}

void getWhitelist() {
    HTTPClient http;
    http.begin("http://192.168.0.151:5000/api/whitelist");
    http.setTimeout(5000);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", api_key);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response:");
        Serial.println(response);
    } 
    else {
        Serial.print("Error on sending GET: ");
        Serial.println(httpResponseCode);
    }
    http.end();
    return response;
}

