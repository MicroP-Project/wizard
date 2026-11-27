// STM32L432KC_TRNG.h
// CMSIS-based TRNG (RNG) polling library for STM32L432KC (RM0394 Section 24)

#ifndef STM32L432KC_TRNG_H
#define STM32L432KC_TRNG_H

#include <stdint.h>
#include "stm32l432xx.h"

// Function prototypes
void initTRNG(void);
uint32_t getRandomNumber(void);
uint32_t getRandomRange(uint32_t min, uint32_t max);
uint32_t nextSpellGenerator(void);

#endif

