#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>
#include <time.h>
#include <mbedtls/md.h>
#include <WiFi.h>
#include <Preferences.h>
#include <ArduinoOTA.h>
#include <secrets.h>
#include <chip_info.h>
#include <parse_whitelist.h>
#include <recent_entry.h>
#include <TelnetLogger.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Adafruit_SH110X.h>

#define RELAY_PIN  5        //D5
#define RELAY_ON   LOW
#define RELAY_OFF  HIGH
#define RSSI_THREASHOLD -75

String ssidStr;
String passwordStr;
String baseUrlStr;
String apiKeyStr;
String otaPass;
Preferences prefs;

void loadSecrets(){
    prefs.begin("pulseid_secrets", true);
    ssidStr = prefs.getString("wifi_ssid", "");
    passwordStr = prefs.getString("wifi_password", "");
    baseUrlStr = prefs.getString("base_url", "");
    apiKeyStr = prefs.getString("api_key", "");
    otaPass = prefs.getString("ota_password", "");
    prefs.end();
}

void setupOTA(){
    ArduinoOTA.setHostname("pulseid-esp32");
    ArduinoOTA.setPassword(otaPass.c_str());
    ArduinoOTA.onStart([]() { logMsg("OTA Started");});
    ArduinoOTA.onEnd([]() { logMsg("OTA Ended");});
    ArduinoOTA.onError([](ota_error_t error) {
        logMsgf("OTA Error: [%u]\n", error);
    });
    ArduinoOTA.begin();
    logMsg("OTA Ready");
}

void connectWifi(){
    WiFi.begin(ssidStr, passwordStr);
    Serial.print("Connecting to WiFi");

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        logMsg("\nConnected to WiFi");
        Serial.print("IP Address: ");
        logMsg(WiFi.localIP().toString());
    } 
    else {
        logMsg("\nFailed to connect to WiFi");
    }
}

bool isAuthorized(const std::string& uuid, const int rssi) {
    for (int i = 0; i < whitelistcount; i++) {
        if (uuid == whitelist[i].ble_uuid.c_str() && rssi >= RSSI_THREASHOLD) {
            logMsgf("Authorized device detected: %s\n", uuid.c_str());
            return true;
        }
    }
    return false; 
}
struct PendingEvent {
    volatile bool triggered = false;
    std::string uuid;
    int rssi;
};

PendingEvent pending;

class CustomScanCallbacks : public NimBLEScanCallbacks {
public:
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
        if (advertisedDevice->haveServiceUUID()) {
            std::string deviceUUID = advertisedDevice->getServiceUUID().toString();
            int deviceRSSI = advertisedDevice->getRSSI();
            if (isAuthorized(deviceUUID, deviceRSSI)) {
                pending.triggered = true;
                pending.uuid = deviceUUID;
                pending.rssi = deviceRSSI;
            }
            // logMsg("Device found:");
            // logMsgf("  Address : %s\n", advertisedDevice->getAddress().toString().c_str());
            // logMsgf("  RSSI    : %d dBm\n", advertisedDevice->getRSSI());
            // logMsgf("  Name    : %s\n", advertisedDevice->getName().c_str());
            // logMsgf("  Service UUIDs: %s\n", deviceUUID.c_str());
            // logMsg(advertisedDevice->toString().c_str());
            
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
        logMsg("\nNTP sync FAILED");
    } else {
        logMsg("\nNTP sync OK");
    }
}

String printDateTime() {
  if (!getLocalTime(&timeinfo)) {
    logMsg("Failed to obtain time");
    return "Failed to obtain time";
  }
  char formattedTime[80];  // Buffer to store the formatted string
  strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  logMsg(formattedTime);
  return String(formattedTime);
}

NimBLEScan* pScan = nullptr;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
bool oledAvailable = false;
void setup() {
    delay(3000);
    Serial.begin(115200);
    Wire.begin(21, 22);  // SDA, SCL
    Wire.setClock(100000);
    delay(3000);
    for (byte addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("I2C device at 0x%02X\n", addr);
        }
    }
    Serial.println("scan done");
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(0xFF);
    if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        oledAvailable = true;
        display.fillScreen(SSD1306_WHITE);
        display.display();
        delay(2000);
        Serial.println("OLED OK"); 
    } else {
        Serial.println("OLED FAIL"); 
    }
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, RELAY_OFF);

    NimBLEDevice::init("");
    delay(200);
    loadSecrets();
    connectWifi();
    ntpSetup();
    delay(200);
    setupOTA();
    setupTelnet();
    delay(200);
    printDateTime();
    ParseWhitelist();
    pScan = NimBLEDevice::getScan();
    pScan->setScanCallbacks(&scanCallbacks);  
    pScan->setActiveScan(false);
    pScan->setDuplicateFilter(true);
    pScan->setMaxResults(0);
    chipInfo();
    bool scan_started = pScan->start(0, false);
    logMsg(scan_started ? "BLE scanning... hold phone close" : "FAILED to start");
    if (oledAvailable) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.print("PulseID Ready");
        display.display();
    }
}

unsigned long lastPoll = 0;
const unsigned long POLL_INTERVAL = 10000; 
unsigned long lastWLPoll = 0;
const unsigned long WLIST_POLLING = 3600000UL;
void loop() {
    if (WiFi.status() != WL_CONNECTED){
        logMsg("WiFi not connected, attempting to reconnect...");
        connectWifi();
    }
    loopTelnet();
    ArduinoOTA.handle();
    if (pending.triggered) {
        pScan->stop();
        std::string uuid = pending.uuid;
        int rssi = pending.rssi;
        logMsgf("Pending event: UUID=%s, RSSI=%d\n", uuid.c_str(), rssi);
        bool authorized = isRecentlyAuthorized(uuid.c_str());
        digitalWrite(RELAY_PIN, RELAY_ON);
        delay(10000);
        loopTelnet();
        digitalWrite(RELAY_PIN, RELAY_OFF);
        logMsg("Access granted, relay activated.");
        pending.triggered = false;
        String timestamp = printDateTime();
        if (!authorized) {
            addRecentAuth(uuid.c_str());
            bool status = sendAttendance(uuid.c_str(), timestamp.c_str(), rssi, "MainDoor");
            if (status) {
                logMsg("Attendance logged successfully.");
            } else {
                logMsg("Failed to log attendance.");
            }
        }
        pScan->start(0, false);
    }
    if (millis() - lastPoll >= POLL_INTERVAL) {
        lastPoll = millis();
        String cmd = pollCmd();
        if (cmd == "reboot") ESP.restart();
        
        unsigned long uptime_ms = millis();
        int free_heap = ESP.getFreeHeap();
        String wifi_ip = WiFi.localIP().toString();
        int wifi_rssi = WiFi.RSSI();
        sendPong(uptime_ms, free_heap, wifi_ip.c_str(), wifi_rssi, printDateTime().c_str());
    }
    if (millis() - lastWLPoll >= WLIST_POLLING) {
        lastWLPoll = millis();
        ParseWhitelist();
    }
}