#include "TM4C123.h"   

#define GREEN_LED 0x08
#define SW1 0x10
#define SW2 0x01

int main(void) 
{ 
    SYSCTL->RCGCGPIO |= (1<<5);             // Enable clock to GPIOF

    GPIOF->LOCK = 0x4C4F434B;               // unlock commit register
    GPIOF->CR = 0x01;                       // make PORTF0 configurable


    // PINS Configurations
    GPIOF->DIR &= ~(SW1 | SW2);             // Set SW1, SW2 as input pins
    GPIOF->DIR |= GREEN_LED;                // Set GREEN_LED as output pin

    GPIOF->DEN |= (GREEN_LED | SW1 | SW2);  // Enable digital function for GREEN_LED, SW1, SW2
    GPIOF->PUR |= (SW1 | SW2);              // Enable pull-up resistors on SW1, SW2

    // Interrupt Configurations
    GPIOF->IS  &= ~(SW1 | SW2);             // SW1, SW2 are edge-sensitive
    GPIOF->IBE &= ~(SW1 | SW2);             // SW1, SW2 are not both edges
    GPIOF->IEV &= ~(SW1 | SW2);             // falling edge trigger
    GPIOF->ICR |= (SW1 | SW2);              // clear any prior interrupt
    GPIOF->IM  |= (SW1 | SW2);              // unmask interrupt

    // Enable Interrupts
    NVIC->ISER[0] |= (1<<30);       				// enable interrupt 30 in NVIC
    // NVIC_EnableIRQ(GPIOF_IRQn);  				// Alternative way using CMSIS function
    while(1) 
    { 
    } 
} 

void GPIOF_Handler(void) {  
    if (GPIOF->MIS & SW1) {                 // check if interrupt causes by PF4/SW1
        GPIOF->DATA |= GREEN_LED;           // turn on green LED
        GPIOF->ICR |= SW1;                  // clear the interrupt flag
    }  
    else if (GPIOF->MIS & SW2) {            // check if interrupt causes by PF0/SW2
        GPIOF->DATA &= ~GREEN_LED;          // turn off green LED
        GPIOF->ICR |= SW2;                  // clear the interrupt flag
    } 
} 