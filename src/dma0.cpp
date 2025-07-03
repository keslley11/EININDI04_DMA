#include <iikitmini.h>      // Biblioteca base do framework Arduino
#include "util/AdcDmaEsp.h" // Classe para configuração do ADC e DMA

void osciloscope(const int16_t *y, size_t ylen)
{
   IIKit.WSerial.plot("adcValue", (uint32_t)1000, y, ylen);
} 

void setup()
{
    adcDmaSetup(ADC1_CHANNEL_0, osciloscope);
}

void loop()
{
    IIKit.loop();
    adcDmaLoop();
}
