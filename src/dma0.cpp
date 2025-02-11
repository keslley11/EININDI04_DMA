#include <iikitmini.h>      // Biblioteca base do framework Arduino
#include <driver/i2s.h>

#define ADC_CHANNEL ADC1_CHANNEL_0  // GPIO36
#define SAMPLE_RATE 1000
#define DMA_BUFFERS 4
#define BUFFER_LEN 64
#define PLOT_INTERVAL 100  // Intervalo de plotagem em ms

__attribute__((aligned(16))) int16_t dma_buffer[DMA_BUFFERS * BUFFER_LEN];

void setup() {
  IIKit.setup();
  
  adc_power_acquire();
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_12);

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = DMA_BUFFERS,
    .dma_buf_len = BUFFER_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0,
    .mclk_multiple = I2S_MCLK_MULTIPLE_256
  };

  if(i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) != ESP_OK){
    Serial.println("ERR:I2S_INIT");
    while(1);
  }

  i2s_set_adc_mode(ADC_UNIT_1, ADC_CHANNEL);
  i2s_adc_enable(I2S_NUM_0);
}

void loop() {
  IIKit.loop();    
  static uint32_t last_plot = 0;
  size_t bytes_read;
  esp_err_t err = i2s_read(I2S_NUM_0, dma_buffer, sizeof(dma_buffer), &bytes_read, 0);

  if(err == ESP_OK) {
    uint16_t samples = bytes_read / sizeof(int16_t);
    
    // Envia todos os dados de uma vez para otimização
    if(millis() - last_plot >= PLOT_INTERVAL) {
      Serial.print(">adc:[");
      for(int i = 0; i < samples; i++) {
        uint16_t value = abs(dma_buffer[i]) & 0x0FFF;
        Serial.print(value);
        if(i < samples-1) Serial.print(",");
      }
      Serial.println("]|");
      last_plot = millis();
    }
  }
}