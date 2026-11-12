#include "TM4C123.h"

#define BLUE_LED  (1 << 2)              // PF2 = T1CCP0 (Timer1A's PWM pin)
#define PERIOD    50000                 // 1 kHz at 50 MHz: 50,000,000 / 1,000
#define DUTY      25                    // Percent of each period the LED is on


int main(void)
{
	SYSCTL->RCGCGPIO  |= (1 << 5);		// Enable clock to GPIOF
	SYSCTL->RCGCTIMER |= (1 << 1);		// Enable clock to Timer1

	__asm__("NOP");						// Wait for clock stabilization
	__asm__("NOP");
	__asm__("NOP");

	// PF2 is driven by Timer1A, not by GPIOF->DATA
	GPIOF->AFSEL |= BLUE_LED;						// Alternate function on PF2
	GPIOF->PCTL   = (GPIOF->PCTL & ~0x00000F00)		// PF2's field is bits 11:8
	              | 0x00000700;						// 7 = T1CCP0
	GPIOF->DEN   |= BLUE_LED;						// PF2 is a digital pin

	TIMER1->CTL      &= ~(1 << 0);		// Disable Timer1A while configuring
	TIMER1->CFG       = 0x4;			// 16-bit mode (PWM uses a split timer)
	TIMER1->TAMR      = 0x0A;			// TAAMS = 1 (PWM), TACMR = 0, TAMR = 2 (periodic)
	TIMER1->TAPR      = 0;				// No extension: the period fits in 16 bits
	TIMER1->TAILR     = PERIOD - 1;		// Period: TAILR + 1 counts
	TIMER1->TAMATCHR  = (PERIOD - 1) - (PERIOD / 100) * DUTY;	// Pin goes low here
	TIMER1->CTL      |= (1 << 0);		// Enable Timer1A: the pin starts toggling

	while (1)							// The timer drives the pin by itself:
	{									// the CPU has nothing left to do
	}
}
