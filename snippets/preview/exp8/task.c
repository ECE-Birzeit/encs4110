// Experiment 8 Task: an RGB colour picker.
//   All three LEDs are dimmed by PWM: red on Timer0B (PF1), blue on Timer1A (PF2), green on Timer1B (PF3).
//   SW1 selects the colour to change: red, green, blue, and round again.
//   SW2 steps the selected colour's level by 20%: 0, 20, 40, 60, 80, 100, then back to 0.
// Provided: the pins, red's Timer0B and blue's Timer1A, pwm_match(), and the debouncing of both buttons.
// Write the five TODOs (highlighted).
#include "TM4C123.h"

// Clocks
#define RCGC_GPIOF       (1U << 5)      // RCGCGPIO bit for Port F
#define RCGC_TIMER0      (1U << 0)      // RCGCTIMER bit for Timer0
#define RCGC_TIMER1      (1U << 1)      // RCGCTIMER bit for Timer1

// Port F pins
#define SW2              (1U << 0)      // PF0: step the level (locked at reset)
#define RED_LED          (1U << 1)      // PF1: T0CCP1, Timer0B's output
#define BLUE_LED         (1U << 2)      // PF2: T1CCP0, Timer1A's output
#define GREEN_LED        (1U << 3)      // PF3: T1CCP1, Timer1B's output
#define SW1              (1U << 4)      // PF4: select the colour
#define LEDS             (RED_LED | BLUE_LED | GREEN_LED)
#define SWITCHES         (SW1 | SW2)

#define GPIO_UNLOCK      0x4C4F434B     // "LOCK" in ASCII: written to GPIOF->LOCK to unlock CR

// Each LED pin's field in GPIOF->PCTL, and the value 7 that hands it to its timer
#define PCTL_PF1_MASK    (0xFU << 4)    // PF1: bits 7:4
#define PCTL_PF1_T0CCP1  (0x7U << 4)
#define PCTL_PF2_MASK    (0xFU << 8)    // PF2: bits 11:8
#define PCTL_PF2_T1CCP0  (0x7U << 8)
#define PCTL_PF3_MASK    (0xFU << 12)   // PF3: bits 15:12
#define PCTL_PF3_T1CCP1  (0x7U << 12)

// Timer register values
#define CTL_TAEN         (1U << 0)      // CTL: Timer A enable
#define CTL_TBEN         (1U << 8)      // CTL: Timer B enable
#define CFG_16BIT        0x4            // CFG: split into two 16-bit timers
#define TMR_PWM          0x0A           // TAMR or TBMR: PWM, edge-count, periodic

// The signal
#define PERIOD           50000          // Counts per period: 50 MHz / 50,000 = 1 kHz
#define STEP             20             // SW2 adds this much to the selected colour, percent

#define DEBOUNCE_MS      20             // How long the buttons must stay up after a release

// The colours, as indexes into level[]
#define RED              0
#define GREEN            1
#define BLUE             2

volatile int32_t  level[3] = { 20, 20, 20 };    // Each colour's level, percent: [RED], [GREEN], [BLUE]
volatile uint32_t selected = RED;               // The colour SW2 changes (Watch window)

void on_press(uint32_t button);


// ---------------------------------------------------------------- provided: don't change
// The match value for a duty cycle in percent: the Activity's pwm_set() without the write,
// so it works for any timer half. The duty is clamped to 1 to 99, so 0 gives 1% and 100 gives 99%.
uint32_t pwm_match(int32_t percent)
{
	if (percent < 1)  percent = 1;
	if (percent > 99) percent = 99;
	return (PERIOD - 1) - (PERIOD / 100) * (uint32_t)percent;
}

// Each button has a ready flag: a falling edge only counts while its button is ready.
volatile uint32_t ready[2] = { 1, 1 };  // [0] SW1, [1] SW2: 1 = its next falling edge is a press
volatile uint32_t up_ms[2];             // How long each button has been up, while not ready

// Called every 1 ms for each button. Once a pressed button has been up for DEBOUNCE_MS
// in a row, it's ready for its next press.
void check_release(uint32_t sw, uint32_t i)
{
	if (ready[i])                       // Not waiting for this one
		return;
	if ((GPIOF->DATA & sw) == 0)        // Down (or bouncing)
		up_ms[i] = 0;
	else if (++up_ms[i] >= DEBOUNCE_MS) // Up, and quiet for DEBOUNCE_MS
		ready[i] = 1;
}

// Every 1 ms
void SysTick_Handler(void)
{
	check_release(SW1, 0);
	check_release(SW2, 1);
}

// Every falling edge of SW1 or SW2, bounces included. Only a ready button's edge is a press:
// it calls on_press() once, and the button isn't ready again until it's released.
void GPIOF_Handler(void)
{
	uint32_t edge = GPIOF->MIS & SWITCHES;  // Which button caused the interrupt
	GPIOF->ICR = edge;

	if ((edge & SW1) && ready[0])
	{
		up_ms[0] = 0;
		ready[0] = 0;
		on_press(SW1);
	}
	if ((edge & SW2) && ready[1])
	{
		up_ms[1] = 0;
		ready[1] = 0;
		on_press(SW2);
	}
}
// ---------------------------------------------------------------- end of provided part


// Show a colour: each argument is a percentage, for red, green, and blue
void set_color(int32_t r, int32_t g, int32_t b)
{
	// TODO 3: write each colour's match value, from pwm_match(), into its timer half: // [!code highlight]
	//         red into TIMER0->TBMATCHR, green into TIMER1->TBMATCHR, blue into TIMER1->TAMATCHR // [!code highlight]
}


// A debounced press of `button` (SW1 or SW2)
void on_press(uint32_t button)
{
	if (button == SW1)
	{
		// TODO 4: select the next colour: RED, GREEN, BLUE, then RED again // [!code highlight]
	}
	else
	{
		// TODO 5: add STEP to the selected colour's level; past 100 it goes back to 0. // [!code highlight]
		//         Then show all three levels with set_color() // [!code highlight]
	}
}


int main(void)
{
	// Provided: clocks
	SYSCTL->RCGCGPIO  |= RCGC_GPIOF;
	SYSCTL->RCGCTIMER |= RCGC_TIMER0 | RCGC_TIMER1;
	while ((SYSCTL->PRGPIO  & RCGC_GPIOF) == 0);
	while ((SYSCTL->PRTIMER & (RCGC_TIMER0 | RCGC_TIMER1)) != (RCGC_TIMER0 | RCGC_TIMER1));

	// Provided: the LED pins, handed to their timers
	GPIOF->AFSEL |= RED_LED | BLUE_LED;
	GPIOF->PCTL   = (GPIOF->PCTL & ~PCTL_PF1_MASK) | PCTL_PF1_T0CCP1;  // PF1: Timer0B
	GPIOF->PCTL   = (GPIOF->PCTL & ~PCTL_PF2_MASK) | PCTL_PF2_T1CCP0;  // PF2: Timer1A
	// TODO 1: hand PF3 to Timer1B, as the two lines above do for PF1 and PF2 (AFSEL, then PCTL) // [!code highlight]

	// Provided: both buttons, with falling-edge interrupts
	GPIOF->LOCK = GPIO_UNLOCK;                      // Unlock PF0 for SW2
	GPIOF->CR  |= SW2;
	GPIOF->LOCK = 0;

	GPIOF->DIR &= ~SWITCHES;                        // Switches in (the timers drive the LED pins)
	GPIOF->PUR |= SWITCHES;                         // Pull-ups: pressed reads 0
	GPIOF->DEN |= LEDS | SWITCHES;
	GPIOF->IS  &= ~SWITCHES;                        // Both buttons: edge interrupts...
	GPIOF->IBE &= ~SWITCHES;                        // ...one direction...
	GPIOF->IEV &= ~SWITCHES;                        // ...falling: a press
	GPIOF->ICR  = SWITCHES;
	GPIOF->IM  |= SWITCHES;
	NVIC_EnableIRQ(GPIOF_IRQn);

	// Provided: red, Timer0B, 1 kHz PWM
	TIMER0->CTL  &= ~CTL_TBEN;                      // Stop Timer0B while configuring it
	TIMER0->CFG   = CFG_16BIT;
	TIMER0->TBMR  = TMR_PWM;
	TIMER0->TBPR  = 0;
	TIMER0->TBILR = PERIOD - 1;
	TIMER0->TBMATCHR = pwm_match(level[RED]);
	TIMER0->CTL  |= CTL_TBEN;

	// Provided: blue, Timer1A, as in the Activity
	TIMER1->CTL  &= ~(CTL_TAEN | CTL_TBEN);         // Stop both halves: they share CFG and CTL
	TIMER1->CFG   = CFG_16BIT;
	TIMER1->TAMR  = TMR_PWM;
	TIMER1->TAPR  = 0;
	TIMER1->TAILR = PERIOD - 1;
	TIMER1->TAMATCHR = pwm_match(level[BLUE]);
	TIMER1->CTL  |= CTL_TAEN;

	// TODO 2: set up green, Timer1B, like Timer0B above, but without the CFG line: Timer1's CFG // [!code highlight]
	//         is already set. Timer1A is running, so start Timer1B with |= (CTL is shared) // [!code highlight]

	set_color(level[RED], level[GREEN], level[BLUE]);

	SysTick_Config(SystemCoreClock / 1000);         // 1 ms, after Port F: it reads the buttons

	while (1)                                       // Everything happens in the interrupts
	{
	}
}
