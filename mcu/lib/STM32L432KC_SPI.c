// STM32L432KC_SPI.c
// Ellen Yu
// ellyu@g.hmc.edu
// Oct. 8 2026
// Code for SPI

#include "STM32L432KC_SPI.h"

void initSPI(int br, int cpol, int cpha){
      // now configure the GPIOs
    
    // turn on the clock for SPI
    RCC->APB2ENR |= (RCC_APB2ENR_SPI1EN);
    // set the SPI GPIOs to Alternate Function Mode
    pinMode(OUTPUT_PIN, GPIO_OUTPUT);
    pinMode(SCK_PIN, GPIO_ALT);
    pinMode(MISO_PIN, GPIO_ALT);
    pinMode(MOSI_PIN, GPIO_ALT);

    // choose AF5 (SPI)
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL3, 0b0101);
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL4, 0b0101);
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL5, 0b0101);

    
    // configure baud rate, CPOL and CPHA
    SPI1->CR1 =0;
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_BR, br);
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPOL, cpol);
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPHA, cpha);

    // set to master mode
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_MSTR, 0b1);

    SPI1->CR2 |= _VAL2FLD(SPI_CR2_DS, 0b0111);
    SPI1->CR2 |= _VAL2FLD(SPI_CR2_SSOE, 1);
    SPI1->CR2 |= _VAL2FLD(SPI_CR2_FRXTH, 1);

    SPI1->CR1 |= _VAL2FLD(SPI_CR1_LSBFIRST, 0b0);

    // configure NSS pin
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_SSM, 0b0);
    // need to set a GPIO as output and manually toggle 
        // in this case, used GPIO PB0

    

    // enable SPI
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_SPE, 0b1);



    digitalWrite(OUTPUT_PIN, 0);

    
    

}

char spiSendReceive(char send){
    // wait for transmit buffer to be empty
    //digitalWrite(OUTPUT_PIN, PIO_HIGH);
    while((SPI1->SR & SPI_SR_TXE) == 0);
    *((volatile char *) (&SPI1->DR)) = send;
    while(!(SPI1->SR & SPI_SR_RXNE));
    char rec = *((volatile char *)(&SPI1->DR));
    //digitalWrite(OUTPUT_PIN, PIO_LOW);
    return rec;
}
