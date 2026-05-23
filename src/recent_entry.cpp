// recent_entry.cpp
#include <Arduino.h>
#include <recent_entry.h>
#define ENTRY_THREASHOLD 60000
struct RecentAuth
{
    char uuid[37]; // 36 characters + null terminator
    unsigned long timestamp; // Unix timestamp in seconds
};

RecentAuth recentAuths[30]; // Array to hold the 30 most recent entries

void addRecentAuth(const char* uuid) {
    unsigned long currentTime = millis();
    int emptySlot = -1;
    for (int i = 0; i < 30; i++) {
        if (recentAuths[i].timestamp != 0 && currentTime - recentAuths[i].timestamp > ENTRY_THREASHOLD){
            recentAuths[i].timestamp = 0;
        }
        if (recentAuths[i].timestamp == 0 && emptySlot == -1) {
            emptySlot = i;
        }
    }
    if (emptySlot != -1){
        strcpy(recentAuths[emptySlot].uuid, uuid);
        recentAuths[emptySlot].timestamp = currentTime;
    }
    else {
        int oldest = 0;
        for (int i = 1; i < 30; i++) {
            if (recentAuths[i].timestamp < recentAuths[oldest].timestamp) {
                oldest = i;
            }
        }
        strcpy(recentAuths[oldest].uuid, uuid);
        recentAuths[oldest].timestamp = currentTime;
    }
}

bool isRecentlyAuthorized(const char* uuid) {
    unsigned long currentTime = millis();
    for (int i = 0; i < 30; i++) {
        if (recentAuths[i].timestamp == 0) continue;
        if (currentTime - recentAuths[i].timestamp > ENTRY_THREASHOLD) {
            recentAuths[i].timestamp = 0;
            continue;
        }
        if (strcmp(recentAuths[i].uuid, uuid) == 0) return true;
    }
    return false;
}