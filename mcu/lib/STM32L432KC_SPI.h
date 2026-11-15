// STM32L432KC_SPI.c
// Ellen Yu & Jessica Li
// ellyu@g.hmc.edu & jesli@hmc.edu
// Nov 14, 2026
// Header for SPI1 (used for comms with accelerometer)

#ifndef STM32L4_SPI_H
#define STM32L4_SPI_H

#include <stdint.h>
#include "STM32L432KC_GPIO.h"
#include <stm32l432xx.h>


// GPIO pin definitions for SPI1
// available pins for SPI1: 
// SCK: PA5, PB3
// MISO: PA6, PB4
// MOSI: PA7, PB5

#define OUTPUT_PIN PA10 // GPIO pin used for SPI NSS (chip select) output
#define SCK_PIN    PA5 
#define MISO_PIN   PA6 
#define MOSI_PIN   PA7 
///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

/* Enables the SPI peripheral and intializes its clock speed (baud rate), polarity, and phase.
 *    -- br: (0b000 - 0b111). The SPI clk will be the master clock / 2^(BR+1).
 *    -- cpol: clock polarity (0: inactive state is logical 0, 1: inactive state is logical 1).
 *    -- cpha: clock phase (0: data captured on leading edge of clk and changed on next edge, 
 *          1: data changed on leading edge of clk and captured on next edge)
 * Refer to the datasheet for more low-level details. */ 
void initSPI(int br, int cpol, int cpha);

/* Transmits a character (1 byte) over SPI and returns the received character.
 *    -- send: the character to send over SPI
 *    -- return: the character received over SPI */
char spiSendReceive(char send);

#endif