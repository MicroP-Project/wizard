// STM32L432KC_TRNG.c
// CMSIS-based TRNG (RNG) polling implementation for STM32L432KC (RM0394 Section 24)

#include "STM32L432KC_TRNG.h"

/**
 * Initializes the 48 MHz HSI48 oscillator, routes it to CLK48,
 * enables the RNG peripheral clock on AHB2, and starts the RNG.
 */
void initTRNG(void) {
    // 1. Enable HSI48 oscillator (required 48 MHz clock source for RNG)
    RCC->CRRCR |= RCC_CRRCR_HSI48ON;
    while ((RCC->CRRCR & RCC_CRRCR_HSI48RDY) == 0);

    // 2. Select HSI48 as the CLK48 clock source (CLK48SEL = 0b00 in RCC_CCIPR)
    RCC->CCIPR &= ~RCC_CCIPR_CLK48SEL;

    // 3. Enable RNG peripheral clock on AHB2
    RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;
    volatile uint32_t dummy = RCC->AHB2ENR; // waiting to avoid timing hazards in clk initializations
    (void)dummy;

    // 4. Clear any residual error flags and enable RNG (polling mode: IE = 0)
    RNG->SR &= ~(RNG_SR_CEIS | RNG_SR_SEIS);
    RNG->CR |= RNG_CR_RNGEN;
}

/**
 * Polls the RNG until a valid 32-bit true random number is ready.
 * Automatically recovers from seed errors per RM0394 Section 24.3.7.
 */
uint32_t getRandomNumber(void) {
    while (1) {
        uint32_t status = RNG->SR;

        // Check for Clock Error (RNG_CLK < HCLK/16)
        if (status & (RNG_SR_CECS | RNG_SR_CEIS)) {
            RNG->SR &= ~RNG_SR_CEIS;
            continue;
        }

        // Check for Seed Error (faulty analog noise sequence)
        if (status & (RNG_SR_SECS | RNG_SR_SEIS)) {
            RNG->SR &= ~RNG_SR_SEIS;
            // RM0394 24.3.7: Cycle RNGEN to reinitialize the seed generator
            RNG->CR &= ~RNG_CR_RNGEN;
            RNG->CR |= RNG_CR_RNGEN;
            continue;
        }

        // Check if valid random data is ready in RNG->DR
        if (status & RNG_SR_DRDY) {
            uint32_t rand_val = RNG->DR;
            // Discard 0 as an extra safeguard against fault conditions
            if (rand_val != 0) {
                return rand_val;
            }
        }
    }
}

/**
 * Helper to return a uniform random integer in [min, max] (inclusive).
 */
uint32_t getRandomRange(uint32_t min, uint32_t max) {
    if (min >= max) return min;
    uint32_t span = max - min + 1;
    return min + (getRandomNumber() % span);
}


uint32_t nextSpellGenerator(void) {
    return getRandomRange(1, 4);
} 

