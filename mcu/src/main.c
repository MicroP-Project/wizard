


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"

// Redirect printf to the debugger's SWO/ITM console (same as lab 5)
int _write(int file, char *ptr, int len) {
  int i = 0;
  for (i = 0; i < len; i++) {
    ITM_SendChar((*ptr++));
  }
  return len;
}

int main(void) {
    configureFlash();
    configureClock();

    gpioEnable(GPIO_PORT_A); // enabling PA pins
    gpioEnable(GPIO_PORT_B); // enabling PB pins

    // Button input initialization (for accelerometer sampling enable)
    pinMode(BUTTON_PIN, GPIO_INPUT);

    // initialize spi for accelerometer (which also calls initSPI for MCU)
    initAccel(); // assumes 80 Mhz, which is how system clock is configured in RCC
    /*Enables GPIOB, initializes SPI1 in Mode 3 at 2.5 MHz, verifies the sensor responds with 0x33 on WHO_AM_I, and configures the LIS3DH for 100 Hz output, X/Y/Z enabled, 12-bit high-resolution mode, $\pm 2g$ range, and Block Data Update (returns 1 on success, 0 on failure). */
    
    volatile int en;
    volatile int debounce_en;

    int user_input;
    while (1){

        debounce_en = 0;
        if (digitalRead(BUTTON_PIN)){
            for (int i = 0; i < 10000; i++) {
                en = digitalRead(BUTTON_PIN);
                if (!en){
                    debounce_en = 0;
                    break;
                }
                else if (i == 9999){
                    debounce_en = 1;
                }
            }
        }

        while (debounce_en){
            accel_t accel_mg;
            accelReadMg(&accel_mg);
            int16_t x_only = accel_mg.x;
            int16_t y_only = accel_mg.y;
            int16_t z_only = accel_mg.z;


            // test for x values: printf("x: %d \n", x_only);

            // 1 -> +x
            // 2 -> -x
            // 3 -> +y
            // 4 -> -y
            // 5 -> +z
            // 6 -> -z
            if (x_only > THRESH_X_Y){
                user_input = 1; // x direction
                break;
                }
            else if (x_only < -THRESH_X_Y){
                user_input = 2;
                break;
            }
            else if (y_only > THRESH_X_Y){
                user_input = 3;
                break;
            }
            else if (y_only < -THRESH_X_Y){
                user_input = 4;
                break;
            }
            else if (z_only > THRESH_Z_UP){
                user_input = 5;
                break;
            }
            else if (z_only < -THRESH_Z_DOWN){
                user_input = 6;
                break;
            }else{
                user_input = 0;
            }
            
        }
        // print user_input onto screen
        // (only if the button was pressed; otherwise this would print on every loop)
        if (en){
            printf("user_input: %d \n", user_input);
        }

        // wait for button to be released
        while(en){
            en = digitalRead(BUTTON_PIN);
        }


    }
}