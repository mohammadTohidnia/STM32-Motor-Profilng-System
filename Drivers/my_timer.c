#include "my_timer.h"

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
            // Enable CC1E Output for Channel 1 (bit 0)
            TIMx->CCER |= (1 << 0); 
            break;
        case 2:
            // Clear OC2M bits (bits 12, 13, 14)
            TIMx->CCMR1 &= ~(7 << 12);
            // Set to PWM Mode 1 (6 << 12) and enable OC2PE Preload (bit 11)
            TIMx->CCMR1 |= (6 << 12) | (1 << 11);
            // Enable CC2E Output for Channel 2 (bit 4)
            TIMx->CCER |= (1 << 4); 
            break;
        case 3:
            // Clear OC3M bits (bits 4, 5, 6)
            TIMx->CCMR2 &= ~(7 << 4);
            // Set to PWM Mode 1 (6 << 4) and enable OC3PE Preload (bit 3)
            TIMx->CCMR2 |= (6 << 4) | (1 << 3);
            // Enable CC3E Output for Channel 3 (bit 8)
            TIMx->CCER |= (1 << 8); 
            break;
        case 4:
            // Clear OC4M bits (bits 12, 13, 14)
            TIMx->CCMR2 &= ~(7 << 12);
            // Set to PWM Mode 1 (6 << 12) and enable OC4PE Preload (bit 11)
            TIMx->CCMR2 |= (6 << 12) | (1 << 11);
            // Enable CC4E Output for Channel 4 (bit 12)
            TIMx->CCER |= (1 << 12); 
            break;
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



/******************* DELAY DEVELOPMENT ********************/



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