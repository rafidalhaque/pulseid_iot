#include <HTTPClient.h>
#include <api_key.h>
#include <ArduinoJson.h>


bool sendAttendance(const char* uuid, const char* timestamp, const int rssi, const char* door) {
    HTTPClient http;
    http.begin(String(base_url) + "/api/attendance");
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
    if (WiFi.status() != WL_CONNECTED) {
       Serial.println("WiFi down, skipping HTTP");
       return false;
    }
    int httpResponseCode = http.POST(body);
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response:");
        Serial.println(response);
        return true;
    } 
    else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
        return false;
    }
    http.end();
}

String getWhitelist() {
    HTTPClient http;
    http.begin(String(base_url) + "/api/whitelist");
    http.setTimeout(5000);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", api_key);
    if (WiFi.status() != WL_CONNECTED) {
       Serial.println("WiFi down, skipping HTTP");
       return "";
    }
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

