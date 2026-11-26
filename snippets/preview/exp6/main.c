// #region full
// #region defines
#include "TM4C123.h"

#define RCGC_GPIOF   (1U << 5)          // RCGCGPIO bit for Port F

// Port F pins
#define SW2          (1U << 0)          // PF0, locked at reset (it can be the NMI input)
#define RED_LED      (1U << 1)          // PF1
#define BLUE_LED     (1U << 2)          // PF2
#define GREEN_LED    (1U << 3)          // PF3
#define SW1          (1U << 4)          // PF4
#define LEDS         (RED_LED | BLUE_LED | GREEN_LED)
#define SWITCHES     (SW1 | SW2)

#define GPIO_UNLOCK  0x4C4F434B         // "LOCK" in ASCII: written to GPIOF->LOCK to unlock CR

// The seven colours, in the order SW1 steps through them. uint8_t: each is a pattern
// for Port F's 8 pins, so it fits in a byte.
const uint8_t colors[7] = {
	RED_LED, RED_LED | GREEN_LED, GREEN_LED, GREEN_LED | BLUE_LED,
	BLUE_LED, RED_LED | BLUE_LED, LEDS
};

volatile uint32_t color_index;          // Which colour is selected: 0 to 6
volatile uint32_t presses;              // SW1 interrupts so far: watch it for bounce
// #endregion defines


// #region handler
// Runs on every falling edge of SW1: the press. It only changes the state;
// the main loop decides what the LED shows.
void GPIOF_Handler(void)
{
	if (GPIOF->MIS & SW1)				// Port F has one handler for all its pins: check which
	{
		color_index = (color_index + 1) % 7;	// Next colour, back to red after white
		presses++;
		GPIOF->ICR = SW1;				// Clear SW1's flag (write 1 to clear)
	}
}
// #endregion handler


// #region main
int main(void)
{
	SYSCTL->RCGCGPIO |= RCGC_GPIOF;				// Clock to Port F
	while ((SYSCTL->PRGPIO & RCGC_GPIOF) == 0);	// Wait until it's ready

	GPIOF->LOCK = GPIO_UNLOCK;					// Unlock PF0...
	GPIOF->CR  |= SW2;							// ...so its settings can be changed
	GPIOF->LOCK = 0;							// Lock again

	GPIOF->DIR = (GPIOF->DIR | LEDS) & ~SWITCHES;	// LEDs out, switches in
	GPIOF->PUR |= SWITCHES;						// Pull-ups: a switch reads 1, and 0 when pressed
	GPIOF->DEN |= LEDS | SWITCHES;

	// SW1 interrupts on its falling edge. SW2 doesn't interrupt: the loop polls it.
	GPIOF->IS  &= ~SW1;							// Edge-sensitive
	GPIOF->IBE &= ~SW1;							// One edge only...
	GPIOF->IEV &= ~SW1;							// ...the falling edge: the press
	GPIOF->ICR  = SW1;							// Clear any edge seen during setup
	GPIOF->IM  |= SW1;							// Let SW1 interrupt
	NVIC_EnableIRQ(GPIOF_IRQn);					// Port F is interrupt 30

	while (1)
	{
		if ((GPIOF->DATA & SW2) == 0)			// Polled: is SW2 held right now?
			GPIOF->DATA &= ~LEDS;				// Held: dark
		else									// Released: show the selected colour
			GPIOF->DATA = (GPIOF->DATA & ~LEDS) | colors[color_index];
	}
}
// #endregion main
// #endregion full
