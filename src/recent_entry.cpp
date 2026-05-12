// recent_entry.cpp
#include <Arduino.h>
#include <recent_entry.h>

struct RecentAuth
{
    char uuid[37]; // 36 characters + null terminator
    unsigned long timestamp; // Unix timestamp in seconds
};

RecentAuth recentAuths[30]; // Array to hold the 30 most recent entries

void addRecentAuth(const char* uuid) {
    unsigned long currentTime = millis();
    for (int i = 0; i < 30; i++) {
        if (recentAuths[i].timestamp == 0) {
            strcpy(recentAuths[i].uuid, uuid);
            recentAuths[i].timestamp = currentTime;
            return;
        }
    }
}

bool isRecentlyAuthorized(const char* uuid) {
    unsigned long currentTime = millis();
    for (int i = 0; i < 30; i++) {
        if (recentAuths[i].timestamp == 0) continue;
        if (currentTime - recentAuths[i].timestamp > 60000) {
            recentAuths[i].timestamp = 0;
            continue;
        }
        if (strcmp(recentAuths[i].uuid, uuid) == 0) return true;
    }
    return false;
}