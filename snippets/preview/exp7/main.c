// #region full
// #region defines
#include "TM4C123.h"

// Clocks
#define RCGC_GPIOF     (1U << 5)        // RCGCGPIO bit for Port F
#define RCGC_TIMER1    (1U << 1)        // RCGCTIMER bit for Timer1

// Port F pins
#define RED_LED        (1U << 1)        // PF1: toggled by SW1
#define BLUE_LED       (1U << 2)        // PF2: heartbeat from Timer1A
#define SW1            (1U << 4)        // PF4

// Timer1 register values
#define CFG_32BIT      0x0              // CFG: one 32-bit timer
#define TAMR_PERIODIC  0x2              // TAMR: periodic, counting down
#define CTL_TAEN       (1U << 0)        // CTL: Timer A enable
#define TATO           (1U << 0)        // IMR, MIS, ICR: Timer A time-out

// Times
#define HEARTBEAT_MS   500              // Blue LED toggles every 500 ms: 1 Hz blink
#define DEBOUNCE_MS    20               // Quiet time before an edge counts as a new press

volatile uint32_t ms;                   // Milliseconds since reset, from SysTick
volatile uint32_t last_edge;            // ms at SW1's last edge, rising or falling
volatile uint32_t edges;                // Every SW1 edge, bounces included
volatile uint32_t presses;              // Real presses, after debouncing
// #endregion defines


// #region handlers
// Every 1 ms
void SysTick_Handler(void)
{
	ms++;
}

// Every edge of SW1, both directions. A falling edge counts as a press only if the
// line was quiet for DEBOUNCE_MS before it; every edge restarts the quiet time.
void GPIOF_Handler(void)
{
	GPIOF->ICR = SW1;						// Clear the flag first

	if (ms - last_edge >= DEBOUNCE_MS		// The line was quiet...
	    && (GPIOF->DATA & SW1) == 0)		// ...and it's now pressed: a falling edge
	{
		GPIOF->DATA ^= RED_LED;
		presses++;
	}
	last_edge = ms;							// Press, release, or bounce: start the quiet time again
	edges++;
}

// Every HEARTBEAT_MS
void TIMER1A_Handler(void)
{
	TIMER1->ICR = TATO;						// Clear the time-out flag
	GPIOF->DATA ^= BLUE_LED;
}
// #endregion handlers


// #region main
int main(void)
{
	SYSCTL->RCGCGPIO  |= RCGC_GPIOF;			// Clocks to Port F and Timer1
	SYSCTL->RCGCTIMER |= RCGC_TIMER1;
	while ((SYSCTL->PRGPIO  & RCGC_GPIOF)  == 0);
	while ((SYSCTL->PRTIMER & RCGC_TIMER1) == 0);

	// LEDs out; SW1 in, with pull-up and an interrupt on both edges
	GPIOF->DIR  = (GPIOF->DIR | RED_LED | BLUE_LED) & ~SW1;
	GPIOF->PUR |= SW1;
	GPIOF->DEN |= RED_LED | BLUE_LED | SW1;
	GPIOF->IS  &= ~SW1;
	GPIOF->IBE |= SW1;							// Both edges: releases restart the quiet time too
	GPIOF->ICR  = SW1;
	GPIOF->IM  |= SW1;
	NVIC_EnableIRQ(GPIOF_IRQn);

	// SysTick: interrupt every 1 ms
	SysTick_Config(SystemCoreClock / 1000);

	// Timer1A: 32-bit periodic, interrupt every HEARTBEAT_MS
	TIMER1->CTL  &= ~CTL_TAEN;					// Stop it while configuring
	TIMER1->CFG   = CFG_32BIT;
	TIMER1->TAMR  = TAMR_PERIODIC;
	TIMER1->TAILR = (SystemCoreClock / 1000) * HEARTBEAT_MS - 1;	// 25,000,000 - 1 at 50 MHz
	TIMER1->ICR   = TATO;						// Clear any old time-out
	TIMER1->IMR  |= TATO;						// Interrupt on time-out
	NVIC_EnableIRQ(TIMER1A_IRQn);				// Timer1A is interrupt 21
	TIMER1->CTL  |= CTL_TAEN;					// Start

	while (1)									// All the work is in the three handlers
	{
	}
}
// #endregion main
// #endregion full
