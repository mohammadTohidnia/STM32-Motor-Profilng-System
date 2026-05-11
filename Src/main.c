#include <stdio.h>
#include "stm32f10x.h"
#include "my_gpio.h"
#include "my_rcc.h"
#include "my_timer.h"

/*void EXTI9_5_IRQHandler(void)
{
	GPIO_ClearInterruptFlag(5) ;
	
	GPIO_TogglePin(GPIOC, 13);

}*/

volatile float k = 0;

int main(void) {
		
	// Set the maximum clk for peripherals
	SystemClock_Config_72MHz();
	// Initialize Systick to processer clock
	SysTick_Init();
	
	GPIO_EnableClock(GPIOA);
	GPIO_InitPin(GPIOA, 0, GPIO_MODE_AF_PP);
	GPIO_WritePin(GPIOA, 0, 1);
	
	PWM_Init(TIM2, 1, 1000) ;
	PWM_SetDutyCycle(TIM2, 1, 0);
	PWM_Start(TIM2);
	
	float fade = 1.0f ;
	while(1) {
			// Toggle the LED
			k += fade ;
			if (k > 100.0f || k < 0.0f){
				fade = -fade;
			}
			PWM_SetDutyCycle(TIM2, 1, k);
			
		
			
			delay_ms(100);
	}
}


