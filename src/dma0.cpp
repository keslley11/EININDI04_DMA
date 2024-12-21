// Este código utiliza o framework Arduino e a biblioteca ESP-IDF para realizar amostragem contínua com ADC no ESP32.
#include <Arduino.h>    // Biblioteca base do framework Arduino, necessária para funções básicas como Serial e delays.
#include <esp_adc/adc_continuous.h> // Biblioteca para modo contínuo com DMA.
#include "util/jtask.h" // Biblioteca para gerenciamento de tarefas com timers.

// Definições para o ADC e DMA.
#define ADC_CHANNEL ADC_CHANNEL_0 // Canal ADC configurado para GPIO36.
#define NUM_SAMPLES 1024          // Número máximo de amostras a serem armazenadas.
#define SAMPLE_RATE 2000          // Taxa de amostragem em Hertz (Hz).

// Handle para o driver do ADC contínuo.
adc_continuous_handle_t adc_handle = NULL; // Ponteiro que gerencia a configuração e operação do ADC.

// Configuração do ADC e DMA.
void setupADC_DMA() {
    // Configuração do driver do ADC contínuo.
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = NUM_SAMPLES * sizeof(uint16_t), // Tamanho do buffer de armazenamento.
        .conv_frame_size = NUM_SAMPLES // Número de amostras por quadro de conversão.
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adc_handle)); // Cria um novo handle para ADC contínuo.

    // Configuração do padrão de conversão do ADC.
    adc_digi_pattern_config_t channel_pattern = {
        .atten = ADC_ATTEN_DB_12, // Atenuação configurada para 12dB (0-3,3V).
        .channel = ADC_CHANNEL,  // Configura o canal do ADC para GPIO36.
        .unit = ADC_UNIT_1,      // Unidade ADC 1.
        .bit_width = ADC_BITWIDTH_12 // Resolução de 12 bits.
    };

    // Configuração do driver ADC para operação contínua.
    adc_continuous_config_t channel_config = {
        .sample_freq_hz = SAMPLE_RATE,         // Taxa de amostragem em Hz.
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,  // Modo de conversão usando uma unidade ADC.
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1 // Formato de saída para os dados do ADC.
    };
    channel_config.pattern_num = 1, // Define o número de padrões de conversão.
    channel_config.adc_pattern = &channel_pattern; // Aponta para o padrão de conversão configurado.

    ESP_ERROR_CHECK(adc_continuous_config(adc_handle, &channel_config)); // Configura o ADC contínuo.
    ESP_ERROR_CHECK(adc_continuous_start(adc_handle)); // Inicia o ADC contínuo.
}

// Variáveis globais para leitura do ADC.
uint32_t bytes_read = 0; // Armazena o número de bytes lidos pelo DMA.
uint16_t adc_buffer[NUM_SAMPLES] = {0}; // Buffer para armazenar os valores do ADC.

// Função para leitura do ADC via DMA.
void readDMA() {
    esp_err_t ret = adc_continuous_read(adc_handle, (uint8_t *) adc_buffer, sizeof(adc_buffer), &bytes_read, 0); // Lê os dados do ADC contínuo.
    if (ret == ESP_OK) { // Verifica se a leitura foi bem-sucedida.
        for (size_t i = 0; i < bytes_read / sizeof(uint16_t); i++) { // Itera sobre as amostras lidas.
            IIKit.WSerial.plot("adcValue", adc_buffer[i]); // Plota os valores do ADC no monitor serial.
        }
    }
}

// Função para alternar o estado de um LED.
void blinkLEDFunc(uint8_t pin) {
    digitalWrite(pin, !digitalRead(pin)); // Inverte o estado atual do pino especificado.
}

// Função para leitura e exibição de valores de entrada analógica.
void managerInputFunc(void) {
    const uint16_t vlPOT1 = IIKit.analogReadPot1(); // Lê o valor do potenciômetro 1.
    const uint16_t vlPOT2 = IIKit.analogReadPot2(); // Lê o valor do potenciômetro 2.
    const uint16_t vlR4a20_1 = IIKit.analogRead4a20_1(); // Lê o valor do canal 4-20mA 1.
    const uint16_t vlR4a20_2 = IIKit.analogRead4a20_2(); // Lê o valor do canal 4-20mA 2.

    IIKit.disp.setText(2, ("P1:" + String(vlPOT1) + "  P2:" + String(vlPOT2)).c_str()); // Atualiza o display com os valores dos potenciômetros.
    IIKit.disp.setText(3, ("T1:" + String(vlR4a20_1) + "  T2:" + String(vlR4a20_2)).c_str()); // Atualiza o display com os valores dos canais 4-20mA.

    IIKit.WSerial.plot("vlPOT1", vlPOT1); // Plota o valor do potenciômetro 1.
    IIKit.WSerial.plot("vlPOT2", vlPOT2); // Plota o valor do potenciômetro 2.
    IIKit.WSerial.plot("vlR4a20_1", vlR4a20_1); // Plota o valor do canal 4-20mA 1.
    IIKit.WSerial.plot("vlR4a20_2", vlR4a20_2); // Plota o valor do canal 4-20mA 2.
}

// Função de configuração do programa.
void setup() {
    Serial.begin(115200); // Inicializa a comunicação serial.
    setupADC_DMA(); // Configura o ADC e o DMA.
    jtaskSetup(1000); // Configura o timer para 1000 Hz (1 ms).
    jtaskAttachFunc(managerInputFunc, 100); // Anexa uma função com base de tempo de 100 ms.
    jtaskAttachFunc([](){blinkLEDFunc(PINLED);}, 500); // Anexa a função de piscar o LED com base de tempo de 500 ms.
    jtaskAttachFunc(readDMA, 1); // Anexa a função de leitura do ADC com base de tempo de 1 ms.
}

// Loop principal do programa.
void loop() {
    jtaskLoop(); // Executa as tarefas enfileiradas.
}