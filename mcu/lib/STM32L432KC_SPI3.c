// STM32L432KC_SPI3.c
// Ellen Yu & Jessica Li
// ellyu@g.hmc.edu & jesli@hmc.edu
// Nov 14, 2026
// Code for SPI3 (used for comms with the FPGA)

#include "STM32L432KC_SPI3.h"

void initSPI3(int br, int cpol, int cpha){
    // turn on the clocks for GPIOB (SPI3 pins) and GPIOA (CS pin)
    gpioEnable(GPIO_PORT_A);
    gpioEnable(GPIO_PORT_B);

    // turn on the clock for SPI3 (SPI3 is on APB1, unlike SPI1 which is on APB2)
    RCC->APB1ENR1 |= (RCC_APB1ENR1_SPI3EN);

    // set the SPI GPIOs to Alternate Function Mode
    pinMode(SPI3_CS_PIN, GPIO_OUTPUT);
    pinMode(SPI3_SCK_PIN, GPIO_ALT);
    pinMode(SPI3_MISO_PIN, GPIO_ALT);
    pinMode(SPI3_MOSI_PIN, GPIO_ALT);

    // choose AF6 (SPI3) -- note SPI3 is AF6, not AF5 like SPI1.
    // Clear each field first: PB3/PB4 reset to AF0 (debug pins), and OR-ing
    // 0b0110 on top of a leftover value would select the wrong function.
    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL3 | GPIO_AFRL_AFSEL4 | GPIO_AFRL_AFSEL5);
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL3, 0b0110);
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL4, 0b0110);
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL5, 0b0110);


    // configure baud rate, CPOL and CPHA
    SPI3->CR1 =0;
    SPI3->CR1 |= _VAL2FLD(SPI_CR1_BR, br);
    SPI3->CR1 |= _VAL2FLD(SPI_CR1_CPOL, cpol);
    SPI3->CR1 |= _VAL2FLD(SPI_CR1_CPHA, cpha);

    // set to master mode
    SPI3->CR1 |= _VAL2FLD(SPI_CR1_MSTR, 0b1);

    // 8-bit data, NSS output enabled, RXNE fires after 8 bits
    SPI3->CR2 |= _VAL2FLD(SPI_CR2_DS, 0b0111);
    SPI3->CR2 |= _VAL2FLD(SPI_CR2_SSOE, 1);
    SPI3->CR2 |= _VAL2FLD(SPI_CR2_FRXTH, 1);

    // MSB first
    SPI3->CR1 |= _VAL2FLD(SPI_CR1_LSBFIRST, 0b0);

    // configure NSS pin
    SPI3->CR1 |= _VAL2FLD(SPI_CR1_SSM, 0b0);

    // need to set a GPIO as output and manually toggle
        // in this case, used GPIO SPI3_CS_PIN (PA8)

    // enable SPI
    SPI3->CR1 |= _VAL2FLD(SPI_CR1_SPE, 0b1);

    // CS idles HIGH so the FPGA doesn't see a frame until we start one
    digitalWrite(SPI3_CS_PIN, 1);

}

char spi3SendReceive(char send){
    // wait for transmit buffer to be empty
    while((SPI3->SR & SPI_SR_TXE) == 0);
    *((volatile char *) (&SPI3->DR)) = send;
    // always read DR back (even if MISO is unconnected) so the RX FIFO doesn't overflow
    while(!(SPI3->SR & SPI_SR_RXNE));
    char rec = *((volatile char *)(&SPI3->DR));
    return rec;
}
