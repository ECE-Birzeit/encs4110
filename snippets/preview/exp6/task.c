// Experiment 6 Task: a vending machine. A drink costs two coins.
//   SW1 = insert a coin.   SW2 = the button: refund, or dispense once two coins are in.
// The interrupt handler and the debouncing are provided: don't change them.
// Write the three TODOs (highlighted): the states, next_state() and show().
#include "TM4C123.h"

#define RCGC_GPIOF   (1U << 5)          // RCGCGPIO bit for Port F

// Port F pins
#define SW2          (1U << 0)          // PF0: the button (locked at reset)
#define RED_LED      (1U << 1)          // PF1
#define BLUE_LED     (1U << 2)          // PF2
#define GREEN_LED    (1U << 3)          // PF3
#define SW1          (1U << 4)          // PF4: insert a coin
#define LEDS         (RED_LED | BLUE_LED | GREEN_LED)
#define SWITCHES     (SW1 | SW2)

#define GPIO_UNLOCK  0x4C4F434B         // "LOCK" in ASCII: written to GPIOF->LOCK to unlock CR

// Debounce timing: a delay loop of about 40 ms at 50 MHz
#define F_CPU_HZ         50000000U
#define CYCLES_PER_LOOP  7U             // one pass of the volatile loop, about 7 cycles
#define DEBOUNCE_MS      40U
#define DEBOUNCE_LOOPS   (F_CPU_HZ / 1000U / CYCLES_PER_LOOP * DEBOUNCE_MS)

// The four states from the diagram
typedef enum {
	IDLE,           // no coins, LED off
	ONE_COIN,       // one coin in, blue
	// TODO 1: add the states READY and VEND here // [!code highlight]
} State;

volatile State    state = IDLE;         // Where the machine is now (Watch window)
volatile uint32_t button;               // The button just pressed: SW1 or SW2 (0 = none)
volatile uint32_t ready[2] = { 1, 1 };  // [0] SW1, [1] SW2: 1 = its next falling edge is a press


// ---------------------------------------------------------------- provided: don't change
// Every falling edge of either button, bounces included. Only the first edge of a press
// counts: it notes the button for main and clears that button's ready flag, so its bounces
// are ignored until debounce() sets the flag again. Each button is debounced on its own.
void GPIOF_Handler(void)
{
	uint32_t edge = GPIOF->MIS & SWITCHES;  // Which button caused the interrupt
	GPIOF->ICR = edge;                  // Clear its flag

	if ((edge & SW1) && ready[0])       // A press of SW1, not a bounce
	{
		ready[0] = 0;
		button = SW1;
	}
	else if ((edge & SW2) && ready[1])  // A press of SW2, not a bounce
	{
		ready[1] = 0;
		button = SW2;
	}
}

// Wait for the press to stop bouncing, for the button b to be released, and for the release
// to stop bouncing. Then b is ready for its next press.
void debounce(uint32_t b)
{
	for (volatile uint32_t i = 0; i < DEBOUNCE_LOOPS; i++);    // Press bounce dies out
	while ((GPIOF->DATA & b) == 0);                             // Wait until it's released
	for (volatile uint32_t i = 0; i < DEBOUNCE_LOOPS; i++);    // Release bounce dies out
	if (b == SW1)
		ready[0] = 1;
	else
		ready[1] = 1;
}
// ---------------------------------------------------------------- end of provided part


// The next state, from the current state and the button pressed: follow the diagram's arrows.
// A button that loops back to the same state falls through to "return s": stay where you are.
State next_state(State s, uint32_t pressed)
{
	switch (s)
	{
	case IDLE:                              // Done for you, as an example
		if (pressed == SW1) return ONE_COIN;
		break;
	// TODO 2: add a case for ONE_COIN, READY and VEND here // [!code highlight]
	}
	return s;
}

// Light the LED for state s, the colour of its circle, and turn the other colours off
void show(State s)
{
	uint32_t led = 0;

	switch (s)
	{
	case IDLE:     led = 0;         break;  // Done for you
	case ONE_COIN: led = BLUE_LED;  break;
	// TODO 3: add READY (GREEN_LED) and VEND (white: LEDS) here // [!code highlight]
	}
	GPIOF->DATA = (GPIOF->DATA & ~LEDS) | led;
}


int main(void)
{
	SYSCTL->RCGCGPIO |= RCGC_GPIOF;                 // Clock to Port F
	while ((SYSCTL->PRGPIO & RCGC_GPIOF) == 0);

	GPIOF->LOCK = GPIO_UNLOCK;                      // Unlock PF0 for SW2
	GPIOF->CR  |= SW2;
	GPIOF->LOCK = 0;

	GPIOF->DIR = (GPIOF->DIR | LEDS) & ~SWITCHES;   // LEDs out, switches in
	GPIOF->PUR |= SWITCHES;                         // Pull-ups: pressed reads 0
	GPIOF->DEN |= LEDS | SWITCHES;

	GPIOF->IS  &= ~SWITCHES;                        // Both buttons: falling edge interrupts
	GPIOF->IBE &= ~SWITCHES;
	GPIOF->IEV &= ~SWITCHES;
	GPIOF->ICR  = SWITCHES;
	GPIOF->IM  |= SWITCHES;
	NVIC_EnableIRQ(GPIOF_IRQn);

	show(state);

	while (1)
	{
		if (button)                     // A press arrived (the handler set it)
		{
			uint32_t b = button;
			button = 0;
			state = next_state(state, b);
			show(state);
			debounce(b);                // Provided: wait out this button's bounce
		}
	}
}
