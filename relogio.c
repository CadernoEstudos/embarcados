#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "ds3231.h"
#include "relogio.h"

static i2c_dev_t rtc_dispositivo;

void iniciar_rtc(void) {
    memset(&rtc_dispositivo, 0, sizeof(i2c_dev_t));
    ds3231_init_desc(&rtc_dispositivo, 0, CONFIG_RTC_SDA, CONFIG_RTC_SCL);
}

void obter_data_hora(char* buffer, int tamanho_maximo) {
    struct tm tempo_atual;
    if (ds3231_get_time(&rtc_dispositivo, &tempo_atual) == ESP_OK) {
        snprintf(buffer, tamanho_maximo, "%02d/%02d/%04d %02d:%02d:%02d",
                 tempo_atual.tm_mday, tempo_atual.tm_mon + 1, tempo_atual.tm_year + 1900,
                 tempo_atual.tm_hour, tempo_atual.tm_min, tempo_atual.tm_sec);
    } else {
        snprintf(buffer, tamanho_maximo, "ERRO_RTC");
    }
}