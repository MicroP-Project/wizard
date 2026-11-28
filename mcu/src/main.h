

#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stm32l432xx.h>          // CMSIS device definitions
#include "STM32L432KC_RCC.h"      // configureClock()
#include "STM32L432KC_GPIO.h"     // gpioEnable(), pinMode(), digitalRead/Write()
#include "STM32L432KC_TIM.h"      // timers / delays
#include "STM32L432KC_SPI.h"      // initSPI(), spiSendReceive()
#include "ACCEL_SPI.h"            // initAccel(), accelReadMg(), accel_t
#include "STM32L432KC_FLASH.h"
#include "STM32L432KC_TRNG.h"       // initTRNG(), nextSpellGenerator()
#include "fpga_spi.h"


#define BUTTON_PIN PA9
#define THRESH_X_Y 0x300
#define THRESH_Z_UP 0x600
#define THRESH_Z_DOWN 0x0

// Round timeout (TIM2 ticks at 10 kHz, set by initTIM's prescaler)
#define TIMEOUT_MS    5000                    // time per round in ms -- change this to adjust
#define TIM2_TICK_HZ  10000                   // TIM2 counter rate after prescaler
#define TIMEOUT_ARR   ((TIMEOUT_MS * (TIM2_TICK_HZ / 1000)) - 1)   // 5000 ms -> 49999


#endif // MAIN_H