#ifndef GPIO_H
#define GPIO_H

#include "stm32f10x.h" 

// Define GPIO Modes. We define the possible modes of GPIO (According to the manual)
typedef enum {
    GPIO_MODE_INPUT_ANALOG      = 0x00,
    GPIO_MODE_INPUT_FLOATING    = 0x04, // 01 00
    GPIO_MODE_INPUT_PULL        = 0x08, // 10 00
    
    // Output modes (50MHz speed chosen for general use)
    GPIO_MODE_OUTPUT_PP         = 0x03, // Push-Pull
    GPIO_MODE_OUTPUT_OD         = 0x07, // Open-Drain
    GPIO_MODE_AF_PP             = 0x0B, // Alternate Function Push-Pull
    GPIO_MODE_AF_OD             = 0x0F  // Alternate Function Open-Drain
} GPIO_Mode_t;

// Define Interrupt Trigger Edges (According to user manual)
typedef enum {
    EXTI_TRIGGER_RISING         = 0x01,
    EXTI_TRIGGER_FALLING        = 0x02,
    EXTI_TRIGGER_RISING_FALLING = 0x03
} EXTI_Trigger_t;


// Custom macros for GPIO Clock Enable
#define CLOCK_ENABLE_PORTA  RCC_APB2ENR_IOPAEN
#define CLOCK_ENABLE_PORTB  RCC_APB2ENR_IOPBEN
#define CLOCK_ENABLE_PORTC  RCC_APB2ENR_IOPCEN
#define CLOCK_ENABLE_PORTD  RCC_APB2ENR_IOPDEN
#define CLOCK_ENABLE_PORTE  RCC_APB2ENR_IOPEEN

// GPIO Functions
void GPIO_EnableClock(GPIO_TypeDef *GPIOx);
void GPIO_InitPin(GPIO_TypeDef *GPIOx, uint8_t pin, GPIO_Mode_t mode);
void GPIO_WritePin(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t state);
void GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint8_t pin);

// External Interrupt Functions
void GPIO_ConfigInterrupt(GPIO_TypeDef *GPIOx, uint8_t pin, EXTI_Trigger_t edge);
void GPIO_EnableInterrupt(uint8_t pin, uint32_t priority);
void GPIO_ClearInterruptFlag(uint8_t pin);

#endif // GPIO_H