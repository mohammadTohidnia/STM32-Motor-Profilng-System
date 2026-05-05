#include "my_rcc.h"

void SystemClock_Config_72MHz(void) {
    
    // --- STEP 0: The Clean Slate Reset (Fixes ST-Link Lockups) ---
    RCC->CR |= RCC_CR_HSION;                        // Force HSI ON
    while (!(RCC->CR & RCC_CR_HSIRDY));             // Wait for HSI stability
    
    RCC->CFGR &= ~RCC_CFGR_SW;                      // Switch System Clock back to HSI
    while ((RCC->CFGR & RCC_CFGR_SWS) != 0x00);     // Wait until hardware confirms HSI
    
    RCC->CR &= ~RCC_CR_PLLON;                       // Turn OFF the PLL
    RCC->CR &= ~RCC_CR_HSEON;                       // Turn OFF the HSE
    
    // --- STEP 1: Initialize HSE (8MHz External Crystal) ---
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));

    // --- STEP 2: Configure Flash Latency and Bus Prescalers ---
    FLASH->ACR |= FLASH_ACR_PRFTBE;             
    FLASH->ACR &= ~FLASH_ACR_LATENCY;           
    FLASH->ACR |= FLASH_ACR_LATENCY_2;          

    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;                // AHB = 72MHz
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;               // APB1 = 36MHz (Max)
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;               // APB2 = 72MHz

    // --- STEP 3: Configure PLL (8MHz * 9 = 72MHz) ---
    RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL); 
    RCC->CFGR |= RCC_CFGR_PLLSRC;                   // Source = HSE
    RCC->CFGR |= RCC_CFGR_PLLMULL9;                     

    // --- STEP 4: Turn on PLL ---
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    // --- STEP 5: Switch System Clock to PLL ---
    RCC->CFGR &= ~RCC_CFGR_SW;      
    RCC->CFGR |= RCC_CFGR_SW_PLL;   
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}