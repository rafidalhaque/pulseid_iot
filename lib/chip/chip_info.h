#pragma once
#include <WiFi.h>
#include <esp_chip_info.h>
#include <esp_system.h>

#define RELAY_PIN 5  // D5
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

void chipInfo() {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    Serial.println("==================== ESP32 System Info ================================");
    Serial.print("Chip Model: ");
    switch (chip_info.model) {
        case CHIP_ESP32:
            Serial.println("ESP32");
            break;
        case CHIP_ESP32S2:
            Serial.println("ESP32-S2");
            break;
        case CHIP_ESP32S3:
            Serial.println("ESP32-S3");
            break;
        case CHIP_ESP32C3:
            Serial.println("ESP32-C3");
            break;
        case CHIP_ESP32H2:
            Serial.println("ESP32-H2");
            break;
        default:
            Serial.println("Chip model UNKNOWN");
            digitalWrite(RELAY_PIN, RELAY_ON);
            digitalWrite(RELAY_PIN, RELAY_OFF);
            break;
    }
    Serial.print("CPU Cores: ");
    Serial.println(chip_info.cores);
    Serial.print("Chip Revision: ");
    Serial.println(chip_info.revision);
    Serial.print("Features: ");
    Serial.println((chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi" : "No WiFi");
    Serial.println((chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "No BLE");
    Serial.println((chip_info.features & CHIP_FEATURE_BT) ? "Classic Bluetooth" : "No Classic Bluetooth");
    Serial.println((chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "Embedded Flash" : "No Embedded Flash");
    Serial.print("Flash Size: ");
    Serial.print(spi_flash_get_chip_size() / (1024 * 1024));
    Serial.println(" MB");
    Serial.print("Free Heap Size: ");
    Serial.print(esp_get_free_heap_size() / 1024);
    Serial.println(" KB");
    Serial.print("Min Free Heap Size:");
    Serial.print(esp_get_minimum_free_heap_size() / 1024);
    Serial.println(" KB");
    // Serial.print("WiFi MAC Address: ");
    // Serial.println(WiFi.macAddress());
    uint8_t bt_mac[6];
    esp_read_mac(bt_mac, ESP_MAC_BT);
    Serial.printf("BT MAC:       %02X:%02X:%02X:%02X:%02X:%02X\n",
        bt_mac[0], bt_mac[1], bt_mac[2],
        bt_mac[3], bt_mac[4], bt_mac[5]);

    // --- SDK / IDF Version ---
    Serial.print("IDF version:  ");
    Serial.println(esp_get_idf_version());

    Serial.println("===============================");

}