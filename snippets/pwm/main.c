// #region full
// #region defines
#include "TM4C123.h"

// Clocks
#define RCGC_GPIOF       (1 << 5)       // RCGCGPIO bit for Port F
#define RCGC_TIMER1      (1 << 1)       // RCGCTIMER bit for Timer1

// The pin: PF2 is the blue LED and also T1CCP0, Timer1A's output
#define BLUE_LED         (1 << 2)
#define PCTL_PF2_MASK    (0xF << 8)     // PF2's field in GPIOF->PCTL: bits 11:8
#define PCTL_PF2_T1CCP0  (0x7 << 8)     // 7 selects T1CCP0

// Timer1 register values
#define CTL_TAEN         (1 << 0)       // CTL: Timer A enable
#define CFG_16BIT        0x4            // CFG: split into two 16-bit timers
#define TAMR_PWM         0x0A           // TAMR: TAAMS = 1 (PWM), TACMR = 0, TAMR = 2 (periodic)

// The signal
#define PERIOD           50000          // Counts per period: 50 MHz / 50,000 = 1 kHz
#define DUTY             25             // Starting duty cycle, percent
// #endregion defines

// #region functions
volatile uint32_t level = DUTY;         // Brightness now, percent (1 to 99)
volatile int      step  = 1;            // +1 while getting brighter, -1 while dimming


// Set the duty cycle in percent. The match must stay strictly between 0 and TAILR.
void pwm_set(uint32_t percent)
{
	if (percent < 1)  percent = 1;
	if (percent > 99) percent = 99;
	TIMER1->TAMATCHR = (PERIOD - 1) - (PERIOD / 100) * percent;
}


// Every 10 ms: one percent brighter or dimmer. 98 steps each way: about 2 s per breath.
void SysTick_Handler(void)
{
	level += step;
	if (level >= 99) step = -1;
	if (level <= 1)  step = +1;
	pwm_set(level);
}
// #endregion functions


// #region main
int main(void)
{
	SYSCTL->RCGCGPIO  |= RCGC_GPIOF;		// Clock to Port F
	SYSCTL->RCGCTIMER |= RCGC_TIMER1;		// Clock to Timer1
	while ((SYSCTL->PRGPIO  & RCGC_GPIOF)  == 0);	// Wait until both are ready
	while ((SYSCTL->PRTIMER & RCGC_TIMER1) == 0);

	GPIOF->AFSEL |= BLUE_LED;									// PF2 driven by a peripheral...
	GPIOF->PCTL   = (GPIOF->PCTL & ~PCTL_PF2_MASK) | PCTL_PF2_T1CCP0;	// ...and that peripheral is Timer1A
	GPIOF->DEN   |= BLUE_LED;									// Digital pin

	TIMER1->CTL     &= ~CTL_TAEN;			// Stop Timer1A while configuring it
	TIMER1->CFG      = CFG_16BIT;			// PWM needs a 16-bit half
	TIMER1->TAMR     = TAMR_PWM;			// PWM, periodic
	TIMER1->TAPR     = 0;					// No extension: the period fits in 16 bits
	TIMER1->TAILR    = PERIOD - 1;			// Period: TAILR + 1 counts
	pwm_set(DUTY);							// Where the pin goes low
	TIMER1->CTL     |= CTL_TAEN;			// Start: the pin toggles by itself from now on

	SysTick_Config(SystemCoreClock / 100);	// SysTick interrupt every 10 ms

	while (1)								// Nothing left for the CPU: the timer makes the
	{										// signal and SysTick changes it
	}
}
// #endregion main
// #endregion full
