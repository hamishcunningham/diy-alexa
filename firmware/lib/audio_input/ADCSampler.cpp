#include "ADCSampler.h"
#include "driver/i2s.h"

ADCSampler::ADCSampler(adc1_channel_t adcChannel) : I2SSampler()
{
    m_adcChannel = adcChannel;
}

void ADCSampler::configureI2S()
{
    // Configure ADC1 channel
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(m_adcChannel, ADC_ATTEN_DB_11);

    // Configure I2S for ADC1
    i2s_set_adc_mode(ADC_UNIT_1, m_adcChannel);

    // Enable the ADC
    i2s_adc_enable(getI2SPort());
}

/**
 * Process the raw data that have been read from the I2S peripherals into samples
 **/
void ADCSampler::processI2SData(uint8_t *i2sData, size_t bytesRead)
{
    uint16_t *rawSamples = (uint16_t *)i2sData;
    for (int i = 0; i < bytesRead / 2; i++)
    {
        addSample((2048 - (rawSamples[i] & 0xfff)) * 15);
    }
}
