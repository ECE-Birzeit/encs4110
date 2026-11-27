// Experiment 12 Task: a dimmer for the blue LED.
//   Each click of the knob changes the brightness: 1% when turning slowly, 5% when fast.
//   The brightness stays within 1 to 99%. Pressing the knob switches the light off and on.
// Provided: the PWM on PF2 (from Experiment 8), QEI1 and its speed, the 1 ms clock with
// the knob button's debounce, and light().
// Write the five TODOs (highlighted).
#include "TM4C123.h"

// Clocks
#define RCGC_GPIOC       (1U << 2)      // RCGCGPIO bit for Port C (encoder and its button)
#define RCGC_GPIOF       (1U << 5)      // RCGCGPIO bit for Port F (blue LED)
#define RCGC_TIMER1      (1U << 1)      // RCGCTIMER bit for Timer1
#define RCGC_QEI1        (1U << 1)      // RCGCQEI bit for QEI1

// The encoder's pins: PC5 = PhA1 (CLK), PC6 = PhB1 (DT), and its button on PC4 (SW)
#define KNOB_SW          (1U << 4)      // PC4: plain GPIO input, low when pressed
#define QEI1_PHA         (1U << 5)
#define QEI1_PHB         (1U << 6)
#define PCTL_PC56_MASK   (0xFFU << 20)  // PC5's and PC6's fields in GPIOC->PCTL: bits 27:20
#define PCTL_PC56_QEI    (0x66U << 20)  // 6 selects PhA1 and PhB1

// The PWM pin: PF2 is the blue LED and also T1CCP0, Timer1A's output
#define BLUE_LED         (1U << 2)
#define PCTL_PF2_MASK    (0xFU << 8)    // PF2's field in GPIOF->PCTL: bits 11:8
#define PCTL_PF2_T1CCP0  (0x7U << 8)    // 7 selects T1CCP0

// Timer1 register values (Experiment 8)
#define CTL_TAEN         (1U << 0)      // CTL: Timer A enable
#define CFG_16BIT        0x4            // CFG: split into two 16-bit timers
#define TAMR_PWM         0x0A           // TAMR: TAAMS = 1 (PWM), TACMR = 0, TAMR = 2 (periodic)
#define PERIOD           50000          // Counts per period: 50 MHz / 50,000 = 1 kHz

// QEI1 register bits (the Activity)
#define CTL_ENABLE       (1U << 0)      // CTL: start the module (can't be cleared again)
#define CTL_CAPMODE      (1U << 3)      // CTL: count the edges of both PhA and PhB
#define CTL_VELEN        (1U << 5)      // CTL: velocity capture on
#define CTL_FILTEN       (1U << 13)     // CTL: input filter on
#define CTL_FILTCNT_MAX  (0xFU << 16)   // CTL: filter samples every 17 clocks
#define STAT_DIRECTION   (1U << 1)      // STAT: 1 = last moved in reverse
#define INT_TIMER        (1U << 1)      // INTEN, RIS, ISC: velocity window ended

#define PPR              20             // Pulses per revolution: 20 on the HW-040 (one per click)
#define EDGES            (PPR * 4)      // Counts per turn with CAPMODE: 80
#define WINDOW_HZ        10             // Speed windows per second: 100 ms each

#define FAST_RPM         120            // Faster than this: big steps
#define SLOW_STEP        1              // Percent per click when slow
#define FAST_STEP        5              // Percent per click when fast
#define DEBOUNCE_MS      20             // How long the knob's button must stay up after a release

volatile int32_t  level = 25;           // Brightness, percent (1 to 99): signed, like in Experiment 8
volatile int32_t  rpm;                  // Speed, negative in reverse
volatile uint32_t on = 1;               // 1 = light on, 0 = off
volatile uint32_t up_ms;                // How long the knob's button has been up


// ---------------------------------------------------------------- provided: don't change
// Experiment 8's pwm_set(): the duty cycle in percent, clamped to 1 to 99
void pwm_set(int32_t percent)
{
	if (percent < 1)  percent = 1;
	if (percent > 99) percent = 99;
	TIMER1->TAMATCHR = (PERIOD - 1) - (PERIOD / 100) * (uint32_t)percent;
}

// Light on: PF2 is driven by the PWM. Light off: PF2 is a plain GPIO output, low.
// The PWM keeps running either way, so switching back on keeps the brightness.
void light(uint32_t on_now)
{
	if (on_now)
		GPIOF->AFSEL |= BLUE_LED;
	else
		GPIOF->AFSEL &= ~BLUE_LED;
}

// The Activity's speed handler: every 100 ms, the edges of the last window as rpm
void QEI1_Handler(void)
{
	QEI1->ISC = INT_TIMER;
	rpm = QEI1->SPEED * WINDOW_HZ * 60 / EDGES;
	if (QEI1->STAT & STAT_DIRECTION)
		rpm = -rpm;
}

// Every 1 ms: wait for the knob button's release, as for SW1 in the Experiment 7 Activity.
// Once it has been up for DEBOUNCE_MS in a row, listen for the next press.
void SysTick_Handler(void)
{
	if ((GPIOC->DATA & KNOB_SW) == 0)		// Down (or bouncing)
		up_ms = 0;
	else if (++up_ms == DEBOUNCE_MS)		// Up, and quiet for DEBOUNCE_MS
	{
		GPIOC->ICR = KNOB_SW;				// Forget the release bounces
		GPIOC->IM |= KNOB_SW;				// Listen for the next press
	}
}
// ---------------------------------------------------------------- end of provided part


// The knob button's falling edge: a press
void GPIOC_Handler(void)
{
	// TODO 1: clear the flag, mask KNOB_SW in IM (SysTick unmasks it), set up_ms to 0, // [!code highlight]
	//         then switch the light: flip `on`, and call light(on) // [!code highlight]
}


int main(void)
{
	int32_t last = 0;						// The position at the last whole click

	// Provided: clocks
	SYSCTL->RCGCGPIO  |= RCGC_GPIOC | RCGC_GPIOF;
	SYSCTL->RCGCTIMER |= RCGC_TIMER1;
	SYSCTL->RCGCQEI   |= RCGC_QEI1;
	while ((SYSCTL->PRGPIO & (RCGC_GPIOC | RCGC_GPIOF)) != (RCGC_GPIOC | RCGC_GPIOF));
	while ((SYSCTL->PRTIMER & RCGC_TIMER1) == 0);
	while ((SYSCTL->PRQEI   & RCGC_QEI1)   == 0);

	// Provided: PF2 to Timer1A, and a 1 kHz PWM, as in the Experiment 8 Activity
	GPIOF->DIR   |= BLUE_LED;									// Output, for when light() is off
	GPIOF->DATA  &= ~BLUE_LED;									// ...and low
	GPIOF->AFSEL |= BLUE_LED;
	GPIOF->PCTL   = (GPIOF->PCTL & ~PCTL_PF2_MASK) | PCTL_PF2_T1CCP0;
	GPIOF->DEN   |= BLUE_LED;

	TIMER1->CTL  &= ~CTL_TAEN;
	TIMER1->CFG   = CFG_16BIT;
	TIMER1->TAMR  = TAMR_PWM;
	TIMER1->TAPR  = 0;
	TIMER1->TAILR = PERIOD - 1;
	pwm_set(level);
	TIMER1->CTL  |= CTL_TAEN;

	// Provided: QEI1 as in the Activity, but the position never wraps within reach of a hand
	GPIOC->AFSEL |= QEI1_PHA | QEI1_PHB;					// Port C: only bits 4 to 6 ever change
	GPIOC->PCTL   = (GPIOC->PCTL & ~PCTL_PC56_MASK) | PCTL_PC56_QEI;
	GPIOC->PUR   |= QEI1_PHA | QEI1_PHB;
	GPIOC->DEN   |= QEI1_PHA | QEI1_PHB;

	QEI1->CTL    = CTL_FILTCNT_MAX | CTL_FILTEN | CTL_CAPMODE | CTL_VELEN;
	QEI1->MAXPOS = 0xFFFFFFFF;						// Wraps like any 32-bit number
	QEI1->CTL   |= CTL_ENABLE;
	QEI1->POS    = 0;
	QEI1->LOAD   = SystemCoreClock / WINDOW_HZ - 1;	// 100 ms windows
	QEI1->INTEN  = INT_TIMER;
	NVIC_EnableIRQ(QEI1_IRQn);

	// TODO 2: set up the knob's button on PC4, like SW1 in Experiment 7: input with a pull-up, // [!code highlight]
	//         digital, falling-edge interrupt, IM, and GPIOC_IRQn in the NVIC. Only |= and &= ~ // [!code highlight]
	//         on Port C, and no AFSEL: that would make PC4 the QEI's index input // [!code highlight]

	SysTick_Config(SystemCoreClock / 1000);		// 1 ms clock, after Port C: it reads PC4

	while (1)
	{
		int32_t steps = 0;						// Whole clicks since last time, negative in reverse
		int32_t size  = SLOW_STEP;				// Percent per click

		// TODO 3: count whole clicks: the position now minus `last`, divided by 4 (one click), // [!code highlight]
		//         into `steps`. Then move `last` on by the clicks you counted (steps * 4) // [!code highlight]

		// TODO 4: if the knob turns faster than FAST_RPM, either way, size is FAST_STEP // [!code highlight]

		// TODO 5: only while the light is on: change level by steps * size, keep it within // [!code highlight]
		//         1 to 99, and send it to pwm_set() // [!code highlight]
	}
}
