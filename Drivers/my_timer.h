#ifndef MY_TIMER_H
#define MY_TIMER_H

#include "stm32f10x.h"

// Initialize a specific timer for PWM at a given frequency
void PWM_Init(TIM_TypeDef *TIMx, uint8_t channel, uint32_t frequency_hz);

// Set the duty cycle (0.0 to 100.0 percent)
void PWM_SetDutyCycle(TIM_TypeDef *TIMx, uint8_t channel, float duty_cycle_percent);

// Start the timer
void PWM_Start(TIM_TypeDef *TIMx);

// Initialize a specific timer channel for Input Capture
// We set prescaler to 71 which means that timer works in 1MHz clock (1 tick == 1 microseconds)
void InputCapture_Init(TIM_TypeDef *TIMx, uint8_t channel);

// Read the captured timestamp value
uint16_t InputCapture_Read(TIM_TypeDef *TIMx, uint8_t channel);

// Initialize the SysTick timer
void SysTick_Init(void);

// Microsecond delay (Max value: ~233,000 us due to 24-bit limit)
void delay_us(uint32_t us);

// Millisecond delay (No maximum limit because we create ms delay with delay_us())
void delay_ms(uint32_t ms);

#endif 