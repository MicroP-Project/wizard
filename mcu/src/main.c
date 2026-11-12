


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"

int main(void) {
    configureClock();

    gpioEnable(GPIO_PORT_A); // enabling PA pins
    gpioEnable(GPIO_PORT_B); // enabling PB pins

    // Button input initialization (for accelerometer sampling enable)
    pinMode(BUTTON_PIN, GPIO_INPUT);

    // initialize spi for accelerometer (which also calls initSPI for MCU)
    initAccel(); // assumes 80 Mhz, which is how system clock is configured in RCC
    /*Enables GPIOB, initializes SPI1 in Mode 3 at 2.5 MHz, verifies the sensor responds with 0x33 on WHO_AM_I, and configures the LIS3DH for 100 Hz output, X/Y/Z enabled, 12-bit high-resolution mode, $\pm 2g$ range, and Block Data Update (returns 1 on success, 0 on failure). */
    
    volatile int en;
    int user_input;
    while (1){
        en = digitalRead(BUTTON_PIN);
        while (en){

            accel_t accel_mg;
            accelReadMg(&accel_mg);
            int16_t x_only = accel_mg.x;
            int16_t y_only = accel_mg.y;
            int16_t z_only = accel_mg.z;
            /*
            if (x_only > THRESH_X_Y){
                user_input = 01; // x direction
                break;
                }
            else if (x_only > THRESH_X_Y)
            // read x
            // read y
            // read z
            */
            en = digitalRead(BUTTON_PIN);
        }

    }
}