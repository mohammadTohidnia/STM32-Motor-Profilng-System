#include "stm32f10x.h"
#include "my_gpio.h"
#include "my_rcc.h"


void delay(volatile uint32_t);

void EXTI0_IRQHandler(void)
{
	GPIO_ClearInterruptFlag(2) ;

}



int main(void) {
		
	// Set the maximum clk for peripherals
	SystemClock_Config_72MHz();
	
	// Enable the clock for Port C, A
	GPIO_EnableClock(GPIOC);
	GPIO_EnableClock(GPIOA);
	
	GPIO_InitPin(GPIOC, 13, GPIO_MODE_OUTPUT_PP) ;
	// Configure PC13 as a Push-Pull Output
	GPIO_InitPin(GPIOC, 13, GPIO_MODE_OUTPUT_PP);
	GPIO_WritePin(GPIOC, 13, 1);
	
	GPIO_InitPin(GPIOA, 5, GPIO_MODE_INPUT_FLOATING) ;
	GPIO_ConfigInterrupt(GPIOA, 5, EXTI_TRIGGER_RISING);
	GPIO_EnableInterrupt(5, 0) ;
	
	
	while(1) {
			// Toggle the LED
			//GPIO_TogglePin(GPIOC, 13);
			//delay(3000000); 
	}
}


// A simple, inaccurate delay function for testing
void delay(volatile uint32_t count) {
    while(count--) {
        // Just burn CPU cycles
    }
}