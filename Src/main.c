
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
#include "my_timer.h"
#include "my_spi.h"

int main(void) {
    // 1. Initialize the Core System
    SystemClock_Config_72MHz();
    SysTick_Init();
    
    // 2. Initialize Serial Debugging (Assuming you have USART1 configured)
    // USART1_Init_115200(); 
    // printf("\r\n--- STM32 SPI Master Controller Ready ---\r\n");

    // 3. Initialize SPI1 with a Clock Divider of 16 (4.5 MHz)
    SPI_Master_Init(SPI1, 16);
    
    // 4. Configure PA4 manually as the Chip Select (CS) pin
    GPIO_EnableClock(GPIOA);
    GPIO_InitPin(GPIOA, 4, GPIO_MODE_OUTPUT_PP);
    
    // Keep CS HIGH (Inactive) to start
    SPI_CS_Disable(GPIOA, 4); 

    uint8_t test_counter = 0; // Create a variable to count up

    while(1) {
        // --- START SPI TRANSACTION ---
        
        // 1. Pull CS Low to select the ESP32-S3
        SPI_CS_Enable(GPIOA, 4);
        
        // 2. Send the changing counter instead of 0x00
        uint8_t received_data = SPI_Transfer(SPI1, test_counter); 
        
        // 3. Pull CS High to end the transaction
        SPI_CS_Disable(GPIOA, 4);
        
        // --- END SPI TRANSACTION ---

        // Print the result to your PC (Uncomment this!)
        // printf("Received from ESP32: 0x%X\r\n", received_data);
        
        test_counter++; // Increase the number by 1 for the next loop
        
        delay_ms(500); 
    }
}
