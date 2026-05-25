#pragma once

const char* get_baseUrl();
const char* get_apiKey();
bool sendAttendance(const char* uuid, const char* timestamp, const int rssi, const char* door);
String getWhitelist();
String pollCmd();
bool sendPong(unsigned long uptime_ms, const int free_heap, const char* wifi_ip, int wifi_rssi, const char* ntpTime);