#include "TM4C123.h" 

#define RED_LED 0x02
#define SWITCH 0x10

int main(void) { 
    unsigned int state;      
    SYSCTL->RCGCGPIO |= (1<<5); 			// Enable Port F
    GPIOF->PUR |= SWITCH;                   // Enable pull-up resistor on PF4
    GPIOF->DIR |= RED_LED;                  // Set PF1 as an output pin and PF4 as an input pin
    GPIOF->DEN |= (RED_LED | SWITCH);       // Enable PF1 and PF4 as a digital GPIO pins
    while(1) {    
        state = GPIOF->DATA & SWITCH;       // Read the state of the switch
        if (state == 0) {                   // If the switch is pressed (since it's Pull-up)
            GPIOF->DATA |= RED_LED;         // Turn on the LED
        } else {                            // If the switch is not pressed
            GPIOF->DATA &= ~RED_LED;        // Turn off the LED
        }
    } 
} 