#include <WiFi.h>
#include <HTTPClient.h>
#include <api_key.h>
#include <ArduinoJson.h>
#include <server.h>

bool sendAttendance(const char* uuid, const char* timestamp, const int rssi, const char* door) {
    if (WiFi.status() != WL_CONNECTED) {
       Serial.println("WiFi down, skipping HTTP");
       return false;
    }
    HTTPClient http;
    http.begin(String(base_url) + "/api/attendance");
    http.setTimeout(5000);
    http.setReuse(false);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Admin-Key", api_key);
    JsonDocument payloadJson; 
    payloadJson["uuid"] = uuid;
    payloadJson["timestamp"] = timestamp;
    payloadJson["rssi"] = rssi;
    payloadJson["door"] = door;
    String body;
    body.reserve(256);
    bool success = false;
    serializeJson(payloadJson, body);
    int httpResponseCode = http.POST(body);
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response:");
        Serial.println(response);
        success = true;
    } 
    else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
        success = false;
    }
    http.end();
    return success;
}

String getWhitelist() {
    if (WiFi.status() != WL_CONNECTED) {
       Serial.println("WiFi down, skipping HTTP");
       return "";
    }
    HTTPClient http;
    http.begin(String(base_url) + "/api/whitelist");
    http.setTimeout(5000);
    http.setReuse(false);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Admin-Key", api_key);
    int httpResponseCode = http.GET();
    String response = "";
    if (httpResponseCode > 0) {
        response = http.getString();
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