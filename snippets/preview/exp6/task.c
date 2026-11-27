// Experiment 6 Task: two buttons, two colour sequences, one interrupt handler.
//   SW1 steps red, blue, green.   SW2 steps yellow, magenta, cyan.
// Provided: the pins, SW1's interrupt, the debouncing of both buttons, and main's loop.
// Write the four TODOs (highlighted).
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

// Debounce timing: a delay loop of about 20 ms at 50 MHz
#define F_CPU_HZ         50000000U
#define CYCLES_PER_LOOP  7U             // one pass of the volatile loop, about 7 cycles
#define DEBOUNCE_MS      20U
#define DEBOUNCE_LOOPS   (F_CPU_HZ / 1000U / CYCLES_PER_LOOP * DEBOUNCE_MS)

// Each button has its own colours and its own place in them
const uint8_t colors1[3] = { RED_LED, BLUE_LED, GREEN_LED };    // SW1: red, blue, green
// TODO 1: SW2's colours: yellow, magenta, cyan (mix two LEDs each) // [!code highlight]
const uint8_t colors2[3] = { 0, 0, 0 };  // [!code highlight]

volatile uint32_t index1;               // SW1's next colour in colors1: 0 to 2
volatile uint32_t index2;               // SW2's next colour in colors2: 0 to 2
volatile uint32_t led;                  // What the LED shows: the last press's colour (0 = off)


// ---------------------------------------------------------------- provided: don't change
// The debounce. Every bounce of a press is a falling edge, and every edge interrupts.
// A button's edge only counts as a press while its ready flag is 1: the handler then
// clears the flag, so the bounces that follow are ignored, until debounce() sets it again.
volatile uint32_t ready[2] = { 1, 1 };  // [0] SW1, [1] SW2: 1 = its next falling edge is a press

// Show the selected colour, turning the other LEDs off
void show(void)
{
	GPIOF->DATA = (GPIOF->DATA & ~LEDS) | led;
}

// Called by main once button sw (ready[i]) has been pressed. Wait for the press to stop
// bouncing, for the button to be released, and for the release to stop bouncing.
// Then the button is ready for its next press.
void debounce(uint32_t sw, uint32_t i)
{
	for (volatile uint32_t n = 0; n < DEBOUNCE_LOOPS; n++);    // Press bounce dies out
	while ((GPIOF->DATA & sw) == 0)                             // Wait for the release,
		show();                                                 // still showing the colour
	for (volatile uint32_t n = 0; n < DEBOUNCE_LOOPS; n++);    // Release bounce dies out
	ready[i] = 1;
}
// ---------------------------------------------------------------- end of provided part


// Every falling edge of SW1 or SW2, bounces included. Both buttons share this handler:
// MIS says which one it was. Like the Activity's, it only changes the state; main shows it.
void GPIOF_Handler(void)
{
	if (GPIOF->MIS & SW1)               // SW1: done for you
	{
		GPIOF->ICR = SW1;               // Clear SW1's flag
		if (ready[0])                   // A press, not a bounce
		{
			ready[0] = 0;
			led = colors1[index1];      // Show SW1's next colour...
			index1 = (index1 + 1) % 3;  // ...and move on: after the third, the first again
		}
	}
	// TODO 2: the same for SW2: check MIS, clear its flag, and if it's a press (ready[1]), // [!code highlight]
	//         show SW2's next colour from colors2 and step index2 // [!code highlight]
}


int main(void)
{
	SYSCTL->RCGCGPIO |= RCGC_GPIOF;                 // Clock to Port F
	while ((SYSCTL->PRGPIO & RCGC_GPIOF) == 0);     // Wait until it's ready

	// TODO 3: unlock PF0, so the lines below can set up SW2 too // [!code highlight]

	GPIOF->DIR = (GPIOF->DIR | LEDS) & ~SWITCHES;   // LEDs out, switches in
	GPIOF->PUR |= SWITCHES;                         // Pull-ups: pressed reads 0
	GPIOF->DEN |= LEDS | SWITCHES;

	// SW1 interrupts on its falling edge: the press
	GPIOF->IS  &= ~SW1;                             // Edge-sensitive
	GPIOF->IBE &= ~SW1;                             // One edge only...
	GPIOF->IEV &= ~SW1;                             // ...the falling edge
	GPIOF->ICR  = SW1;                              // Clear any edge seen during setup
	GPIOF->IM  |= SW1;                              // Let SW1 interrupt
	// TODO 4: make SW2 interrupt on its falling edge too // [!code highlight]
	NVIC_EnableIRQ(GPIOF_IRQn);                     // Port F is interrupt 30

	while (1)                                       // Provided: show, and debounce a press
	{
		show();
		if (ready[0] == 0)                          // SW1 was pressed
			debounce(SW1, 0);
		if (ready[1] == 0)                          // SW2 was pressed
			debounce(SW2, 1);
	}
}
