
/*
#include <stdio.h>
#include "stm32f10x.h"
#include "my_gpio.h"
#include "my_rcc.h"
#include "my_timer.h"
#include "my_spi.h"
#include <stdbool.h>





int main(void) {
		
	// Set the maximum clk for peripherals
	SystemClock_Config_72MHz();
	// Initialize Systick to processer clock (To use delay functions)
	SysTick_Init();
	
	GPIO_EnableClock(GPIOC);
	GPIO_InitPin(GPIOC, 13, GPIO_MODE_OUTPUT_PP);
	GPIO_WritePin(GPIOC, 13, 1);
	SPI_Master_Init(SPI1,16);

	uint8_t data = 0x88;
	bool is_sent = false;

	while(1) {
			// Toggle the LED
			uint8_t received_data = SPI_Transfer(SPI1, data);
			if (data == received_data){
				is_sent = true;
				GPIO_WritePin(GPIOC, 13, 0);
			}
			else{
				is_sent = false;
				GPIO_WritePin(GPIOC, 13, 1);
			}
			
			data++ ;
			delay_ms(1000);
	}
}*/

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "stm32f10x.h"
#include "my_rcc.h"
#include "my_gpio.h"
#include "my_timer.h" // Includes both your PWM and Input Capture functions
#include "my_adc.h"

// --- Global Variables for RPM Calculation ---
volatile uint16_t current_capture = 0;
volatile uint16_t previous_capture = 0;
volatile uint32_t pulse_period_us = 0;

// The smoothed RPM value you will use in your main loop
volatile float motor_rpm = 0.0f; 

// Setup for the Moving Average
#define ENCODER_HOLES_PER_REV 4.0f 
#define RPM_BUFFER_SIZE 8 
volatile float rpm_buffer[RPM_BUFFER_SIZE] = {0};
volatile uint8_t rpm_index = 0;
bool new_pulse_received = false;

// The Hardware Interrupt Handler (ISR)
void TIM3_IRQHandler(void) {
    if (TIM3->SR & (1 << 1)) { 
        
        current_capture = InputCapture_Read(TIM3, 1);
        
        if (current_capture >= previous_capture) {
            pulse_period_us = current_capture - previous_capture;
        } else {
            pulse_period_us = (0xFFFF - previous_capture) + current_capture;
        }
        
        previous_capture = current_capture;
        
        
        // Limit Rejection (Debounce)
        // If period is less than 500us, it would mean >30,000 RPM. 
        // This is physically impossible, so we ignore the glitch
        
        if (pulse_period_us > 500) {
            
            // Calculate the raw RPM for this single pulse
            float raw_rpm = (60000000.0f / (float)pulse_period_us) / ENCODER_HOLES_PER_REV; 
            new_pulse_received = true;
            
            // The Moving Average Circular Buffer
            
            // Overwrite the oldest data point with the new raw data
            rpm_buffer[rpm_index] = raw_rpm;
            
            // Move the index forward. If it hits the end (8), loop back to 0
            rpm_index = (rpm_index + 1) % RPM_BUFFER_SIZE;
            
            // Calculate the average of the buffer
            float sum = 0.0f;
            for(int i = 0; i < RPM_BUFFER_SIZE; i++) {
                sum += rpm_buffer[i];
            }
            
            // Output the final, cleaned, smoothed value!
            motor_rpm = sum / (float)RPM_BUFFER_SIZE;
        }
        
        // Clear the interrupt flag
        TIM3->SR &= ~(1 << 1); 
    }
}

int main(void) {
    // 1. Core System Configuration
    SystemClock_Config_72MHz();
    SysTick_Init();
    
    // 2. Initialize Serial Debugging (Assuming you have your USART driver ready)
    // USART1_Init_115200();
    // printf("\r\n--- Motor Profiling System: LIVE TEST ---\r\n");
		
    // 3. Configure the GPIO Pins
    GPIO_EnableClock(GPIOA);
    
    // PA0: Alternate Function Push-Pull for TIM2 PWM Output
    GPIO_InitPin(GPIOA, 0, GPIO_MODE_AF_PP); 
    
    // PA6: Floating Input for TIM3 Input Capture (Optocoupler)
    // Note: If your sensor needs a pull-up, change this to GPIO_MODE_INPUT_PULL
    GPIO_InitPin(GPIOA, 6, GPIO_MODE_INPUT_FLOATING); 
		
    // 4. Initialize the Peripherals
    // Set TIM2 Channel 1 to a 1000 Hz PWM frequency
    PWM_Init(TIM2, 1, 1000); 
    
    // Set TIM3 Channel 1 to Input Capture mode
    InputCapture_Init(TIM3, 1);
		
		// Initialize ADC to read potentiometer value
		ADC_Init(ADC1, 2);
		
    // Set duty to 0% at first
    PWM_SetDutyCycle(TIM2, 1, 0.0f);
    PWM_Start(TIM2);
		
    while(1) {
        // The hardware is handling the PWM generation and the Input Capture 
        // completely in the background. The CPU is completely free!
			
        uint16_t potentiometer_value = ADC_Read(ADC1);
				float duty_percentage = ((float)potentiometer_value / 4095.0f) * 100.0f;
				PWM_SetDutyCycle(TIM2, 1, duty_percentage);
        
        // Note: If the motor stops, the optocoupler stops firing, meaning the interrupt 
        // stops updating. If no pulse is seen for a long time, force RPM to 0.
        if (new_pulse_received == true) {
            // The motor is spinning. Reset the flag to catch the next window.
            new_pulse_received = false; 
        } else {
            // The flag is still false. No interrupts fired in the last 250ms!
            motor_rpm = 0.0f;
            pulse_period_us = 0;
            
            // Clear the moving average buffer so old data doesn't corrupt a restart
            for(int i = 0; i < RPM_BUFFER_SIZE; i++) {
                rpm_buffer[i] = 0.0f;
            }
        }
				
				// Safely copy the volatile RPM variable so it doesn't change mid-print
        float current_rpm = motor_rpm;
        
        // Update the screen every 250ms
        delay_ms(250); 
    }
}