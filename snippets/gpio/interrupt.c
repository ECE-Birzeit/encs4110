#include "TM4C123.h"

#define RCGC_GPIOF    (1U << 5)         // RCGCGPIO bit for Port F
#define GREEN_LED     (1U << 3)         // PF3
#define SW1           (1U << 4)         // PF4
#define SW2           (1U << 0)         // PF0 (locked at reset)
#define SWITCHES      (SW1 | SW2)

#define GPIO_UNLOCK   0x4C4F434B        // "LOCK" in ASCII: written to LOCK to unlock CR
#define GPIOF_IRQ_BIT (1U << 30)        // Port F is interrupt 30: bit 30 of NVIC ISER0

int main(void)
{
	SYSCTL->RCGCGPIO |= RCGC_GPIOF;                 // Clock to Port F
	while ((SYSCTL->PRGPIO & RCGC_GPIOF) == 0);     // Wait until Port F is ready

	GPIOF->LOCK = GPIO_UNLOCK;                      // Unlock the commit register
	GPIOF->CR  |= SW2;                              // Make PF0 configurable
	GPIOF->LOCK = 0;

	// Pins
	GPIOF->DIR  = (GPIOF->DIR | GREEN_LED) & ~SWITCHES;    // LED out, switches in
	GPIOF->DEN |= GREEN_LED | SWITCHES;
	GPIOF->PUR |= SWITCHES;                         // Pull-ups: pressed reads 0

	// Interrupts on the falling edge (the press)
	GPIOF->IS  &= ~SWITCHES;                        // Edge-sensitive
	GPIOF->IBE &= ~SWITCHES;                        // One edge only...
	GPIOF->IEV &= ~SWITCHES;                        // ...the falling one
	GPIOF->ICR  = SWITCHES;                         // Clear any earlier edge
	GPIOF->IM  |= SWITCHES;                         // Unmask

	NVIC->ISER[0] = GPIOF_IRQ_BIT;                  // Enable interrupt 30 in the NVIC
	// NVIC_EnableIRQ(GPIOF_IRQn);                  // The same, with the CMSIS function

	while (1)
	{
	}
}

void GPIOF_Handler(void)
{
	if (GPIOF->MIS & SW1)                           // SW1 caused it
	{
		GPIOF->DATA |= GREEN_LED;                   // Green on
		GPIOF->ICR = SW1;                           // Clear SW1's flag
	}
	if (GPIOF->MIS & SW2)                           // SW2 caused it (checked separately:
	{                                               // both flags can be set at once)
		GPIOF->DATA &= ~GREEN_LED;                  // Green off
		GPIOF->ICR = SW2;                           // Clear SW2's flag
	}
}
