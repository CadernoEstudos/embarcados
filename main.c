#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

// Bibliotecas dos sensores (UncleRus e SSD1306)
#include "dht.h"
#include "ssd1306.h"
#include "i2cdev.h" // Necessario para o RTC

// Os componentes
#include "armazenamento.h"
#include "relogio.h"

static const char *TAG = "ESTUFA_MAIN";


// --- CONFIGURAÇÕES DOS PINOS ---
#define CONFIG_DHT_PIN 15
#define CONFIG_LDR_ADC_CHANNEL 4
#define CONFIG_OLED_SDA 8
#define CONFIG_OLED_SCL 9
#define CONFIG_RTC_SDA 16
#define CONFIG_RTC_SCL 17
#define CONFIG_BOTAO_ECRA 0
#define CONFIG_BOTAO_ACAO 14
#define CONFIG_SETPOINT_PADRAO 25
// ----------------------------------


// Variaveis Globais de Estado
int temp_atual = 0;
int umi_atual = 0;
int lum_atual = 0;
int setpoint = CONFIG_SETPOINT_PADRAO;
bool controlo_ligado = true;
bool log_habilitado = false;
int ecra_atual = 0; // 0: Sensores, 1: SetPoint, 2: Controlo, 3: Registo

// Instancias
SSD1306_t oled;
adc_oneshot_unit_handle_t adc1_handle;

// Tarefa 1: Leitura de Sensores e Logica de Controle
void task_sensores(void *pvParameter) {
    char data_hora[30]; 

    while(1) {
        int16_t t=0, h=0;
        
        if(dht_read_data(DHT_TYPE_DHT11, CONFIG_DHT_PIN, &h, &t) == ESP_OK) {
            temp_atual = t / 10;
            umi_atual = h / 10;
        }

        int raw_val = 0;
        adc_oneshot_read(adc1_handle, CONFIG_LDR_ADC_CHANNEL, &raw_val);
        lum_atual = raw_val;

        obter_data_hora(data_hora, sizeof(data_hora));

        if(controlo_ligado) {
            if(temp_atual > setpoint) {
                ESP_LOGW(TAG, "ALERTA: Temperatura alta! Exaustor ON.");
            }
        }

        if(log_habilitado) {
            salvar_log(data_hora, temp_atual, umi_atual, lum_atual);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// Tarefa 2: Ecra OLED e Menu Interativo
void task_menu(void *pvParameter) {
    char linha[32];
    
    gpio_set_direction(CONFIG_BOTAO_ECRA, GPIO_MODE_INPUT);
    gpio_set_pull_mode(CONFIG_BOTAO_ECRA, GPIO_PULLUP_ONLY);
    gpio_set_direction(CONFIG_BOTAO_ACAO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(CONFIG_BOTAO_ACAO, GPIO_PULLUP_ONLY);

    while(1) {
        if(gpio_get_level(CONFIG_BOTAO_ECRA) == 0) {
            ecra_atual++;
            if(ecra_atual > 3) ecra_atual = 0;
            vTaskDelay(pdMS_TO_TICKS(300));
        }

        ssd1306_clear_screen(&oled, false);

        switch(ecra_atual) {
            case 0: 
                sprintf(linha, "Temp: %d C", temp_atual);
                ssd1306_display_text(&oled, 0, linha, strlen(linha), false);
                sprintf(linha, "Humid: %d %%", umi_atual);
                ssd1306_display_text(&oled, 2, linha, strlen(linha), false);
                sprintf(linha, "Lumi: %d", lum_atual);
                ssd1306_display_text(&oled, 4, linha, strlen(linha), false);
                break;
            
            case 1: 
                ssd1306_display_text(&oled, 0, "> AJUSTAR TEMP:", 15, false);
                sprintf(linha, "SET POINT: %d C", setpoint);
                ssd1306_display_text(&oled, 3, linha, strlen(linha), false);
                
                if(gpio_get_level(CONFIG_BOTAO_ACAO) == 0) {
                    setpoint++;
                    if(setpoint > 40) setpoint = 15; 
                    vTaskDelay(pdMS_TO_TICKS(300));
                }
                break;

            case 2: 
                ssd1306_display_text(&oled, 0, "> CONTROLO SIST.", 16, false);
                sprintf(linha, "Estado: %s", controlo_ligado ? "LIGADO  " : "DESLIG. ");
                ssd1306_display_text(&oled, 3, linha, strlen(linha), false);
                
                if(gpio_get_level(CONFIG_BOTAO_ACAO) == 0) {
                    controlo_ligado = !controlo_ligado;
                    vTaskDelay(pdMS_TO_TICKS(300));
                }
                break;

            case 3: 
                ssd1306_display_text(&oled, 0, "> GRAVAR DADOS", 14, false);
                sprintf(linha, "Registo: %s", log_habilitado ? "ATIVO   " : "INATIVO ");
                ssd1306_display_text(&oled, 3, linha, strlen(linha), false);
                
                if(gpio_get_level(CONFIG_BOTAO_ACAO) == 0) {
                    log_habilitado = !log_habilitado;
                    vTaskDelay(pdMS_TO_TICKS(300));
                }
                break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

// Tarefa 3: Comunicacao Serial
void task_serial(void *pvParameter) {
    char comando[20];
    while(1) {
        if (scanf("%19s", comando) == 1) {
            if (strcmp(comando, "lerlog") == 0) {
                ler_log_serial();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "A iniciar Sistema da Estufa LAB01...");

    iniciar_spiffs();
    ESP_ERROR_CHECK(i2cdev_init());
    iniciar_rtc();

    adc_oneshot_unit_init_cfg_t init_config = { .unit_id = ADC_UNIT_1 };
    adc_oneshot_new_unit(&init_config, &adc1_handle);
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(adc1_handle, CONFIG_LDR_ADC_CHANNEL, &config);

    i2c_master_init(&oled, CONFIG_OLED_SDA, CONFIG_OLED_SCL, -1);
    ssd1306_init(&oled, 128, 64);
    ssd1306_clear_screen(&oled, false);
    ssd1306_contrast(&oled, 0xFF);
    ssd1306_display_text_x3(&oled, 0, "ESTUFA", 6, false);
    vTaskDelay(pdMS_TO_TICKS(2000));
    ssd1306_clear_screen(&oled, false);

    xTaskCreate(task_sensores, "TaskSensores", 4096, NULL, 5, NULL);
    xTaskCreate(task_menu, "TaskMenu", 4096, NULL, 5, NULL);
    xTaskCreate(task_serial, "TaskSerial", 4096, NULL, 1, NULL);
}