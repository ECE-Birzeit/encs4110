// Experiment 8 Task: a stopwatch on the LCD, showing mm:ss.t (minutes, seconds, tenths).
//   SW1 starts and stops it. SW2 resets it to 00:00.0, but only while it's stopped.
//   The green LED is on while it runs.
// Provided: lcd.c and lcd.h (the Activity's I2C and LCD driver, with millis(): add both
// to the project), the pins, the debouncing of both buttons (GPIOF_Handler and
// SysTick_Handler), and the main loop.
// Write the three TODOs (highlighted).
#include "TM4C123.h"
#include "lcd.h"

#define RCGC_GPIOF     (1U << 5)

// Port F pins
#define SW2            (1U << 0)        // PF0: reset (locked at reset)
#define GREEN_LED      (1U << 3)        // PF3: on while running
#define SW1            (1U << 4)        // PF4: start and stop
#define SWITCHES       (SW1 | SW2)

#define GPIO_UNLOCK    0x4C4F434B

#define DEBOUNCE_MS    20               // How long the buttons must stay up after a release

// The stopwatch: on_press() changes these, elapsed() reads them
volatile uint32_t running;              // 1 while it counts
volatile uint32_t start_ms;             // millis() when the current run started
volatile uint32_t total_ms;             // The time of all the runs before this one

void on_press(uint32_t button);


// ---------------------------------------------------------------- provided: don't change
// Debounce: act on the press, then wait for the release.
//   A falling edge on a button is a press: GPIOF_Handler calls on_press() once, then turns
//   the buttons' interrupts off, so the bounces that follow are ignored.
//   Every 1 ms, SysTick waits for the release: once both buttons have been up for
//   DEBOUNCE_MS in a row, it turns the interrupts back on, ready for the next press.
volatile uint32_t up_ms;                // How long both buttons have been up

void GPIOF_Handler(void)
{
	uint32_t button = GPIOF->MIS;       // Which button was pressed
	GPIOF->ICR = button;
	GPIOF->IM &= ~SWITCHES;             // Ignore the buttons until they're released
	up_ms = 0;

	if (button & SW1)
		on_press(SW1);
	else if (button & SW2)
		on_press(SW2);
}

void SysTick_Handler(void)
{
	ms++;                               // The 1 ms clock: millis() and delay_ms()

	if ((GPIOF->DATA & SWITCHES) != SWITCHES)   // A button is down (or bouncing)
		up_ms = 0;
	else if (++up_ms == DEBOUNCE_MS)    // Both up, and quiet for DEBOUNCE_MS
	{
		GPIOF->ICR = SWITCHES;          // Forget the release bounces
		GPIOF->IM |= SWITCHES;          // Listen for the next press
	}
}
// ---------------------------------------------------------------- end of provided part


// TODO 1: what each button does, from the diagram, using millis() to time the runs // [!code highlight]
void on_press(uint32_t button)
{
}

// TODO 2: return the stopwatch's time in ms. Read the shared variables with interrupts off. // [!code highlight]
uint32_t elapsed(void)
{
	return 0;
}

// TODO 3: print t (milliseconds) on line 2 as mm:ss.t, always two digits for mm and ss // [!code highlight]
void show_time(uint32_t t)
{
}


int main(void)
{
	uint32_t shown = 0xFFFFFFFF;        // The tenth on the screen (none yet)

	// Provided: Port F and both buttons' interrupts. This comes before SysTick_Config,
	// because SysTick_Handler reads the buttons.
	SYSCTL->RCGCGPIO |= RCGC_GPIOF;
	while ((SYSCTL->PRGPIO & RCGC_GPIOF) == 0);
	GPIOF->LOCK = GPIO_UNLOCK;
	GPIOF->CR  |= SW2;
	GPIOF->LOCK = 0;
	GPIOF->DIR  = (GPIOF->DIR | GREEN_LED) & ~SWITCHES;
	GPIOF->PUR |= SWITCHES;
	GPIOF->DEN |= GREEN_LED | SWITCHES;
	GPIOF->IS  &= ~SWITCHES;            // Edges...
	GPIOF->IBE &= ~SWITCHES;            // ...one direction...
	GPIOF->IEV &= ~SWITCHES;            // ...falling: a press
	GPIOF->ICR  = SWITCHES;
	GPIOF->IM  |= SWITCHES;
	NVIC_EnableIRQ(GPIOF_IRQn);

	SysTick_Config(SystemCoreClock / 1000);     // The 1 ms clock: millis() and delay_ms()
	I2C0_Init();                        // The LCD's I2C bus first...
	LCD_Init();                         // ...then the LCD
	LCD_SetCursor(0, 0);
	LCD_Print("Stopwatch");

	while (1)                           // Provided: redraw only when the tenth changes
	{
		uint32_t t = elapsed();
		if (t / 100 != shown)
		{
			shown = t / 100;
			show_time(t);
		}
	}
}
