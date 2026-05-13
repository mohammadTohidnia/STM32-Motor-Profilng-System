#ifndef MY_SPI_H
#define MY_SPI_H

#include "stm32f10x.h"
#include "my_gpio.h" 

// --- Custom SPI_CR1 Definitions ---
#define MY_SPI_CR1_MSTR  (1 << 2)  // Bit 2: Master configuration
#define MY_SPI_CR1_SPE   (1 << 6)  // Bit 6: SPI Enable
#define MY_SPI_CR1_SSI   (1 << 8)  // Bit 8: Internal slave select
#define MY_SPI_CR1_SSM   (1 << 9)  // Bit 9: Software slave management

// --- Custom SPI_SR Definitions ---
#define MY_SPI_SR_RXNE   (1 << 0)  // Bit 0: Receive buffer not empty
#define MY_SPI_SR_TXE    (1 << 1)  // Bit 1: Transmit buffer empty
#define MY_SPI_SR_BSY    (1 << 7)  // Bit 7: Busy flag

// Initialize the SPI Peripheral as Master
void SPI_Master_Init(SPI_TypeDef *SPIx, uint16_t prescaler_div);

// Transmit and Receive a single byte over SPI
uint8_t SPI_Transfer(SPI_TypeDef *SPIx, uint8_t data);

// Software NSS Control (Chip Select)
void SPI_CS_Enable(GPIO_TypeDef *GPIOx, uint8_t pin);
void SPI_CS_Disable(GPIO_TypeDef *GPIOx, uint8_t pin);

#endif // MY_SPI_H