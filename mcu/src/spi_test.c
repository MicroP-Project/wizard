// spi_test.c
// Ellen Yu & Jessica Li
// ellyu@g.hmc.edu & jesli@hmc.edu
// Nov 14, 2026
// Test for the MCU -> FPGA SPI3 link.
// Hold the button and swing the wand: when a +x cast (user_input == 1) is detected,
// send a packet with the "right" flag set. fpga/src/fpga_spi_test.sv lights an LED for 1 s.
//
// This file has its own main(): exclude main.c from the Segger build while testing
// (or exclude this file when building the real game).

#include <stdio.h>
#include "main.h"        // clock, GPIO, accelerometer, BUTTON_PIN, THRESH_X_Y
#include "fpga_spi.h"    // initFPGA(), fpgaSendRound()

// Redirect printf to the debugger's SWO/ITM console.
// NOTE: SWO is on PB3, which is now SPI3 SCK, so prints may not show up after initFPGA().
int _write(int file, char *ptr, int len) {
  for (int i = 0; i < len; i++) ITM_SendChar(*ptr++);
  return len;
}

// Returns 1 only if the button reads high for the whole debounce window
static int buttonPressed(void) {
    for (int i = 0; i < 10000; i++) {
        if (!digitalRead(BUTTON_PIN)) return 0;
    }
    return 1;
}

// Same direction encoding as main.c (1 = +x, ..., 6 = -z, 0 = none)
static int readCast(void) {
    accel_t a;
    accelReadMg(&a);
    if (a.x >  THRESH_X_Y)     return 1;
    if (a.x < -THRESH_X_Y)     return 2;
    if (a.y >  THRESH_X_Y)     return 3;
    if (a.y < -THRESH_X_Y)     return 4;
    if (a.z >  THRESH_Z_UP)    return 5;
    if (a.z < -THRESH_Z_DOWN)  return 6;
    return 0;
}

int main(void) {
    configureFlash();
    configureClock();

    gpioEnable(GPIO_PORT_A);
    gpioEnable(GPIO_PORT_B);
    pinMode(BUTTON_PIN, GPIO_INPUT);

    int accel_ok = initAccel();   // SPI1 (PA5/6/7, CS PA10)
    printf("accel ok: %d\n", accel_ok);

    initFPGA();                   // SPI3 (PB3/4/5, CS PA8)

    while (1) {
        if (!buttonPressed()) continue;

        // sample until a direction is detected or the button is released
        int user_input = 0;
        while (user_input == 0 && digitalRead(BUTTON_PIN)) {
            user_input = readCast();
        }

        if (user_input == 1) {
            fpgaSendRound(FPGA_DIR_NONE, FPGA_FLAG_RIGHT);   // packet = 0x01 -> FPGA LED on for 1 s
        }
        printf("user_input: %d\n", user_input);

        // wait for button release so one press = one cast
        while (digitalRead(BUTTON_PIN));
    }
}
