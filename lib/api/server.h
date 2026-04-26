#pragma once

bool sendAttendance(const char* uuid, const char* timestamp, const int rssi, const char* door);
String getWhitelist();