

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

#define BUTTON_PIN PA6 // LED pin for blinking (D4, green LED on development pcb)
#define THRESH_X_Y 100
#define THRESH_Z 1000


#endif // MAIN_H