// ACCEL_SPI.h
// Driver for the ST LIS3DH 3-axis accelerometer over 4-wire SPI.
// Built on top of STM32L432KC_SPI (SPI1: SCK=PB3, MISO=PB4, MOSI=PB5, CS=PB0).
// Datasheet: https://pages.hmc.edu/harris/class/e85/LIS3DH.pdf (Doc ID 17530)

#ifndef ACCEL_SPI_H
#define ACCEL_SPI_H

#include <stdint.h>
#include <stm32l432xx.h>
#include "STM32L432KC_GPIO.h"
#include "STM32L432KC_SPI.h"

///////////////////////////////////////////////////////////////////////////////
// Wiring / SPI settings
///////////////////////////////////////////////////////////////////////////////

// Chip select: active low, must idle HIGH. Software-controlled GPIO.
#define ACCEL_CS_PIN   OUTPUT_PIN   // PB0

// LIS3DH: SPC idles high when CS is high, data driven on the falling edge and
// sampled on the rising edge -> SPI mode 3 (CPOL = 1, CPHA = 1). MSB first, 8-bit.
#define ACCEL_SPI_CPOL 1
#define ACCEL_SPI_CPHA 1

// Max SPC is 10 MHz (Table 6). SPI1 clock = 80 MHz -> f = 80 MHz / 2^(BR+1).
// BR = 0b100 -> 2.5 MHz: plenty fast and safe with default GPIO slew rate / jumper wires.
#define ACCEL_SPI_BR   0b100

///////////////////////////////////////////////////////////////////////////////
// SPI command byte (first byte of each transaction)
//   bit 7: RW  (1 = read, 0 = write)
//   bit 6: MS  (1 = auto-increment address on multi-byte transfers)
//   bit 5-0: register address
///////////////////////////////////////////////////////////////////////////////
#define ACCEL_READ     0x80
#define ACCEL_MULTI    0x40
#define ACCEL_ADDR_MSK 0x3F

///////////////////////////////////////////////////////////////////////////////
// Register map (Table 17). Do NOT write to reserved registers (00h-06h,
// 10h-1Eh, 34h-37h) -- the datasheet warns this can permanently damage the part.
///////////////////////////////////////////////////////////////////////////////
#define LIS3DH_STATUS_REG_AUX 0x07
#define LIS3DH_OUT_ADC1_L     0x08
#define LIS3DH_INT_COUNTER    0x0E
#define LIS3DH_WHO_AM_I       0x0F
#define LIS3DH_TEMP_CFG_REG   0x1F
#define LIS3DH_CTRL_REG1      0x20
#define LIS3DH_CTRL_REG2      0x21
#define LIS3DH_CTRL_REG3      0x22
#define LIS3DH_CTRL_REG4      0x23
#define LIS3DH_CTRL_REG5      0x24
#define LIS3DH_CTRL_REG6      0x25
#define LIS3DH_REFERENCE      0x26
#define LIS3DH_STATUS_REG     0x27
#define LIS3DH_OUT_X_L        0x28
#define LIS3DH_OUT_X_H        0x29
#define LIS3DH_OUT_Y_L        0x2A
#define LIS3DH_OUT_Y_H        0x2B
#define LIS3DH_OUT_Z_L        0x2C
#define LIS3DH_OUT_Z_H        0x2D
#define LIS3DH_FIFO_CTRL_REG  0x2E
#define LIS3DH_FIFO_SRC_REG   0x2F
#define LIS3DH_INT1_CFG       0x30
#define LIS3DH_INT1_SOURCE    0x31
#define LIS3DH_INT1_THS       0x32
#define LIS3DH_INT1_DURATION  0x33
#define LIS3DH_CLICK_CFG      0x38
#define LIS3DH_CLICK_SRC      0x39
#define LIS3DH_CLICK_THS      0x3A
#define LIS3DH_TIME_LIMIT     0x3B
#define LIS3DH_TIME_LATENCY   0x3C
#define LIS3DH_TIME_WINDOW    0x3D

#define LIS3DH_WHO_AM_I_VAL   0x33  // Fixed device ID

///////////////////////////////////////////////////////////////////////////////
// Register bit fields
///////////////////////////////////////////////////////////////////////////////

// CTRL_REG1: ODR[7:4] | LPen | Zen | Yen | Xen
#define LIS3DH_ODR_POWERDOWN  (0x0 << 4)
#define LIS3DH_ODR_1HZ        (0x1 << 4)
#define LIS3DH_ODR_10HZ       (0x2 << 4)
#define LIS3DH_ODR_25HZ       (0x3 << 4)
#define LIS3DH_ODR_50HZ       (0x4 << 4)
#define LIS3DH_ODR_100HZ      (0x5 << 4)
#define LIS3DH_ODR_200HZ      (0x6 << 4)
#define LIS3DH_ODR_400HZ      (0x7 << 4)
#define LIS3DH_ODR_1K6HZ_LP   (0x8 << 4)  // low-power mode only
#define LIS3DH_ODR_1K25HZ     (0x9 << 4)  // 1.25 kHz normal / 5 kHz low-power
#define LIS3DH_CTRL1_LPEN     (1 << 3)
#define LIS3DH_CTRL1_ZEN      (1 << 2)
#define LIS3DH_CTRL1_YEN      (1 << 1)
#define LIS3DH_CTRL1_XEN      (1 << 0)
#define LIS3DH_CTRL1_XYZEN    (0x07)

// CTRL_REG4: BDU | BLE | FS[5:4] | HR | ST[2:1] | SIM
#define LIS3DH_CTRL4_BDU      (1 << 7)  // don't update H/L until both read
#define LIS3DH_CTRL4_BLE      (1 << 6)  // keep 0 (LSB at lower address)
#define LIS3DH_FS_2G          (0x0 << 4)
#define LIS3DH_FS_4G          (0x1 << 4)
#define LIS3DH_FS_8G          (0x2 << 4)
#define LIS3DH_FS_16G         (0x3 << 4)
#define LIS3DH_FS_MSK         (0x3 << 4)
#define LIS3DH_CTRL4_HR       (1 << 3)  // high resolution (12-bit)
#define LIS3DH_CTRL4_SIM      (1 << 0)  // keep 0 (4-wire SPI)

// CTRL_REG5
#define LIS3DH_CTRL5_BOOT     (1 << 7)  // reload factory trim (self-clearing)

// STATUS_REG
#define LIS3DH_STATUS_ZYXDA   (1 << 3)  // new X/Y/Z sample available
#define LIS3DH_STATUS_ZYXOR   (1 << 7)  // a sample was overwritten before read

///////////////////////////////////////////////////////////////////////////////
// Types
///////////////////////////////////////////////////////////////////////////////

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} accel_t;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

/* Low-level register access. Each call is one complete CS-framed transaction. */
uint8_t accelReadReg(uint8_t reg);
void    accelWriteReg(uint8_t reg, uint8_t val);

/* Read `len` consecutive registers starting at `reg` (uses MS auto-increment). */
void    accelReadMulti(uint8_t reg, uint8_t *buf, uint8_t len);

/* Enables GPIOB, configures SPI1 for the LIS3DH, then configures the sensor:
 * 100 Hz, X/Y/Z on, high-resolution (12-bit), +/-2 g, block data update.
 *    -- return: 1 if WHO_AM_I == 0x33 (sensor found), 0 otherwise.
 * Call configureClock() first. Give the sensor ~5 ms after power-up before calling. */
int     initAccel(void);

/* Change the full-scale range. fs = LIS3DH_FS_2G / _4G / _8G / _16G. */
void    accelSetRange(uint8_t fs);

/* 1 if a new X/Y/Z sample is ready (STATUS_REG.ZYXDA). */
int     accelDataReady(void);

/* Raw 12-bit signed counts (already right-shifted from the left-justified 16-bit
 * register value). */
void    accelReadRaw(accel_t *a);

/* Acceleration in milli-g, scaled for the currently configured range. */
void    accelReadMg(accel_t *a);

#endif
