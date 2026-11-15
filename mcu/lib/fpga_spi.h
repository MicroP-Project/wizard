// fpga_spi.h
// Ellen Yu & Jessica Li
// ellyu@g.hmc.edu & jesli@hmc.edu
// Nov 14, 2026
// Driver for sending game state to the FPGA over SPI3.
// Built on top of STM32L432KC_SPI3 (SPI3: SCK=PB3, MISO=PB4, MOSI=PB5, CS=PA8).
// FPGA side: fpga/src/fpga_spi.sv (16-bit packet, latched on CS rising edge).

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
// Packet format (16 bits, sent MSB first -> lands in packet[15:0] on the FPGA)
//
//   byte 0 = packet[15:8]:  state[7:6] | direction[5:3] | lives[2:0]
//   byte 1 = packet[7:0]:   score (0-255)
//
// FPGA decode:  state = packet[15:14], direction = packet[13:11],
//               lives = packet[10:8],  score     = packet[7:0]
///////////////////////////////////////////////////////////////////////////////

// state (2 bits)
#define FPGA_STATE_IDLE      0  // title / waiting for button
#define FPGA_STATE_PLAYING   1  // show the spell direction to cast
#define FPGA_STATE_HIT       2  // correct direction cast
#define FPGA_STATE_GAMEOVER  3

// direction (3 bits) -- same numbering as user_input in main.c
#define FPGA_DIR_NONE  0
#define FPGA_DIR_POS_X 1
#define FPGA_DIR_NEG_X 2
#define FPGA_DIR_POS_Y 3
#define FPGA_DIR_NEG_Y 4
#define FPGA_DIR_POS_Z 5
#define FPGA_DIR_NEG_Z 6

#define FPGA_STATE_SHIFT 6
#define FPGA_STATE_MSK   0x03
#define FPGA_DIR_SHIFT   3
#define FPGA_DIR_MSK     0x07
#define FPGA_LIVES_SHIFT 0
#define FPGA_LIVES_MSK   0x07   // max 7 lives

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

/* Configures SPI3 for the FPGA (mode 3, 2.5 MHz) and leaves CS high (idle).
 * Call configureClock() first. */
void    initFPGA(void);

/* Sends one raw 2-byte packet as a single CS-framed transaction.
 * The FPGA latches it when CS goes back high. */
void    fpgaSendPacket(uint8_t byte0, uint8_t byte1);

/* Packs the game state into byte 0 (see packet format above). Values are
 * masked to their field widths. */
uint8_t fpgaPackStatus(uint8_t state, uint8_t direction, uint8_t lives);

/* Packs and sends the whole game state in one call. */
void    fpgaSendState(uint8_t state, uint8_t direction, uint8_t lives, uint8_t score);

#endif
