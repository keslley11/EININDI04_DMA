#include <iikitmini.h>         // Biblioteca base do framework Arduino
#include "util/AdcDmaEsp.h"  // Classe para configuração do ADC e DMA
#include "util/jtask.h"      // Biblioteca para gerenciamento de tarefas com timers

AdcDmaESP adcDma; // Instância para gerenciar o ADC e DMA

// Função para leitura do ADC via DMA
void readDMA() {
    if (adcDma.hasData() == ESP_OK) { // Verifica se há novos dados no ADC
        uint16_t* buffer = adcDma.getBuffer(); // Obtém o buffer de dados
        size_t sampleCount = adcDma.getSampleCount(); // Número de amostras disponíveis

        for (size_t i = 0; i < sampleCount; i++) {
            IIKit.WSerial.plot("adcValue", buffer[i]); // Plota os valores do ADC no monitor serial
        }
    }
}

// Função para alternar o estado de um LED
void blinkLEDFunc(uint8_t pin) {
    digitalWrite(pin, !digitalRead(pin)); // Inverte o estado atual do pino especificado
}

// Função para leitura e exibição de valores de entrada analógica
void managerInputFunc() {
    const uint16_t vlPOT1 = IIKit.analogReadPot1();
    const uint16_t vlPOT2 = IIKit.analogReadPot2();
    const uint16_t vlR4a20_1 = IIKit.analogRead4a20_1();
    const uint16_t vlR4a20_2 = IIKit.analogRead4a20_2();

    IIKit.disp.setText(2, ("P1:" + String(vlPOT1) + "  P2:" + String(vlPOT2)).c_str());
    IIKit.disp.setText(3, ("T1:" + String(vlR4a20_1) + "  T2:" + String(vlR4a20_2)).c_str());

    IIKit.WSerial.plot("vlPOT1", vlPOT1);
    IIKit.WSerial.plot("vlPOT2", vlPOT2);
    IIKit.WSerial.plot("vlR4a20_1", vlR4a20_1);
    IIKit.WSerial.plot("vlR4a20_2", vlR4a20_2);
}

// Configuração inicial
void setup() {
    IIKit.setup();
    adcDma.setup(2000, ADC1_CHANNEL, ADC_BITWIDTH_12); // Configura o ADC e DMA (2000 Hz, GPIO39, 12 bits)
    jtaskSetup(); // Configura o timer para 1000 Hz (1 ms)

    jtaskAttachFunc(managerInputFunc, 100); // Executa a leitura e exibição de entradas a cada 100 ms
    jtaskAttachFunc([]() { blinkLEDFunc(def_pin_D1); }, 500); // Alterna o estado do LED a cada 500 ms
    jtaskAttachFunc(readDMA, 1); // Processa os dados do ADC a cada 1 ms
}

// Loop principal
void loop() {
    IIKit.loop();
    jtaskLoop(); // Executa as tarefas enfileiradas
}
