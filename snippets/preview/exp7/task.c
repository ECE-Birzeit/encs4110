// Experiment 7 Task: a reaction-time game.
//   SW1 starts a round: the LED goes off, and after a random 1 to 3 s it turns green.
//   SW2 as soon as it's green: your reaction time in ms, and blue. SW2 before green: a false start (red).
// Provided: the pins, the 1 ms clock, the debouncing of both buttons, and show().
// Write the five TODOs (highlighted).
#include "TM4C123.h"

// Clocks
#define RCGC_GPIOF     (1U << 5)        // RCGCGPIO bit for Port F
#define RCGC_TIMER1    (1U << 1)        // RCGCTIMER bit for Timer1

// Port F pins
#define SW2            (1U << 0)        // PF0: react (locked at reset)
#define RED_LED        (1U << 1)        // PF1
#define BLUE_LED       (1U << 2)        // PF2
#define GREEN_LED      (1U << 3)        // PF3
#define SW1            (1U << 4)        // PF4: start a round
#define LEDS           (RED_LED | BLUE_LED | GREEN_LED)
#define SWITCHES       (SW1 | SW2)

#define GPIO_UNLOCK    0x4C4F434B       // "LOCK" in ASCII: written to GPIOF->LOCK to unlock CR

// Timer1 register values
#define CFG_32BIT      0x0              // CFG: one 32-bit timer
#define TAMR_ONESHOT   0x1              // TAMR: one-shot: count down once, then stop
#define CTL_TAEN       (1U << 0)        // CTL: Timer A enable
#define TATO           (1U << 0)        // IMR, MIS, ICR: Timer A time-out

#define DEBOUNCE_MS    20               // How long the buttons must stay up after a release

typedef enum {
	IDLE,           // waiting for SW1 to start a round
	// TODO 1: add the states WAITING (timer running, LED off) and GO (green on) // [!code highlight]
} State;

volatile State    state = IDLE;
volatile uint32_t ms;                   // Milliseconds since reset, from SysTick
volatile uint32_t go_ms;                // ms when the LED turned green
volatile uint32_t reaction;             // The last reaction time, in ms (Watch window)
volatile uint32_t best;                 // The best reaction time so far (0 = no round yet)
volatile uint32_t false_starts;         // How many times SW2 came too early

void on_press(uint32_t button);


// ---------------------------------------------------------------- provided: don't change
// Light exactly the LEDs in `led` (0 for off), turning the others off
void show(uint32_t led)
{
	GPIOF->DATA = (GPIOF->DATA & ~LEDS) | led;
}

// The Activity's debounce, for both buttons: act on the press, then wait for the release.
volatile uint32_t up_ms;                // How long both buttons have been up

// Every 1 ms: count the time, and once both buttons have been up for DEBOUNCE_MS in a row,
// listen for the next press
void SysTick_Handler(void)
{
	ms++;

	if ((GPIOF->DATA & SWITCHES) != SWITCHES)   // A button is down (or bouncing)
		up_ms = 0;
	else if (++up_ms == DEBOUNCE_MS)    // Both up, and quiet for DEBOUNCE_MS
	{
		GPIOF->ICR = SWITCHES;          // Forget the release bounces
		GPIOF->IM |= SWITCHES;          // Listen for the next press
	}
}

// A falling edge of SW1 or SW2: a press. Calls on_press() once, then ignores the buttons
// until they're released.
void GPIOF_Handler(void)
{
	uint32_t button = GPIOF->MIS;       // Which button was pressed
	GPIOF->ICR = button;
	GPIOF->IM &= ~SWITCHES;             // Ignore the bounces: SysTick turns this back on
	up_ms = 0;

	if (button & SW1)
		on_press(SW1);
	else if (button & SW2)
		on_press(SW2);
}
// ---------------------------------------------------------------- end of provided part


// A debounced press of `button` (SW1 or SW2). What it does depends on the state.
void on_press(uint32_t button)
{
	switch (state)
	{
	case IDLE:
		// TODO 2: on SW1, start a round: LED off, a random wait of 1000 + ms % 2000 ms // [!code highlight]
		//         loaded into Timer1A (TAILR) and the timer started (TAEN), then WAITING // [!code highlight]
		break;
	// TODO 3: add WAITING and GO. Only SW2 counts in them. In WAITING it's a false start: // [!code highlight]
	//         stop the timer, show red, count it, back to IDLE. In GO: store the reaction, // [!code highlight]
	//         update best if it's the first round or a better time, show blue, back to IDLE. // [!code highlight]
	}
}

// TODO 4: when Timer1A runs out: clear its time-out flag, light green, // [!code highlight]
//         note the time in go_ms, and go to GO // [!code highlight]
void TIMER1A_Handler(void)
{
}


int main(void)
{
	// Provided: clocks, pins, both buttons' interrupts, the 1 ms clock
	SYSCTL->RCGCGPIO  |= RCGC_GPIOF;
	SYSCTL->RCGCTIMER |= RCGC_TIMER1;
	while ((SYSCTL->PRGPIO  & RCGC_GPIOF)  == 0);
	while ((SYSCTL->PRTIMER & RCGC_TIMER1) == 0);

	GPIOF->LOCK = GPIO_UNLOCK;                      // Unlock PF0 for SW2
	GPIOF->CR  |= SW2;
	GPIOF->LOCK = 0;

	GPIOF->DIR = (GPIOF->DIR | LEDS) & ~SWITCHES;   // LEDs out, switches in
	GPIOF->PUR |= SWITCHES;                         // Pull-ups: pressed reads 0
	GPIOF->DEN |= LEDS | SWITCHES;
	GPIOF->IS  &= ~SWITCHES;                        // Both buttons: edge interrupts...
	GPIOF->IBE &= ~SWITCHES;                        // ...one direction...
	GPIOF->IEV &= ~SWITCHES;                        // ...falling: a press
	GPIOF->ICR  = SWITCHES;
	GPIOF->IM  |= SWITCHES;
	NVIC_EnableIRQ(GPIOF_IRQn);

	SysTick_Config(SystemCoreClock / 1000);         // 1 ms clock, after Port F: it reads the buttons

	// TODO 5: set up Timer1A: stopped, 32-bit, one-shot, time-out interrupt enabled in the // [!code highlight]
	//         timer (IMR) and the NVIC. Don't start it: SW1 does that, once per round. // [!code highlight]

	show(0);

	while (1)                                       // Everything happens in the interrupts
	{
	}
}
