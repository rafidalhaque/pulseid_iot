#pragma once

#include <Arduino.h>
#include <mbedtls/md.h>
#include <time.h>

String genBLEuuid(const char* master_secret, const char* member_id) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return "Failed to obtain time";
    }
    char date_str[16];
    strftime(date_str, sizeof(date_str), "%Y-%m-%d", &timeinfo);

    String HMAC_message = String(member_id) + String(date_str);

    uint8_t hmac_output[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char*)master_secret, strlen(master_secret));
    mbedtls_md_hmac_update(&ctx, (const unsigned char*)HMAC_message.c_str(), HMAC_message.length());
    mbedtls_md_hmac_finish(&ctx, hmac_output);
    mbedtls_md_free(&ctx);
    char uuid_str[37];
    snprintf(uuid_str, sizeof(uuid_str), "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             hmac_output[0], hmac_output[1], hmac_output[2], hmac_output[3],
             hmac_output[4], hmac_output[5], hmac_output[6], hmac_output[7],
             hmac_output[8], hmac_output[9], hmac_output[10], hmac_output[11],
             hmac_output[12], hmac_output[13], hmac_output[14], hmac_output[15]);
    return String(uuid_str);
}
