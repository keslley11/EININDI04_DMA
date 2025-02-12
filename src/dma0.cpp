#include <iikitmini.h> // Biblioteca base do framework Arduino
#include <driver/i2s.h>


#ifndef DMA_BUFFERS
  #define DMA_BUFFERS 4
#endif

#ifndef BUFFER_LEN
  #define BUFFER_LEN 64
#endif

typedef void (*ADCCallback)(const int16_t *data, size_t count);
ADCCallback _adcDmaEspcallback;
__attribute__((aligned(16))) int16_t dma_buffer[DMA_BUFFERS * BUFFER_LEN];
uint32_t _plotInterval = 0;
uint32_t _last_plot = 0;

void adcDmaSetup(adc1_channel_t channel, ADCCallback callback, adc_bits_width_t width_bit = ADC_WIDTH_BIT_12, uint32_t sample_rate = 1000, uint32_t plotInterval = 100)
{
  _adcDmaEspcallback = callback;
  _plotInterval = plotInterval;
  adc_power_acquire();
  adc1_config_width(width_bit);
  adc1_config_channel_atten(channel, ADC_ATTEN_DB_12);

  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
      .sample_rate = sample_rate,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = DMA_BUFFERS,
      .dma_buf_len = BUFFER_LEN,
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0,
      .mclk_multiple = I2S_MCLK_MULTIPLE_256};

  if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) != ESP_OK)
  {
    Serial.println("ERR:I2S_INIT");
    while (1)
      ;
  }

  i2s_set_adc_mode(ADC_UNIT_1, channel);
  i2s_adc_enable(I2S_NUM_0);
}

void adcDmaLoop()
{
  size_t bytes_read;
  esp_err_t err = i2s_read(I2S_NUM_0, dma_buffer, sizeof(dma_buffer), &bytes_read, 0);
  if (err == ESP_OK)
  {
    // Envia todos os dados de uma vez para otimização
    if (millis() - _last_plot >= _plotInterval)
    {
      _adcDmaEspcallback(dma_buffer, (uint16_t)(bytes_read / sizeof(int16_t)));
      _last_plot = millis();
    }
  }
}

// Função para leitura do ADC via DMA
void readDMA(const int16_t *y, size_t ylen)
{
  IIKit.WSerial.plot("adcValue", (uint32_t)1, y, ylen); // Plota os valores do ADC no monitor serial
}

void setup()
{
  IIKit.setup();
  adcDmaSetup(ADC1_CHANNEL_0, readDMA, ADC_WIDTH_BIT_12);
}

void loop()
{
  IIKit.loop();
  adcDmaLoop();
}