// fpga_spi.c
// Ellen Yu & Jessica Li
// ellyu@g.hmc.edu & jesli@hmc.edu
// Nov 14, 2026
// Driver for sending game state to the FPGA over SPI3.
// See fpga_spi.h for wiring and packet format.

#include "fpga_spi.h"

///////////////////////////////////////////////////////////////////////////////
// Chip select helpers
///////////////////////////////////////////////////////////////////////////////

static inline void fpgaSelect(void)   { digitalWrite(FPGA_CS_PIN, PIO_LOW);  }
static inline void fpgaDeselect(void) {
    // Make sure the last bit has fully shifted out before releasing CS,
    // otherwise the FPGA latches the packet one bit short.
    while (SPI3->SR & SPI_SR_BSY);
    digitalWrite(FPGA_CS_PIN, PIO_HIGH);
}

///////////////////////////////////////////////////////////////////////////////
// Setup
///////////////////////////////////////////////////////////////////////////////

void initFPGA(void) {
    // initSPI3() enables GPIOA/GPIOB, sets PB3/4/5 to AF6, and leaves CS high.
    initSPI3(FPGA_SPI_BR, FPGA_SPI_CPOL, FPGA_SPI_CPHA);
    digitalWrite(FPGA_CS_PIN, PIO_HIGH);
}

///////////////////////////////////////////////////////////////////////////////
// Sending
///////////////////////////////////////////////////////////////////////////////

void fpgaSendPacket(uint8_t byte0, uint8_t byte1) {
    fpgaSelect();
    spi3SendReceive(byte0);   // received byte (FPGA's garbage sdo) is ignored
    spi3SendReceive(byte1);
    fpgaDeselect();
}

uint8_t fpgaPackStatus(uint8_t state, uint8_t direction, uint8_t lives) {
    return (uint8_t)(((state     & FPGA_STATE_MSK) << FPGA_STATE_SHIFT) |
                     ((direction & FPGA_DIR_MSK)   << FPGA_DIR_SHIFT)   |
                     ((lives     & FPGA_LIVES_MSK) << FPGA_LIVES_SHIFT));
}

void fpgaSendState(uint8_t state, uint8_t direction, uint8_t lives, uint8_t score) {
    fpgaSendPacket(fpgaPackStatus(state, direction, lives), score);
}
