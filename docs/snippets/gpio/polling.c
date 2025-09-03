//Program 1 
#include "TM4C123.h" 

int main(void) { 
    unsigned int state; 
    SYSCTL->RCGCGPIO |= 0x20;   /* enable clock to GPIOF */ 
    GPIOF->LOCK = 0x4C4F434B;   // unlockGPIOCR register 
    GPIOF->CR = 0x01;           
    // Enable GPIOPUR register enable to commit 
    GPIOF->PUR |= 0x10;        
    GPIOF->DIR |= 0x02;          
    GPIOF->DEN |= 0x12;         
    // Enable Pull Up resistor PF4 
    //set PF1 as an output and PF4 as an input pin 
    // Enable PF1 and PF4 as a digital GPIO pins  
    while(1) {    
        state = GPIOF->DATA & 0x10; 
        if (state == 0) {
            GPIOF->DATA |= 0x2;
        } else {
            GPIOF->DATA &= ~0x2;
        }
    } 
} 