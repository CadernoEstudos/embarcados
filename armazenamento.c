#include <stdio.h>
#include <string.h>
#include "esp_spiffs.h"
#include "esp_log.h"
#include "armazenamento.h"

static const char *TAG = "SPIFFS";

void iniciar_spiffs(void) {
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };
    if (esp_vfs_spiffs_register(&conf) != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao montar SPIFFS");
    } else {
        ESP_LOGI(TAG, "SPIFFS montado!");
    }
}

void salvar_log(const char* data_hora, int temp, int umi, int lum) {
    FILE* f = fopen("/spiffs/log.txt", "a");
    if (f == NULL) return;
    fprintf(f, "[%s] T:%dC U:%d%% L:%d\n", data_hora, temp, umi, lum);
    fclose(f);
}

void ler_log_serial(void) {
    FILE* f = fopen("/spiffs/log.txt", "r");
    if (f == NULL) return;
    char linha[64];
    printf("\n--- LOG DA ESTUFA ---\n");
    while (fgets(linha, sizeof(linha), f) != NULL) printf("%s", linha);
    printf("--- FIM ---\n\n");
    fclose(f);
}