#include <iikitmini.h>      // Biblioteca base do framework Arduino
#include "util/AdcDmaEsp.h" // Classe para configuração do ADC e DMA
#include "util/jtask.h"     // Biblioteca para gerenciamento de tarefas com timers

AdcDmaEsp adcDma; // Instância para gerenciar o ADC e DMA
uint32_t timeCount = 0;

// Função para leitura do ADC via DMA
void readDMA(const uint16_t *buffer, size_t sampleCount)
{
    for (size_t i = 0; i < sampleCount; i++)
    {
        IIKit.WSerial.plot("adcValue",timeCount++, abs(buffer[i]) & 0x0FFF); // Plota os valores do ADC no monitor serial
    }
}

// Configuração inicial
void setup()
{
    IIKit.setup();
    jtaskSetup();       
    if(!adcDma.begin()) {
        IIKit.WSerial.println("Failed to initialize ADC!");
        while(1);
    }
    adcDma.setCallback(readDMA);                                // Configura o timer para 1000 Hz (1 ms)
    jtaskAttachFunc([](){adcDma.update();}, 100);               // Processa os dados do ADC a cada 1 ms
}

// Loop principal
void loop()
{
    IIKit.loop();
    jtaskLoop(); // Executa as tarefas enfileiradas
}
