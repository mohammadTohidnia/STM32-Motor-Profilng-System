#include "my_timer.h"

/************************************************ Timer PWM Mode *******************************************************/

// We assume that we configure the clock tree to the max of 72MHz
// Assume a 72MHz timer clock base (APB1 runs at 36MHz, but Timers are multiplied by 2)
#define TIMER_CLOCK_HZ 72000000 
#define PWM_RESOLUTION 1000 // ARR value for easy duty cycle math (0-1000)

void PWM_Init(TIM_TypeDef *TIMx, uint8_t channel, uint32_t frequency_hz) {
    // Enable the clock for the specific timer
		if (TIMx == TIM1) {
        RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;  // TIM1 lives on the APB2 bus
    }
    else if (TIMx == TIM2){									 // TIM2,3,4 live on APB1 bus
			RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
		} 
    else if (TIMx == TIM3){
			RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
		} 
    else if (TIMx == TIM4){
			RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
		} 
    
    // Calculate and set Prescaler and Auto-Reload
    // Formula: Timer_Clock / (Prescaler * ARR) = Frequency
    uint32_t prescaler = (TIMER_CLOCK_HZ / (PWM_RESOLUTION * frequency_hz)) - 1; // We decrease 1 because what we write will be plused by 1
    
    TIMx->PSC = prescaler;
    TIMx->ARR = PWM_RESOLUTION - 1; 
    
    // Enable Auto-Reload Preload
    TIMx->CR1 |= (1 << 7);

    // Configure the Specific Channel
    // CCMR1 handles Channels 1 & 2. CCMR2 handles Channels 3 & 4.
    switch (channel) {
        case 1:
            // Clear OC1M bits (bits 4, 5, 6)
            TIMx->CCMR1 &= ~(7 << 4);
            // Set to PWM Mode 1 (6 << 4) and enable OC1PE Preload (bit 3)
            TIMx->CCMR1 |= (6 << 4) | (1 << 3);
            
            // If TIM1, enable CC1E (bit 0) AND Complementary CC1NE (bit 2)
            if (TIMx == TIM1)
							TIMx->CCER |= (1 << 0) | (1 << 2); 
            else 
							TIMx->CCER |= (1 << 0); 			// Enable CC1E Output for Channel 1 (bit 0)
            break;
            
        case 2:
            // Clear OC2M bits (bits 12, 13, 14)
            TIMx->CCMR1 &= ~(7 << 12);
            // Set to PWM Mode 1 (6 << 12) and enable OC2PE Preload (bit 11)
            TIMx->CCMR1 |= (6 << 12) | (1 << 11);
            
						// If TIM1, enable CC1E (bit 4) AND Complementary CC1NE (bit 6)
            if (TIMx == TIM1)
							TIMx->CCER |= (1 << 4) | (1 << 6); 
            else 
							TIMx->CCER |= (1 << 4); 			// Enable CC1E Output for Channel 2 (bit 4)
            break;
						
        case 3:
            // Clear OC3M bits (bits 4, 5, 6)
            TIMx->CCMR2 &= ~(7 << 4);
            // Set to PWM Mode 1 (6 << 4) and enable OC3PE Preload (bit 3)
            TIMx->CCMR2 |= (6 << 4) | (1 << 3);
				
            // If TIM1, enable CC1E (bit 8) AND Complementary CC1NE (bit 10)
            if (TIMx == TIM1)
							TIMx->CCER |= (1 << 8) | (1 << 10); 
            else 
							TIMx->CCER |= (1 << 8); 			// Enable CC1E Output for Channel 3 (bit 8)
            break;
						
        case 4:
            // Clear OC4M bits (bits 12, 13, 14)
            TIMx->CCMR2 &= ~(7 << 12);
            // Set to PWM Mode 1 (6 << 12) and enable OC4PE Preload (bit 11)
            TIMx->CCMR2 |= (6 << 12) | (1 << 11);
						
						// NOTE: Channel 4 doesn't have complementry output for PWM. So, we only have basic PWM generation for channel 4
						
            // Enable CC4E Output for Channel 4 (bit 12)
            TIMx->CCER |= (1 << 12); 
            break;
    }
		
		// Advanced Timer Specific Configurations (Set dead time and output eanable)
    if (TIMx == TIM1) {
        // BDTR Register configuration:
        // Bit 15: MOE (Main Output Enable) -> Must be 1 to output signals
        // Bits 7:0: DTG (Dead-Time Generator) -> 0x2F gives ~650ns dead-time at 72MHz
        TIMx->BDTR |= (1 << 15) | 0x2F;
    }
		
}

void PWM_SetDutyCycle(TIM_TypeDef *TIMx, uint8_t channel, float duty_cycle_percent) {
    // Prevent out-of-bounds values
    if (duty_cycle_percent > 100.0f) duty_cycle_percent = 100.0f;
    if (duty_cycle_percent < 0.0f) duty_cycle_percent = 0.0f;
    
    // Calculate the Capture/Compare Register (CCR) value
    // E.g., 25% of 1000 = 250
    uint32_t ccr_value = (uint32_t)((duty_cycle_percent / 100.0f) * PWM_RESOLUTION);
    
    // Write to the correct shadow register
    switch (channel) {
        case 1: TIMx->CCR1 = ccr_value; break;
        case 2: TIMx->CCR2 = ccr_value; break;
        case 3: TIMx->CCR3 = ccr_value; break;
        case 4: TIMx->CCR4 = ccr_value; break;
    }
}

void PWM_Start(TIM_TypeDef *TIMx) {
    // Set the CEN (Counter Enable) bit in Control Register 1
    TIMx->CR1 |= (1 << 0);
}








/************************************************ DELAY DEVELOPMENT *******************************************************/

void SysTick_Init(void) {
    // Disable SysTick before configuration (clear bit 0)
    SysTick->CTRL &= ~(1 << 0);
    
    // Set the clock source to the Processor Clock (72MHz)
    SysTick->CTRL |= (1 << 2);
}

void delay_us(uint32_t us) {
    if (us == 0) return; // Prevent infinite loop if 0 is passed
    
    // Calculate the countdown value. 
    // At 72MHz, 1 microsecond = 72 ticks.
    // We subtract 1 because counting down to 0 takes 1 extra tick.
    SysTick->LOAD = (us * 72) - 1;
    
    // Clear the current value register
    SysTick->VAL = 0;
    
    // Enable the SysTick Timer (Set bit 0)
    SysTick->CTRL |= (1 << 0);
    
    // Wait for the COUNTFLAG to be set
    // Bit 16 in the CTRL register turns to 1 the exact moment the timer hits 0
    while (!(SysTick->CTRL & (1 << 16)));
    
    // Disable the timer to save power
    SysTick->CTRL &= ~(1 << 0);
}

void delay_ms(uint32_t ms) {
    // We chain the microsecond delay to easily handle multiple milliseconds
    for (uint32_t i = 0; i < ms; i++) {
        delay_us(1000); // Wait 1000 microseconds (1 millisecond)
    }
}








/************************************************ Timer Input Capture Mode *******************************************************/

void InputCapture_Init(TIM_TypeDef *TIMx, uint8_t channel) {
    // Enable the clock for the specific timer
    if (TIMx == TIM1) 
			RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    else if (TIMx == TIM2)
			RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    else if (TIMx == TIM3)
			RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    else if (TIMx == TIM4)
			RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    // Set Timebase to 1 MHz (1 tick = 1 microsecond)
    // 72MHz / 72 = 1MHz. We subtract 1 for the hardware register.
    TIMx->PSC = 71; 
    TIMx->ARR = 0xFFFF; // We set it to maximum amout

    // Configure the Specific Channel for Input Capture
    switch (channel) {
        case 1:
            TIMx->CCMR1 &= ~(3 << 0);  // Clear CC1S bits
            TIMx->CCMR1 |= (1 << 0);   // Set CC1S = 01 (Configure as Input on TI1)
						// 0x0F (1111) is the maximum filter strength. It samples the pin 8 times
            // at a divided clock rate to guarantee the pulse is real, not a glitch.
            TIMx->CCMR1 |= (0x0F << 4);
            TIMx->CCER  &= ~(1 << 1);  // Clear CC1P to trigger on Rising Edge
            TIMx->CCER  |= (1 << 0);   // Set CC1E to Enable Capture
            TIMx->DIER  |= (1 << 1);   // Set CC1IE to Enable Interrupt
            break;
        case 2:
            TIMx->CCMR1 &= ~(3 << 8);  
            TIMx->CCMR1 |= (1 << 8);   // CC2S = 01 (Input on TI2)
						TIMx->CCMR1 |= (0x0F << 12);
            TIMx->CCER  &= ~(1 << 5);  // Rising Edge
            TIMx->CCER  |= (1 << 4);   // Enable Capture
            TIMx->DIER  |= (1 << 2);   // Enable Interrupt
            break;
        case 3:
            TIMx->CCMR2 &= ~(3 << 0);  
            TIMx->CCMR2 |= (1 << 0);   // CC3S = 01 (Input on TI3)
						TIMx->CCMR2 |= (0x0F << 4);
            TIMx->CCER  &= ~(1 << 9);  // Rising Edge
            TIMx->CCER  |= (1 << 8);   // Enable Capture
            TIMx->DIER  |= (1 << 3);   // Enable Interrupt
            break;
        case 4:
            TIMx->CCMR2 &= ~(3 << 8);  
            TIMx->CCMR2 |= (1 << 8);   // CC4S = 01 (Input on TI4)
						TIMx->CCMR2 |= (0x0F << 12);
            TIMx->CCER  &= ~(1 << 13); // Rising Edge
            TIMx->CCER  |= (1 << 12);  // Enable Capture
            TIMx->DIER  |= (1 << 4);   // Enable Interrupt
            break;
    }

    // Enable the Timer Interrupt in the NVIC (Processor level)
    if (TIMx == TIM1) {

			NVIC_EnableIRQ(TIM1_CC_IRQn); 
    } else if (TIMx == TIM2) {
        NVIC_EnableIRQ(TIM2_IRQn);
    } else if (TIMx == TIM3) {
        NVIC_EnableIRQ(TIM3_IRQn);
    } else if (TIMx == TIM4) {
        NVIC_EnableIRQ(TIM4_IRQn);
    }

    // Start the Timer
    TIMx->CR1 |= (1 << 0);
}

// We will read the value in the event time
uint16_t InputCapture_Read(TIM_TypeDef *TIMx, uint8_t channel) {
    // Read the hardware Capture/Compare Register
    switch (channel) {
        case 1: return TIMx->CCR1;
        case 2: return TIMx->CCR2;
        case 3: return TIMx->CCR3;
        case 4: return TIMx->CCR4;
        default: return 0;
    }
}