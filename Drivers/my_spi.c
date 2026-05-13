#include "my_spi.h"

void SPI_Master_Init(SPI_TypeDef *SPIx, uint16_t prescaler_div) {
    // Enable Clocks and Configure Pins using your custom GPIO Driver
    if (SPIx == SPI1) {
        // SPI1 is on APB2
        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
        RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; // Enable Alternate Functions
        
        GPIO_EnableClock(GPIOA);

        // PA5 (SCK) and PA7 (MOSI) as Alternate Function Push-Pull
        GPIO_InitPin(GPIOA, 5, GPIO_MODE_AF_PP);
        GPIO_InitPin(GPIOA, 7, GPIO_MODE_AF_PP);

        // PA6 (MISO) as Floating Input
        GPIO_InitPin(GPIOA, 6, GPIO_MODE_INPUT_FLOATING);
    } 
    else if (SPIx == SPI2) {
        // SPI2 is on APB1
        RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
        RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; 

        GPIO_EnableClock(GPIOB);

        // PB13 (SCK) and PB15 (MOSI) as Alternate Function Push-Pull
        GPIO_InitPin(GPIOB, 13, GPIO_MODE_AF_PP);
        GPIO_InitPin(GPIOB, 15, GPIO_MODE_AF_PP);

        // PB14 (MISO) as Floating Input
        GPIO_InitPin(GPIOB, 14, GPIO_MODE_INPUT_FLOATING);
    }

    // Configure the Control Register (CR1)
    SPIx->CR1 = 0x0000; // Ensure SPI is disabled before configuring

    // Apply my custom definitions: Master Mode + SSM + SSI
    SPIx->CR1 |= MY_SPI_CR1_MSTR;
    SPIx->CR1 |= MY_SPI_CR1_SSM | MY_SPI_CR1_SSI;

    // Set Baud Rate Prescaler (Bits 5:3)
    uint16_t br_bits = 0x03; // Default divide by 16
    if (prescaler_div == 2)        br_bits = 0x00;
    else if (prescaler_div == 4)   br_bits = 0x01;
    else if (prescaler_div == 8)   br_bits = 0x02;
    else if (prescaler_div == 16)  br_bits = 0x03;
    else if (prescaler_div == 32)  br_bits = 0x04;
    else if (prescaler_div == 64)  br_bits = 0x05;
    else if (prescaler_div == 128) br_bits = 0x06;
    else if (prescaler_div == 256) br_bits = 0x07;
    
    SPIx->CR1 |= (br_bits << 3);

    // Enable the SPI Peripheral using your custom definition
    SPIx->CR1 |= MY_SPI_CR1_SPE;
}

uint8_t SPI_Transfer(SPI_TypeDef *SPIx, uint8_t data) {
    // Wait until the Transmit Buffer is Empty (TXE flag)
    while (!(SPIx->SR & MY_SPI_SR_TXE));

    // Write data to the Data Register
    SPIx->DR = data;

    // Wait until the Receive Buffer is Not Empty (RXNE flag)
    while (!(SPIx->SR & MY_SPI_SR_RXNE));

    // Wait until SPI is no longer busy (BSY flag)
    while (SPIx->SR & MY_SPI_SR_BSY);

    // Read and return the received data
    return (uint8_t)SPIx->DR;
}

// Helper function to pull the CS pin LOW (SPI line Active)
void SPI_CS_Enable(GPIO_TypeDef *GPIOx, uint8_t pin) {
    
    GPIO_WritePin(GPIOx, pin, 0); 
}

// Helper function to pull the CS pin HIGH (SPI line Inactive)
void SPI_CS_Disable(GPIO_TypeDef *GPIOx, uint8_t pin) {
    GPIO_WritePin(GPIOx, pin, 1);
}