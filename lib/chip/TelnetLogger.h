#pragma once
#include <Arduino.h>

void setupTelnet();
void loopTelnet();

void logMsg(const String& msg);
void logMsgf(const char* format, ...);