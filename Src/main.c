
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


#include <stdio.h>
#include "stm32f10x.h"
#include "my_rcc.h"
#include "my_gpio.h"
#include "my_timer.h" // Includes both your PWM and Input Capture functions

// --- Global Variables for RPM Calculation ---
// These must be volatile because they change inside the hardware interrupt
volatile uint16_t current_capture = 0;
volatile uint16_t previous_capture = 0;
volatile uint32_t pulse_period_us = 0;
volatile float motor_rpm = 0.0f;

// Set this to the number of slots/holes in your optocoupler encoder disc
#define ENCODER_HOLES_PER_REV 4.0f 

// --- The Hardware Interrupt Handler (ISR) ---
// The CPU drops everything and runs this the exact microsecond the optocoupler fires
void TIM3_IRQHandler(void) {
    // Check if the interrupt was caused by a Capture on Channel 1 (CC1IF flag)
    if (TIM3->SR & (1 << 1)) { 
        
        // Read the new timestamp
        current_capture = InputCapture_Read(TIM3, 1);
        
        // Calculate the microsecond time difference 
        if (current_capture >= previous_capture) {
            pulse_period_us = current_capture - previous_capture;
        } else {
            // Handle the 16-bit timer wrapping around back to 0
            pulse_period_us = (0xFFFF - previous_capture) + current_capture;
        }
        
        previous_capture = current_capture;
        
        // Calculate RPM
        // Formula: (1 minute in microseconds / period) / holes per revolution
        if (pulse_period_us > 0) {
            motor_rpm = (60000000.0f / (float)pulse_period_us) / ENCODER_HOLES_PER_REV; 
        }
        
        // Clear the interrupt flag so the CPU can leave the handler
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

    // Start the motor at 50% speed
    PWM_SetDutyCycle(TIM2, 1, 50.0f);
    PWM_Start(TIM2);

    while(1) {
        // The hardware is handling the PWM generation and the Input Capture 
        // completely in the background. The CPU is completely free!
        
        // Safely copy the volatile RPM variable so it doesn't change mid-print
        float current_rpm = motor_rpm;
        
        // Note: If the motor stops, the optocoupler stops firing, meaning the interrupt 
        // stops updating. If no pulse is seen for a long time, force RPM to 0.
        if (SysTick->VAL % 1000 == 0) { // Simple software timeout logic
             // If period is unreasonably long (e.g., > 100ms between holes), it's stopped
             if(pulse_period_us > 100000) motor_rpm = 0.0f;
        }

        // Print the live RPM to the screen
        // printf("Live Motor RPM: %.2f\r\n", current_rpm);
        
        // Update the screen every 250ms
        delay_ms(250); 
    }
}