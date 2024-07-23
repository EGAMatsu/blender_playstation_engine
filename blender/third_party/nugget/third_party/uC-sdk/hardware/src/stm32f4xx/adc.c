#include "adc.h"

#include <stm32f4xx.h>
#include <stm32f4xx_adc.h>

#include <hardware.h>

static ADC_TypeDef *adclist[] = {ADC1, ADC2, ADC3};

void adc_calibrate(uint8_t adc)
{
}

//global configuration for all DMAs
//let's keep it like this for now
void adc_config_all()
{
    ADC_CommonInitTypeDef commondef;
    commondef.ADC_Mode = ADC_Mode_Independent;				//Each ADC is independent of others
    commondef.ADC_Prescaler = ADC_Prescaler_Div8;
    commondef.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;		//NO DMA, ADC_DMAAccessMode_1 if we want a sequence of 16b values
    commondef.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;	//smallest delay
    ADC_CommonInit(&commondef);
}

void adc_config_single(adc_t adc, uint8_t channel, pin_t pin)
{
    if (adc < adc_1 || adc > adc_3 || channel > 18)
        return;

    gpio_config_analog(pin, pin_dir_read, pull_none);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 << (adc - 1), ENABLE);

    ADC_InitTypeDef def;
    ADC_StructInit(&def);
    def.ADC_DataAlign = ADC_DataAlign_Right;
    def.ADC_Resolution = ADC_Resolution_12b;
    def.ADC_ContinuousConvMode = DISABLE;
    def.ADC_ExternalTrigConv = 0;
    def.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    def.ADC_NbrOfConversion = 1;
    def.ADC_ScanConvMode = ENABLE;
    ADC_Init(adclist[adc - 1], &def);

    ADC_RegularChannelConfig(adclist[adc - 1], channel, 1, ADC_SampleTime_144Cycles);

    ADC_Cmd(adclist[adc - 1],ENABLE);
}

void adc_config_continuous(adc_t adc, uint8_t *channel, pin_t *pin, uint16_t *dest, uint8_t nb)
{
    if (adc < adc_1 || adc > adc_3)
        return;
    int i;
    for (i = 0 ; i < nb ; i++)
        if (channel[i] > 18)
            return;

    for (i = 0 ; i < nb ; i++)
        gpio_config_analog(pin[i], pin_dir_read, pull_none);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);

    /*
        DMA2
        ADC1: Channel 0, Stream 0 or Stream 4
        ADC2: Channel 1, Stream 2 or Stream 3
        ADC3: Channel 2, Stream 0 or Stream 1
    */
    DMA_Stream_TypeDef *dmastreams[] = {DMA2_Stream4, DMA2_Stream2, DMA2_Stream1};
    uint32_t dmachannels[] = {DMA_Channel_0, DMA_Channel_1, DMA_Channel_2};

    DMA_InitTypeDef dmadef;
    DMA_StructInit(&dmadef);
    DMA_DeInit(dmastreams[adc]);
    dmadef.DMA_Channel = dmachannels[adc - 1];
    dmadef.DMA_PeripheralBaseAddr = (uint32_t)&(adclist[adc - 1])->DR;
    dmadef.DMA_Memory0BaseAddr = (uint32_t) &dest[0];
    dmadef.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dmadef.DMA_BufferSize = nb;
    dmadef.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dmadef.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dmadef.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dmadef.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dmadef.DMA_Mode = DMA_Mode_Circular;
    dmadef.DMA_Priority = DMA_Priority_High;
    dmadef.DMA_FIFOMode = DMA_FIFOMode_Disable;
    dmadef.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    dmadef.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dmadef.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(dmastreams[adc - 1], &dmadef);
    DMA_Cmd(dmastreams[adc - 1], ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 << (adc - 1), ENABLE);

    ADC_CommonInitTypeDef commondef;
    commondef.ADC_Mode = ADC_Mode_Independent;
    commondef.ADC_Prescaler = ADC_Prescaler_Div2;
    commondef.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    commondef.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&commondef);

    ADC_InitTypeDef def;
    ADC_StructInit(&def);
    def.ADC_DataAlign = ADC_DataAlign_Right;
    def.ADC_Resolution = ADC_Resolution_12b;
    def.ADC_ContinuousConvMode = ENABLE;
    //def.ADC_ExternalTrigConv = 0;
    def.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    def.ADC_NbrOfConversion = nb;
    def.ADC_ScanConvMode = ENABLE;
    ADC_Init(adclist[adc - 1], &def);

    for (i = 0 ; i < nb ; i++)
        ADC_RegularChannelConfig(adclist[adc - 1], channel[i], (i + 1), ADC_SampleTime_144Cycles);

    ADC_DMARequestAfterLastTransferCmd(adclist[adc - 1], ENABLE);

    ADC_DMACmd(adclist[adc - 1], ENABLE);

    ADC_Cmd(adclist[adc - 1], ENABLE);

    ADC_SoftwareStartConv(adclist[adc - 1]);
}

uint16_t adc_get(adc_t adc)
{
    if (adc < adc_1 || adc > adc_3)
        return 0;

    ADC_SoftwareStartConv(adclist[adc - 1]);
    while(!ADC_GetFlagStatus(adclist[adc - 1], ADC_FLAG_EOC)){}
    uint16_t res = ADC_GetConversionValue(adclist[adc - 1]);
    ADC_ClearFlag(adclist[adc - 1], ADC_FLAG_EOC);

    return res;
}
