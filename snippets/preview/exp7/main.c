// #region full
// #region defines
#include "TM4C123.h"

// Clocks
#define RCGC_GPIOF     (1U << 5)        // RCGCGPIO bit for Port F
#define RCGC_TIMER1    (1U << 1)        // RCGCTIMER bit for Timer1

// Port F pins: the three LEDs are one RGB LED, so only one colour is lit at a time
#define RED_LED        (1U << 1)        // PF1
#define BLUE_LED       (1U << 2)        // PF2
#define GREEN_LED      (1U << 3)        // PF3
#define LEDS           (RED_LED | BLUE_LED | GREEN_LED)
#define SW1            (1U << 4)        // PF4

// Timer1 register values
#define CFG_32BIT      0x0              // CFG: one 32-bit timer
#define TAMR_PERIODIC  0x2              // TAMR: periodic, counting down (TAILD = 0)
#define CTL_TAEN       (1U << 0)        // CTL: Timer A enable
#define TATO           (1U << 0)        // IMR, MIS, ICR: Timer A time-out

// Times
#define HEARTBEAT_MS   500              // The LED turns on or off every 500 ms: a 1 Hz blink
#define HEARTBEAT_LOAD ((SystemCoreClock / 1000) * HEARTBEAT_MS - 1)   // 25,000,000 - 1 at 50 MHz
#define DEBOUNCE_MS    20               // How long SW1 must stay up after a release

// The colours SW1 steps through. uint8_t: each is a pattern for Port F's 8 pins.
const uint8_t colors[3] = { RED_LED, GREEN_LED, BLUE_LED };

volatile uint32_t ms;                   // Milliseconds since reset, from SysTick
volatile uint32_t up_ms;                // How long SW1 has been up, in ms
volatile uint32_t presses;              // Presses, after debouncing
volatile uint32_t color_index;          // The colour now: 0 red, 1 green, 2 blue
volatile uint32_t led_on;               // 1 while the heartbeat has the LED on
// #endregion defines


// #region handlers
// Light the current colour, or nothing, depending on the heartbeat.
// Both Port F and Timer1A interrupts call it. They have the same priority, so one never
// interrupts the other halfway through this write.
void show(void)
{
	uint32_t led = led_on ? colors[color_index] : 0;
	GPIOF->DATA = (GPIOF->DATA & ~LEDS) | led;
}

// Every 1 ms: count the time, and wait for SW1's release. Once SW1 has been up for
// DEBOUNCE_MS in a row, listen for the next press.
void SysTick_Handler(void)
{
	ms++;

	if ((GPIOF->DATA & SW1) == 0)			// SW1 is down (or bouncing)
		up_ms = 0;
	else if (++up_ms == DEBOUNCE_MS)		// Up, and quiet for DEBOUNCE_MS
	{
		GPIOF->ICR = SW1;					// Forget the release bounces
		GPIOF->IM |= SW1;					// Listen for the next press
	}
}

// SW1's falling edge: a press. Act on it, then ignore SW1 until it's released.
void GPIOF_Handler(void)
{
	GPIOF->ICR = SW1;						// Clear the flag
	GPIOF->IM &= ~SW1;						// Ignore the bounces: SysTick turns this back on
	up_ms = 0;

	color_index = (color_index + 1) % 3;	// Next colour
	presses++;
	led_on = 1;								// Show it now, whatever the heartbeat was doing...
	show();
	TIMER1->TAILR = HEARTBEAT_LOAD;			// ...and restart the heartbeat: a full 500 ms on
}

// Every HEARTBEAT_MS: on, off, on, off...
void TIMER1A_Handler(void)
{
	TIMER1->ICR = TATO;						// Clear the time-out flag
	led_on = !led_on;
	show();
}
// #endregion handlers


// #region main
int main(void)
{
	SYSCTL->RCGCGPIO  |= RCGC_GPIOF;			// Clocks to Port F and Timer1
	SYSCTL->RCGCTIMER |= RCGC_TIMER1;
	while ((SYSCTL->PRGPIO  & RCGC_GPIOF)  == 0);
	while ((SYSCTL->PRTIMER & RCGC_TIMER1) == 0);

	// LEDs out; SW1 in, with pull-up and an interrupt on its falling edge (a press)
	GPIOF->DIR  = (GPIOF->DIR | LEDS) & ~SW1;
	GPIOF->PUR |= SW1;
	GPIOF->DEN |= LEDS | SW1;
	GPIOF->IS  &= ~SW1;							// Edge...
	GPIOF->IBE &= ~SW1;							// ...one direction...
	GPIOF->IEV &= ~SW1;							// ...falling
	GPIOF->ICR  = SW1;
	GPIOF->IM  |= SW1;
	NVIC_EnableIRQ(GPIOF_IRQn);

	// SysTick: interrupt every 1 ms. After Port F: its handler reads SW1
	SysTick_Config(SystemCoreClock / 1000);

	// Timer1A: 32-bit periodic, interrupt every HEARTBEAT_MS
	TIMER1->CTL  &= ~CTL_TAEN;					// Stop it while configuring
	TIMER1->CFG   = CFG_32BIT;
	TIMER1->TAMR  = TAMR_PERIODIC;
	TIMER1->TAILR = HEARTBEAT_LOAD;
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
