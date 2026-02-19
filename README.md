# Projeto Estufa LAB01 🌱

Desenvolvimento de um sistema embarcado para monitoramento e controle de uma estufa, utilizando a placa **Franzininho WiFi LAB01** (baseada no ESP32-S2). Este projeto serve como uma Prova de Conceito (PoC) para consolidar conhecimentos em sistemas embarcados, leitura de sensores, interfaces com o usuário e armazenamento de dados.

## 📋 Descrição do Projeto

O sistema monitora variáveis ambientais essenciais de uma estufa (Temperatura, Umidade e Luminosidade) e exibe os dados em tempo real em um display OLED. Através de um menu interativo controlado por botões, o usuário pode:
- Visualizar as leituras dos sensores.
- Ajustar o *Set Point* de temperatura desejada.
- Ligar ou desligar a lógica de controle do sistema (ex: acionamento de um exaustor).
- Habilitar ou desabilitar o registro (log) de dados.

Quando o log está habilitado, o sistema salva periodicamente os dados na memória não volátil (SPIFFS) do ESP32, acompanhados de um carimbo de data e hora (Timestamp) fornecido por um módulo RTC DS3231. Esses dados podem ser extraídos posteriormente via comunicação serial.

### 🛠️ Tecnologias e Componentes Utilizados
- **Microcontrolador:** Franzininho WiFi LAB01 (ESP32-S2)
- **Framework:** ESP-IDF (FreeRTOS)
- **Sensores:** DHT11 (Temperatura e Umidade) e LDR (Luminosidade via ADC)
- **Módulos:** Display OLED SSD1306 (I2C) e Relógio RTC DS3231 (I2C)
- **Bibliotecas Externas:** `esp-idf-lib` (UncleRus)

---

## ⚙️ Instruções para Instalação, Configuração e Uso

### 1. Requisitos de Software
- ESP-IDF v5.0 ou superior instalado.
- Biblioteca [esp-idf-lib](https://github.com/UncleRus/esp-idf-lib) configurada no projeto (via `idf_component.yml`).

### 2. Configuração de Hardware (Pinos Padrão)
As conexões podem ser alteradas diretamente no arquivo `main.c` (ou via `menuconfig` dependendo da versão do projeto). O padrão utilizado é:
- **DHT11 (Dados):** GPIO 15
- **LDR (ADC):** Canal 4 (GPIO 5)
- **OLED (SDA / SCL):** GPIO 8 / GPIO 9
- **RTC DS3231 (SDA / SCL):** GPIO 16 / GPIO 17
- **Botão de Tela (Navegação):** GPIO 0
- **Botão de Ação (Ajustes):** GPIO 14

### 3. Compilação e Gravação
Abra o terminal na pasta raiz do projeto e execute:
```bash
# Compile o projeto
idf.py build

# Grave na placa (substitua a porta COM/ttyUSB correta)
idf.py -p PORTA flash

# Abra o monitor serial para ver os logs e enviar comandos
idf.py -p PORTA monitor
