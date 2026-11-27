#include "TM4C123.h"

#define RCGC_GPIOF      (1U << 5)       // RCGCGPIO bit for Port F
#define RCGC_TIMER1     (1U << 1)       // RCGCTIMER bit for Timer1
#define BLUE_LED        (1U << 2)       // PF2

// Timer1 register values
#define CFG_16BIT       0x4             // CFG: two 16-bit timers
#define TAMR_PERIODIC   0x2             // TAMR: periodic, counting down
#define CTL_TAEN        (1U << 0)       // CTL: Timer A enable
#define TATO            (1U << 0)       // IMR, MIS, ICR: Timer A time-out
#define TAPR_MAX        0xFF            // Largest 8-bit prescaler: divide by 256
#define TAILR_MAX       0xFFFF          // Largest 16-bit load value
#define TIMER1A_IRQ_BIT (1U << 21)      // Timer1A is interrupt 21: bit 21 of NVIC ISER0

int main(void)
{
	SYSCTL->RCGCGPIO  |= RCGC_GPIOF;                // Clocks to Port F and Timer1
	SYSCTL->RCGCTIMER |= RCGC_TIMER1;
	while ((SYSCTL->PRGPIO  & RCGC_GPIOF)  == 0);   // Wait until both are ready
	while ((SYSCTL->PRTIMER & RCGC_TIMER1) == 0);

	GPIOF->DIR |= BLUE_LED;                         // Blue LED out
	GPIOF->DEN |= BLUE_LED;

	TIMER1->CTL  &= ~CTL_TAEN;                      // Stop the timer while configuring
	TIMER1->CFG   = CFG_16BIT;                      // 16-bit mode
	TIMER1->TAMR  = TAMR_PERIODIC;                  // Periodic
	TIMER1->TAPR  = TAPR_MAX;                       // Prescaler: divide by 256
	TIMER1->TAILR = TAILR_MAX;                      // Count 65,536 prescaled ticks
	TIMER1->ICR   = TATO;                           // Clear any earlier time-out
	TIMER1->IMR  |= TATO;                           // Interrupt on time-out
	NVIC->ISER[0] = TIMER1A_IRQ_BIT;                // Enable interrupt 21 in the NVIC
	TIMER1->CTL  |= CTL_TAEN;                       // Start

	while (1)
	{
	}
}

void TIMER1A_Handler(void)
{
	if (TIMER1->MIS & TATO)                         // A time-out
	{
		GPIOF->DATA ^= BLUE_LED;                    // Toggle the blue LED
		TIMER1->ICR = TATO;                         // Clear the flag
	}
}
