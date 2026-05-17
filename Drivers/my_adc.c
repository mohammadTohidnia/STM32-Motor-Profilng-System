#include "my_adc.h"
#include "my_gpio.h"
#include "my_timer.h"

void ADC_Init(ADC_TypeDef *ADCx, uint8_t channel) {
    // Set ADC Clock Prescaler to divide by 6 (72MHz / 6 = 12MHz). We can't exceed 14 MHz clk for ADC
    // The ADCPRE bits control the clock for ALL ADCs, so we set it universally
    RCC->CFGR &= ~(3 << 14);  // First clear
    RCC->CFGR |=  (2 << 14);  // Then set

    // Enable Clocks for the specific ADC and GPIOA
    if (ADCx == ADC1) {
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    } 
    else if (ADCx == ADC2) {
        RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;
    }									
    
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // Enable Port A for the analog pins

    // Configure the correct GPIO Pin as Analog Input 
    // (Assuming Channels 0-7 live on Port A)
    if (channel <= 7) {
        GPIO_InitPin(GPIOA, channel, GPIO_MODE_INPUT_ANALOG);
    }

    // Wake up the specific ADC from power-down mode
    ADCx->CR2 |= (1 << 0); // Set ADON bit

    // Delay for Tstab (Stabilization time)
    delay_ms(1);
		
		
    // Run Hardware Calibration
    ADCx->CR2 |= (1 << 2); // Set CAL bit
    while (ADCx->CR2 & (1 << 2)); // Wait in loop until hardware clears the CAL bit

    // Configure Sequence
    // SQR1: Total length of sequence. Default is 0x00000000 = 1 conversion
    ADCx->SQR1 = 0x00000000;
    
    // SQR3: Set the first conversion in the sequence to our requested channel
    ADCx->SQR3 = channel;

    // Configure Sample Time
    // We use the maximum sample time (239.5 cycles) for the most stable reading
    if (channel <= 9) {
        ADCx->SMPR2 |= (7 << (channel * 3));
    }

    // Enable Continuous Conversion Mode
    ADCx->CR2 |= (1 << 1); // Set CONT bit

    // Start the continuous conversions
    ADCx->CR2 |= (1 << 0); // Set ADON bit a second time to start
}

uint16_t ADC_Read(ADC_TypeDef *ADCx) {
    // Return the latest value from the specific ADC's Data Register
    return (uint16_t)(ADCx->DR & 0x0FFF);
}