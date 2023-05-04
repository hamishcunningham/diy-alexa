#ifndef __adc_sampler_h__
#define __adc_sampler_h__

#include "I2SSampler.h"
#include "driver/adc.h"

class ADCSampler : public I2SSampler
{
private:
    adc1_channel_t m_adcChannel;

protected:
    void configureI2S();
    void processI2SData(uint8_t *i2sData, size_t bytesRead);

public:
    ADCSampler(adc1_channel_t adc_channel);
};

#endif
