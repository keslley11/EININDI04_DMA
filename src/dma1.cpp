#include <iikitmini.h>      // Biblioteca base do framework Arduino
#include "util/AdcDmaEsp.h" // Classe para configuração do ADC e DMA

void setup()
{
  IIKit.setup();
  adcDmaSetup(ADC1_CHANNEL_3,[](const int16_t *y, size_t ylen){IIKit.WSerial.plot("adcValue", (uint32_t) 1000, y, ylen);}, ADC_WIDTH_BIT_12);
}

void loop()
{
  IIKit.loop();
  adcDmaLoop();
}
