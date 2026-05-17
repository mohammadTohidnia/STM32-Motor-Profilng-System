#ifndef MY_ADC_H
#define MY_ADC_H

#include "stm32f10x.h"

// Initialize a specific ADC (ADC1 or ADC2) for a single channel in continuous mode
// This function is designed to use channels 0 to 7
void ADC_Init(ADC_TypeDef *ADCx, uint8_t channel);

// Read the current 12-bit analog value (0-4095) from the specified ADC
uint16_t ADC_Read(ADC_TypeDef *ADCx);

#endif // MY_ADC_H