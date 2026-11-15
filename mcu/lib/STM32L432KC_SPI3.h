// STM32L432KC_SPI3.h
// Ellen Yu & Jessica Li
// ellyu@g.hmc.edu & jesli@hmc.edu
// Nov 14, 2026
// Header for SPI3 (used for comms with the FPGA)

#ifndef STM32L4_SPI3_H
#define STM32L4_SPI3_H

#include <stdint.h>
#include "STM32L432KC_GPIO.h"
#include <stm32l432xx.h>


// GPIO pin definitions for SPI3
// available pins for SPI3 on the 32-pin L432KC (all AF6):
// SCK: PB3
// MISO: PB4
// MOSI: PB5
// (PC10-PC12 also map to SPI3 but are not bonded out on this package)
//
// Names are prefixed with SPI3_ so they don't clash with the SPI1 defines
// (OUTPUT_PIN, SCK_PIN, ...) in STM32L432KC_SPI.h.

#define SPI3_CS_PIN   PA8  // GPIO pin used for SPI3 chip select to the FPGA (any free GPIO works)
#define SPI3_SCK_PIN  PB3
#define SPI3_MISO_PIN PB4  // FPGA -> MCU; may be left unconnected if the FPGA is receive-only
#define SPI3_MOSI_PIN PB5
///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

/* Enables the SPI3 peripheral and intializes its clock speed (baud rate), polarity, and phase.
 *    -- br: (0b000 - 0b111). The SPI clk will be the APB1 clock (80 MHz) / 2^(BR+1).
 *    -- cpol: clock polarity (0: inactive state is logical 0, 1: inactive state is logical 1).
 *    -- cpha: clock phase (0: data captured on leading edge of clk and changed on next edge,
 *          1: data changed on leading edge of clk and captured on next edge)
 * Leaves the chip select (SPI3_CS_PIN) HIGH (idle).
 * Refer to the datasheet for more low-level details. */
void initSPI3(int br, int cpol, int cpha);

/* Transmits a character (1 byte) over SPI3 and returns the received character.
 * Does NOT touch chip select; frame a packet like this:
 *      digitalWrite(SPI3_CS_PIN, PIO_LOW);
 *      spi3SendReceive(byte0);
 *      spi3SendReceive(byte1);
 *      while (SPI3->SR & SPI_SR_BSY);      // let the last bit finish shifting out
 *      digitalWrite(SPI3_CS_PIN, PIO_HIGH); // FPGA latches the packet here
 *    -- send: the character to send over SPI3
 *    -- return: the character received over SPI3 */
char spi3SendReceive(char send);

#endif
