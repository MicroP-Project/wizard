// fpga_spi.h
// Ellen Yu & Jessica Li
// ellyu@g.hmc.edu & jesli@hmc.edu
// Nov 14, 2026
// Driver for sending game state to the FPGA over SPI3.
// Built on top of STM32L432KC_SPI3 (SPI3: SCK=PB3, MISO=PB4, MOSI=PB5, CS=PA8).
// FPGA side: fpga/src/fpga_spi.sv (8-bit packet, latched on CS rising edge).

#ifndef FPGA_SPI_H
#define FPGA_SPI_H

#include <stdint.h>
#include <stm32l432xx.h>
#include "STM32L432KC_GPIO.h"
#include "STM32L432KC_SPI3.h"

///////////////////////////////////////////////////////////////////////////////
// Wiring / SPI settings
///////////////////////////////////////////////////////////////////////////////

// Chip select: active low, idles HIGH. Software-controlled GPIO.
#define FPGA_CS_PIN   SPI3_CS_PIN   // PA8

// Same mode as the accelerometer: SCK idles high, MCU changes MOSI on the falling
// edge, FPGA samples on the rising edge -> SPI mode 3 (CPOL = 1, CPHA = 1). MSB first.
#define FPGA_SPI_CPOL 1
#define FPGA_SPI_CPHA 1

// f = 80 MHz / 2^(BR+1). BR = 0b100 -> 2.5 MHz: slow enough for the FPGA to
// handle easily and safe with jumper wires.
#define FPGA_SPI_BR   0b100

///////////////////////////////////////////////////////////////////////////////
// Packet format (1 byte, sent MSB first -> lands in packet[7:0] on the FPGA)
//
//   bit:   7  6  |  5  4  3   |    2     |   1   |   0
//         unused | next spell | time out | wrong | right
//
//   next spell: the direction the player must cast next (shown by the FPGA)
//   right:      1 = the player cast the correct spell in time  (FPGA: score + 1)
//   wrong:      1 = the player cast the wrong spell            (FPGA: lives - 1)
//   time out:   1 = the player ran out of time                 (FPGA: lives - 1)
//
// The MCU sends one packet per round: the result of the round that just ended
// plus the spell for the next round. For the very first round, send the first
// spell with no flags set. Lives, score and game over are handled on the FPGA.
//
// FPGA decode:  next_spell = packet[5:3], timeout = packet[2],
//               wrong      = packet[1],   right   = packet[0]
///////////////////////////////////////////////////////////////////////////////

// spell directions (3 bits) -- same numbering as user_input in main.c
#define FPGA_DIR_NONE  0
#define FPGA_DIR_POS_X 1
#define FPGA_DIR_NEG_X 2
#define FPGA_DIR_POS_Y 3
#define FPGA_DIR_NEG_Y 4
#define FPGA_DIR_POS_Z 5
#define FPGA_DIR_NEG_Z 6

// result flags (one bit each)
#define FPGA_FLAG_NONE    0b000   // no result yet (first round)
#define FPGA_FLAG_RIGHT   0b001   // bit 0
#define FPGA_FLAG_WRONG   0b010   // bit 1
#define FPGA_FLAG_TIMEOUT 0b100   // bit 2

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

/* Configures SPI3 for the FPGA (mode 3, 2.5 MHz) and leaves CS high (idle).
 * Call configureClock() first. */
void    initFPGA(void);

/* Sends one raw 1-byte packet as a single CS-framed transaction.
 * The FPGA latches it when CS goes back high. */
void    fpgaSendPacket(uint8_t packet);

/* Builds the packet from the next spell and a result flag, then sends it.
 *    -- next_spell: FPGA_DIR_POS_X ... FPGA_DIR_NEG_Z
 *    -- result:     FPGA_FLAG_NONE, FPGA_FLAG_RIGHT, FPGA_FLAG_WRONG or FPGA_FLAG_TIMEOUT
 * Example: fpgaSendRound(FPGA_DIR_NEG_Y, FPGA_FLAG_RIGHT); */
void    fpgaSendRound(uint8_t next_spell, uint8_t result);

#endif
