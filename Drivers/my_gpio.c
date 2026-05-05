#include "my_gpio.h"

// Enable the Peripheral Clock via the RCC register
void GPIO_EnableClock(GPIO_TypeDef *GPIOx) {
    if (GPIOx == GPIOA) {
        RCC->APB2ENR |= CLOCK_ENABLE_PORTA;
    } else if (GPIOx == GPIOB) {
        RCC->APB2ENR |= CLOCK_ENABLE_PORTB;
    } else if (GPIOx == GPIOC) {
        RCC->APB2ENR |= CLOCK_ENABLE_PORTC;
    } else if (GPIOx == GPIOD) {
        RCC->APB2ENR |= CLOCK_ENABLE_PORTD;
    } else if (GPIOx == GPIOE) {
        RCC->APB2ENR |= CLOCK_ENABLE_PORTE;
    }
}

// Initialize the Pin (Handling CRL for pins 0-7, CRH for pins 8-15)
void GPIO_InitPin(GPIO_TypeDef *GPIOx, uint8_t pin, GPIO_Mode_t mode) {
    
    uint8_t pinPos;
    
    if (pin < 8) {
        // Pins 0-7 use Configuration Register Low (CRL)
        pinPos = pin * 4;
        
        // Clear the 4 bits for this pin first
        GPIOx->CRL &= ~(0xF << pinPos);
        // Set the new mode
        GPIOx->CRL |= (mode << pinPos);
        
    } else {
        // Pins 8-15 use Configuration Register High (CRH)
        pinPos = (pin - 8) * 4;
        
        // Clear the 4 bits
        GPIOx->CRH &= ~(0xF << pinPos);
        // Set the new mode
        GPIOx->CRH |= (mode << pinPos);
    }
}

// Write HIGH or LOW to the pin
void GPIO_WritePin(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t state) {
    if (state) {
        // BSRR - Lower 16 bits set the pin to High
        GPIOx->BSRR = (1 << pin);
    } else {
        // BSRR - Higher 16 bits set the pin to LOW
        GPIOx->BSRR = (1 << (pin + 16));
  
    }
}

// 4. Toggle the pin state
void GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint8_t pin) {
    // ODR (Output Data Register) holds the current state. XOR it with 1 to toggle.
    GPIOx->ODR ^= (1 << pin);
}

// Helper function to map the GPIO Port to the AFIO numbering system
// Inside the manual, the number related to each port is the number of that port oredr (For example 0 for PORTA, 1 for PORTB, ...)
static uint8_t GPIO_GetPortSource(GPIO_TypeDef *GPIOx) {
    if (GPIOx == GPIOA) return 0;
    if (GPIOx == GPIOB) return 1;
    if (GPIOx == GPIOC) return 2;
    if (GPIOx == GPIOD) return 3;
    if (GPIOx == GPIOE) return 4;
    return 0;
}

// Configure the GPIO pin to act as an external interrupt
void GPIO_ConfigInterrupt(GPIO_TypeDef *GPIOx, uint8_t pin, EXTI_Trigger_t edge) {
    // 1. Enable the Alternate Function I/O (AFIO) clock 
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    // Map the Port to the EXTI line using the EXTICR registers.
    // There are 4 registers (EXTICR[0] to [3]), each holding 4 pins
    uint8_t register_index = pin / 4;      // Divides by 4 to find register 0, 1, 2, or 3
    uint8_t bit_position = (pin % 4) * 4;  // Finds the starting bit (0, 4, 8, or 12)
    uint8_t port_source = GPIO_GetPortSource(GPIOx);

    // Clear the 4 bits first, then set the new port source 
    AFIO->EXTICR[register_index] &= ~(0xF << bit_position);
    AFIO->EXTICR[register_index] |= (port_source << bit_position);

    // Configure the trigger edge (Rising, Falling, or Both) 
    // Clear both edges first to be safe
    EXTI->RTSR &= ~(1 << pin);
    EXTI->FTSR &= ~(1 << pin);

    if (edge == EXTI_TRIGGER_RISING || edge == EXTI_TRIGGER_RISING_FALLING) {
        EXTI->RTSR |= (1 << pin); // Set Rising Edge register 
    }
    if (edge == EXTI_TRIGGER_FALLING || edge == EXTI_TRIGGER_RISING_FALLING) {
        EXTI->FTSR |= (1 << pin); // Set Falling Edge register 
    }
}

// Unmask the line and enable it in the core NVIC
// The lower the number, the higher the priority
void GPIO_EnableInterrupt(uint8_t pin, uint32_t priority) {
    // Unmask the interrupt on the EXTI line 
    EXTI->IMR |= (1 << pin);

    // Enable the specific line in the Cortex-M3 NVIC 
    IRQn_Type irq_number;
    
    // Pins 10-15 share a single interrupt vector 
    if (pin >= 10) {
        irq_number = EXTI15_10_IRQn;
    } 
    // Pins 5-9 share a single interrupt vector 
    else if (pin >= 5) {
        irq_number = EXTI9_5_IRQn;
    } 
    // Pins 0-4 have dedicated vectors 
    else {
        // EXTI0_IRQn is mapped to 6, EXTI1 to 7, etc. We can just add the pin number.
        irq_number = (IRQn_Type)(EXTI0_IRQn + pin); 
    }
		
    // Use the core CMSIS functions to set priority and enable
    NVIC_SetPriority(irq_number, priority);
    NVIC_EnableIRQ(irq_number);
}

// Clear the pending flag (MUST be called inside the IRQ Handler)
void GPIO_ClearInterruptFlag(uint8_t pin) {
    // To clear the pending register, hardware requires writing a 1
    if (EXTI->PR & (1 << pin)) {
        EXTI->PR |= (1 << pin); 
    }
}


