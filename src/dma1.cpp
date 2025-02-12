#include <iikitmini.h>      // Biblioteca base do framework Arduino
#include "util/AdcDmaEsp.h" // Classe para configuração do ADC e DMA
#include "util/jtask.h"

uint32_t currentSample = 0;      // Índice da amostra atual
#define RESOLUTION 256           // Número de pontos na senoide (quanto maior, mais suave)
#define MEIARESOLUTION 127.5

void updateDAC() {
  dacWrite(def_pin_DAC1, uint8_t((MEIARESOLUTION * sin((2 * PI * currentSample) / RESOLUTION)) + MEIARESOLUTION)); // Escreve o valor no DAC
  currentSample = (currentSample + 1) % RESOLUTION;
}

void setup()
{
  IIKit.setup();
  adcDmaSetup(CHANNEL_ADC1,[](const int16_t *y, size_t ylen){IIKit.WSerial.plot("adcValue", (uint32_t) 1000, y, ylen);}, ADC_WIDTH_BIT_12);
  
  // Calcula o tempo entre amostras com base na frequência inicial
  unsigned long frequency = 10UL;                    // Frequência inicial da senoide em Hz  
  unsigned long sampleRate = frequency * RESOLUTION; // Número de amostras por segundo que queremos gerar.
  unsigned long samplePeriod = 1000000UL / sampleRate; // Tempo entre uma amostra e a próxima, em microssegundos (µs). OBS.: 1000000: Número de microssegundos em um segundo (1 segundo = 1.000.000 µs).
  jtaskAttachFunc(updateDAC, samplePeriod);  
}

void loop()
{
  IIKit.loop();
  adcDmaLoop();
  jtaskLoop();  
}
