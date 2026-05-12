#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>
#include <time.h>
#include <mbedtls/md.h>
#include <WiFi.h>
#include <wifi_secret.h>
#include <chip_info.h>
#include <parse_whitelist.h>
#include <recent_entry.h>

#define RELAY_PIN  5        //D5
#define RELAY_ON   LOW
#define RELAY_OFF  HIGH

void connectWifi(){
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
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

bool isAuthorized(const std::string& uuid) {
    for (int i = 0; i < whitelistcount; i++) {
        if (uuid == whitelist[i].ble_uuid.c_str()) {
            Serial.printf("Authorized device detected: %s\n", uuid.c_str());
            return true;
        }
    }
    return false; 
}
struct PendingEvent {
    bool triggered = false;
    std::string uuid;
    int rssi;
};

PendingEvent pending;

class CustomScanCallbacks : public NimBLEScanCallbacks {
public:
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
        if (advertisedDevice->haveServiceUUID()) {
            std::string deviceUUID = advertisedDevice->getServiceUUID().toString();
            if (isAuthorized(deviceUUID)) {
                pending.triggered = true;
                pending.uuid = deviceUUID;
                pending.rssi = advertisedDevice->getRSSI();
            }
            Serial.println("Device found:");
            Serial.printf("  Address : %s\n", advertisedDevice->getAddress().toString().c_str());
            Serial.printf("  RSSI    : %d dBm\n", advertisedDevice->getRSSI());
            Serial.printf("  Name    : %s\n", advertisedDevice->getName().c_str());
            Serial.printf("  Service UUIDs: %s\n", deviceUUID.c_str());
            Serial.println(advertisedDevice->toString().c_str());
            
        }
        else {
            return;
        }
    }
};

CustomScanCallbacks scanCallbacks;

struct tm timeinfo;

void ntpSetup() {
    const long gmtOffset_sec = 21600;
    const int daylightOffset_sec = 0;
    configTime(gmtOffset_sec, daylightOffset_sec, "asia.pool.ntp.org", "time.cloudflare.com", "time.google.com");

    int retry = 0;
    Serial.print("Waiting for NTP sync");
    while (!getLocalTime(&timeinfo) && retry < 20) {
        Serial.print(".");
        delay(100);
        retry++;
    }
    if (retry >= 20) {
        Serial.println("\nNTP sync FAILED");
    } else {
        Serial.println("\nNTP sync OK");
    }
}

String printDateTime() {
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "Failed to obtain time";
  }
  char formattedTime[80];  // Buffer to store the formatted string
  strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  Serial.println(formattedTime);
  return String(formattedTime);
}

NimBLEScan* pScan = nullptr;

void setup() {
    delay(3000);
    Serial.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, RELAY_OFF);

    NimBLEDevice::init("");
    delay(200);
    connectWifi();
    ntpSetup();
    delay(200);
    printDateTime();
    ParseWhitelist();
    pScan = NimBLEDevice::getScan();
    pScan->setScanCallbacks(&scanCallbacks);  
    pScan->setActiveScan(true);
    pScan->setDuplicateFilter(false);
    chipInfo();
    bool scan_started = pScan->start(0, false);
    Serial.println(scan_started ? "BLE scanning... hold phone close" : "FAILED to start");
}

void loop() {
    if (WiFi.status() != WL_CONNECTED){
        Serial.println("WiFi not connected, attempting to reconnect...");
        connectWifi();
    }
    delay(1000);
    if (pending.triggered) {
        Serial.printf("Pending event: UUID=%s, RSSI=%d\n", pending.uuid.c_str(), pending.rssi);
        digitalWrite(RELAY_PIN, RELAY_ON);
        delay(2000);
        digitalWrite(RELAY_PIN, RELAY_OFF);
        Serial.println("Access granted, relay activated.");
        pScan->stop();
        String timestamp = printDateTime();
        bool authorized = isRecentlyAuthorized(pending.uuid.c_str());
        if (!authorized) {
            addRecentAuth(pending.uuid.c_str());
            bool status = sendAttendance(pending.uuid.c_str(), timestamp.c_str(), pending.rssi, "MainDoor");
            if (status) {
                Serial.println("Attendance logged successfully.");
            } else {
                Serial.println("Failed to log attendance.");
            }
        }
        pending.triggered = false;
        pScan->start(0, false);
    }
    delay(1000);
}