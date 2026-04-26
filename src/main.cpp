#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>
#include <time.h>
#include <WiFi.h>
#include <wifi_secret.h>
#include <chip_info.h>
#include <parse_whitelist.h>

#define RELAY_PIN  5        //D5
#define RELAY_ON   LOW
#define RELAY_OFF  HIGH

void connectWifi(){
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
        delay(500);
        Serial.print(".");
        retry++;
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

struct PendingEvent {
    bool triggered = false;
    const char* uuid;
    int rssi;
};

PendingEvent pending;

class CustomScanCallbacks : public NimBLEScanCallbacks {
public:
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
        if (advertisedDevice->haveServiceUUID()) {
            std::string deviceUUID = advertisedDevice->getServiceUUID().toString();
            const char* uuid = deviceUUID.c_str();
            Serial.println("Device found:");
            Serial.printf("  Address : %s\n", advertisedDevice->getAddress().toString().c_str());
            Serial.printf("  RSSI    : %d dBm\n", advertisedDevice->getRSSI());
            Serial.printf("  Name    : %s\n", advertisedDevice->getName().c_str());
            Serial.printf("  Service UUIDs: %s\n", uuid);
            Serial.println(advertisedDevice->toString().c_str());
            
        }
        else {
            return;
        }
    }
};

CustomScanCallbacks scanCallbacks;

void ntpSetup() {
    const long gmtOffset_sec = 21600;
    const int daylightOffset_sec = 0;
    configTime(gmtOffset_sec, daylightOffset_sec, "asia.pool.ntp.org", "time.cloudflare.com", "time.google.com");
}

void printDateTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  char formattedTime[80];  // Buffer to store the formatted string
  strftime(formattedTime, sizeof(formattedTime), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  Serial.println(formattedTime);
}

void setup() {
    Serial.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, RELAY_OFF);

    NimBLEDevice::init("");
    delay(200);
    connectWifi();
    ntpSetup();
    printDateTime();
    ParseWhitelist();
    NimBLEScan* pScan = NimBLEDevice::getScan();
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
}