#include "TM4C123.h"

#define BLUE_LED 0x04


int main(void)
{
	SYSCTL->RCGCGPIO |= (1<<5);			// Enable clock to GPIOF
	SYSCTL->RCGCTIMER |= (1<<1);		// Enable clock to Timer1

	__asm__("NOP");						// Wait for clock stabilization
	__asm__("NOP");						// Wait for clock stabilization
	__asm__("NOP");						// Wait for clock stabilization

	
	GPIOF->DIR |= BLUE_LED;				// Set blue LED as output pin
	GPIOF->DEN |= BLUE_LED;				// Enable digital function for blue LED

	TIMER1->CTL = 0;        			// Disable the timer
	TIMER1->CFG = 0x4;       			// Choose 16-bit mode
	TIMER1->TAMR = 0x02;       			// Periodic mode
	TIMER1->TAPR = 256  - 1;			// Set prescaler value
	TIMER1->TAILR = 65536 - 1;  		// Set initial reload value
	TIMER1->ICR = 0x1;           		// Clear any prior interrupts
	TIMER1->IMR |=(1<<0);				// Enable timeout interrupt
	NVIC->ISER[0] |= (1<<21);			// Enable Timer1A interrupt in NVIC
	TIMER1->CTL |= 0x01;          		// Enable the timer
	
    while(1)							// Main loop
    {
    }
}


void TIMER1A_Handler()
{
	if(TIMER1->MIS & 0x1) {				// Check if timer timeout interrupt occurred
		GPIOF->DATA  ^= BLUE_LED;		// Toggle blue LED
 		TIMER1->ICR = 0x1;				// Clear timer interrupt flag
	}
}
