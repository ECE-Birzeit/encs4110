#include "TM4C123.h"

#define RED 0x02
#define BLUE 0x04
#define GREEN 0x08
#define YELLOW RED + GREEN
#define MAGENTA BLUE + RED
#define CYAN GREEN + BLUE
#define WHITE RED + GREEN + BLUE
#define SW1 0x10
#define SW2 0x01
#define DELAY 900000



const int sequence[] = {RED, BLUE, GREEN, YELLOW, MAGENTA, CYAN, WHITE};
int index = 0;

void delay( volatile unsigned long ulLoop ){
	for (ulLoop = 0; ulLoop < DELAY; ulLoop++) {
		for (ulLoop = 0; ulLoop < DELAY; ulLoop++) {
  		}
 	}
}

int main(void)
{
	SYSCTL->RCGCGPIO |= (1<<5);
	SYSCTL->RCGCTIMER |= (1<<1);
	delay(0);
	GPIOF->DIR |= BLUE;
	GPIOF->DEN |= BLUE;

	TIMER1->CTL = 0;        		// Disable the timer
	TIMER1->CFG = 0x4;       		// Choose 16-bit mode
	TIMER1->TAMR = 0x02;       		// Periodic mode
	TIMER1->TAPR = 256  - 1;	// Prescaler
	TIMER1->TAILR = 65536 - 1;  // Initial Value
	TIMER1->ICR = 0x1;           	// Clear Any Prior Interrupts
	TIMER1->IMR |=(1<<0);			// Enable Timeout Interrupt
	NVIC->ISER[0] |= (1<<21);
	TIMER1->CTL |= 0x01;          	// Enable the timer
    while(1)
    {
    }
}


void TIMER1A_Handler()
{
	if(TIMER1->MIS & 0x1) {
		GPIOF->DATA  ^= BLUE;
 		TIMER1->ICR = 0x1;
	}
}
