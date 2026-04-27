#pragma once
#include <ArduinoJson.h>
#include <server.h>
#define MAX_MEMBERS 25

struct WhitelistEntry {
    String member_id;
    String member_key;
    String ble_uuid;
};
extern WhitelistEntry whitelist[MAX_MEMBERS];
extern int whitelistcount;

void ParseWhitelist();