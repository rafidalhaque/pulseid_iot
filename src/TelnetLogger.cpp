// TelnetLogger.cpp

#include <WiFi.h>
#include <TelnetLogger.h>
#include <stdarg.h>

WiFiServer telnetServer(23);
WiFiClient telnetClient;

void setupTelnet() {
    telnetServer.begin();
    telnetServer.setNoDelay(true);
}

void loopTelnet() {
    // accept new connection
    if (telnetServer.hasClient()) {
        if (telnetClient) telnetClient.stop();  // kick old client
        telnetClient = telnetServer.available();
    }
}

void logMsg(const String& msg) {
    Serial.println(msg);
    if (telnetClient && telnetClient.connected()) {
        telnetClient.println(msg);
    }
}

void logMsgf(const char* format, ...) {
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    logMsg(String(buf));
}