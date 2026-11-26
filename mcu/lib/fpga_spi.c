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

// Start a packet: pull CS low so the FPGA starts shifting in bits.
void fpgaSelect(void) {
    digitalWrite(FPGA_CS_PIN, PIO_LOW);
}

// End a packet: wait for the last bit to finish, then pull CS high.
// The FPGA saves the packet on this rising edge of CS.
void fpgaDeselect(void) {
    // BSY = 1 while SPI3 is still sending. If CS went high before the last
    // bit finished, the FPGA would save the packet one bit short.
    while ((SPI3->SR & SPI_SR_BSY) != 0) {
        // do nothing, just wait
    }
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

void fpgaSendPacket(uint8_t packet) {
    fpgaSelect();
    spi3SendReceive(packet);   // received byte (FPGA's garbage sdo) is ignored
    fpgaDeselect();
}

void fpgaSendRound(uint8_t next_spell, uint8_t result) {
    uint8_t spell_bits = (next_spell & 0b111) << 3;   // bits 5-3
    uint8_t flag_bits  = (result     & 0b111);        // bits 2-0
    fpgaSendPacket(spell_bits | flag_bits);
}
