


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

volatile int time_up;

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
    
    initTRNG();

    RCC->APB1ENR1 |= (1 << 0); // TIM2EN
    initTIM(TIM2);

    // Enable interrupt for TIM2 
    NVIC->ISER[0] |= (1 << 28);

    TIM2->ARR = 20000;// Set timer max count (2s at 10 kHz)
    TIM2->EGR |= 1;     // Force update
    TIM2->SR &= ~(0x1); // Clear UIF
    TIM2->DIER |= TIM_DIER_UIE;

    // enable interrupt globally
    __enable_irq();


    volatile int en = 0;
    volatile int debounce_en = 0;
    volatile int required_spell = nextSpellGenerator(); // generating next move 1-4

    int valid_input = 0;
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

            //printf("x: %d \n", x_only);
            //printf("y: %d \n", y_only);
            //printf("z: %d \n", z_only);

            // 5 -> +x
            // 6 -> -x
            // 1 -> +y
            // 2 -> -y
            // 3 -> +z
            // 4 -> -z
            //if ((x_only > THRESH_X_Y) & ((x_only * x_only + y_only * y_only + z_only * z_only) > 2000000)){
            //    user_input = 5; // x direction
            //    printf("user_input: %d \n", user_input);
            //    // SPI write to the FPGA with the specific value
            //    break;
            //    }
            //else if ((x_only < -THRESH_X_Y) & ((x_only * x_only + y_only * y_only + z_only * z_only) > 2000000)){
            //    user_input = 6;
            //    printf("user_input: %d \n", user_input);
            //    break;
            //}
            //else 
            if ((y_only > THRESH_X_Y) & ((x_only * x_only + y_only * y_only + z_only * z_only) > 2000000)){
                user_input = 1;
                valid_input = 1;
                printf("user_input: %d \n", user_input);
                break;
            }
            else if ((y_only < -THRESH_X_Y) & ((x_only * x_only + y_only * y_only + z_only * z_only) > 2000000)){
                user_input = 2;
                valid_input = 1;
                printf("user_input: %d \n", user_input);
                break;
            }
            else if (z_only > THRESH_Z_UP){
                user_input = 3;
                valid_input = 1;
                printf("user_input: %d \n", user_input);
                break;
            }
            else if (z_only < -THRESH_Z_DOWN){
                user_input = 4;
                valid_input = 1;
                printf("user_input: %d \n", user_input);
                break;
            }else{
                user_input = 0;
            }
            

        }
        
        if(valid_input){
          valid_input = 0;
            // take in the user input and check whether the input match the required spell
          if (user_input == required_spell){
            // take next spell
            required_spell = nextSpellGenerator(); // generating next move 1 to 4
            // spiwrite (correct, wrong, next spell, time up)
          } else {
            // spiwrite (correct, wrong, next spell, time up = 0)
          }
            
            
            
        }

        // wait for button to be released
        while(en){
            en = digitalRead(BUTTON_PIN);
            if (time_up){
              break;
            }
        }

        if (time_up){
            required_spell = nextSpellGenerator(); // generating next move
            // spiwrite (correct, wrong, next spell, time up)
        }
    }
}


void TIM2_IRQHandler(void){
    if (TIM2->SR & TIM_SR_UIF) {      // was it the overflow?
        TIM2->SR &= ~TIM_SR_UIF;
        time_up = 1;
    }
} 
