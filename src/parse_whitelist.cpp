#include <parse_whitelist.h>
#include <gen_ble_uuid.h>
#include <TelnetLogger.h>

WhitelistEntry whitelist[MAX_MEMBERS];
int whitelistcount = 0;

void ParseWhitelist() {
    String whitelistResp = getWhitelist();
    if (whitelistResp == "") return;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, whitelistResp);

    if (error) {
        logMsg("Failed to parse whitelist JSON: ");
        logMsg(error.c_str());
        return;
    }
    JsonArray arr = doc["whitelist"].as<JsonArray>();
    logMsg("Parsed whitelist:");
    whitelistcount = 0;

    for (JsonObject entry : arr) {
        if (whitelistcount >= MAX_MEMBERS) {
            logMsg("Whitelist is full, skipping remaining entries.");
            break;
        }
        String member_id = entry["employee_id"].as<String>();
        String member_key = entry["employee_key"].as<String>();
        logMsgf("  Member ID: %s, Member Key: %s\n", member_id.c_str(), member_key.c_str());
        whitelist[whitelistcount].member_id = member_id;
        whitelist[whitelistcount].member_key = member_key;
        whitelist[whitelistcount].ble_uuid = genBLEuuid(member_key.c_str(), member_id.c_str());

        logMsgf("Whitelist count: %d\n", whitelistcount);
        logMsgf("Generated BLE UUID: %s\n", whitelist[whitelistcount].ble_uuid.c_str());
        whitelistcount++;
    }
}