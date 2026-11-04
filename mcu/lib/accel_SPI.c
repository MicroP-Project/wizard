// accel_SPI.c
// Driver for the ST LIS3DH 3-axis accelerometer over 4-wire SPI (SPI1).
// See ACCEL_SPI.h for wiring and register definitions.

#include "ACCEL_SPI.h"

// Sensitivity in high-resolution (12-bit) mode, mg per LSB (Table 3):
//   +/-2g: 1, +/-4g: 2, +/-8g: 4, +/-16g: 12
static uint8_t accel_mg_per_lsb = 1;

///////////////////////////////////////////////////////////////////////////////
// Chip select helpers
///////////////////////////////////////////////////////////////////////////////

static inline void accelSelect(void)   { digitalWrite(ACCEL_CS_PIN, PIO_LOW);  }
static inline void accelDeselect(void) {
    // Make sure the last byte has fully shifted out before releasing CS.
    while (SPI1->SR & SPI_SR_BSY);
    digitalWrite(ACCEL_CS_PIN, PIO_HIGH);
}

///////////////////////////////////////////////////////////////////////////////
// Register access
///////////////////////////////////////////////////////////////////////////////

uint8_t accelReadReg(uint8_t reg) {
    accelSelect();
    spiSendReceive(ACCEL_READ | (reg & ACCEL_ADDR_MSK));
    uint8_t val = (uint8_t) spiSendReceive(0x00);   // dummy byte clocks data out
    accelDeselect();
    return val;
}

void accelWriteReg(uint8_t reg, uint8_t val) {
    accelSelect();
    spiSendReceive(reg & ACCEL_ADDR_MSK);            // RW = 0
    spiSendReceive(val);
    accelDeselect();
}

void accelReadMulti(uint8_t reg, uint8_t *buf, uint8_t len) {
    accelSelect();
    spiSendReceive(ACCEL_READ | ACCEL_MULTI | (reg & ACCEL_ADDR_MSK));
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = (uint8_t) spiSendReceive(0x00);
    }
    accelDeselect();
}

///////////////////////////////////////////////////////////////////////////////
// Setup
///////////////////////////////////////////////////////////////////////////////

int initAccel(void) {
    // initSPI() uses pinMode() on port B but does not turn on its clock.
    gpioEnable(GPIO_PORT_B);

    initSPI(ACCEL_SPI_BR, ACCEL_SPI_CPOL, ACCEL_SPI_CPHA);

    // initSPI() leaves CS low; the LIS3DH frames transactions on CS falling
    // edges, so CS must idle high.
    digitalWrite(ACCEL_CS_PIN, PIO_HIGH);

    // Check the device is present and SPI mode is correct.
    if (accelReadReg(LIS3DH_WHO_AM_I) != LIS3DH_WHO_AM_I_VAL) return 0;

    // 100 Hz, normal (not low-power) mode, X/Y/Z enabled.
    accelWriteReg(LIS3DH_CTRL_REG1, LIS3DH_ODR_100HZ | LIS3DH_CTRL1_XYZEN);

    // No high-pass filter, no interrupts.
    accelWriteReg(LIS3DH_CTRL_REG2, 0x00);
    accelWriteReg(LIS3DH_CTRL_REG3, 0x00);

    // Block data update, little endian, +/-2g, high resolution, 4-wire SPI.
    accelWriteReg(LIS3DH_CTRL_REG4, LIS3DH_CTRL4_BDU | LIS3DH_FS_2G | LIS3DH_CTRL4_HR);
    accel_mg_per_lsb = 1;

    return 1;
}

void accelSetRange(uint8_t fs) {
    uint8_t r4 = accelReadReg(LIS3DH_CTRL_REG4);
    r4 = (r4 & ~LIS3DH_FS_MSK) | (fs & LIS3DH_FS_MSK);
    accelWriteReg(LIS3DH_CTRL_REG4, r4);

    switch (fs & LIS3DH_FS_MSK) {
        case LIS3DH_FS_2G:  accel_mg_per_lsb = 1;  break;
        case LIS3DH_FS_4G:  accel_mg_per_lsb = 2;  break;
        case LIS3DH_FS_8G:  accel_mg_per_lsb = 4;  break;
        case LIS3DH_FS_16G: accel_mg_per_lsb = 12; break;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Data
///////////////////////////////////////////////////////////////////////////////

int accelDataReady(void) {
    return (accelReadReg(LIS3DH_STATUS_REG) & LIS3DH_STATUS_ZYXDA) != 0;
}

void accelReadRaw(accel_t *a) {
    uint8_t b[6];
    // One burst read of OUT_X_L..OUT_Z_H so all three axes come from the same sample.
    accelReadMulti(LIS3DH_OUT_X_L, b, 6);

    // Output is 16-bit two's complement, left-justified; in HR mode the top
    // 12 bits are valid. Arithmetic right shift keeps the sign.
    a->x = ((int16_t)((b[1] << 8) | b[0])) >> 4;
    a->y = ((int16_t)((b[3] << 8) | b[2])) >> 4;
    a->z = ((int16_t)((b[5] << 8) | b[4])) >> 4;
}

void accelReadMg(accel_t *a) {
    accelReadRaw(a);
    a->x *= accel_mg_per_lsb;
    a->y *= accel_mg_per_lsb;
    a->z *= accel_mg_per_lsb;
}
