#include "TM4C123.h"

#define BLUE_LED  (1 << 2)              // PF2 = T1CCP0 (Timer1A's PWM pin)
#define PERIOD    50000                 // 1 kHz at 50 MHz

volatile uint32_t level = 1;			// Brightness in percent, 1 to 99
volatile int      step  = 1;			// +1 while getting brighter, -1 while dimming


// Keep the match strictly between 0 and TAILR: the two ends are special cases
void pwm_set(uint32_t percent)
{
	if (percent < 1)  percent = 1;
	if (percent > 99) percent = 99;
	TIMER1->TAMATCHR = (PERIOD - 1) - (PERIOD / 100) * percent;
}


int main(void)
{
	SYSCTL->RCGCGPIO  |= (1 << 5);		// Enable clock to GPIOF
	SYSCTL->RCGCTIMER |= (1 << 1);		// Enable clock to Timer1

	__asm__("NOP");						// Wait for clock stabilization
	__asm__("NOP");
	__asm__("NOP");

	GPIOF->AFSEL |= BLUE_LED;						// PF2 driven by Timer1A
	GPIOF->PCTL   = (GPIOF->PCTL & ~0x00000F00) | 0x00000700;	// 7 = T1CCP0
	GPIOF->DEN   |= BLUE_LED;

	TIMER1->CTL   &= ~(1 << 0);			// Disable Timer1A while configuring
	TIMER1->CFG    = 0x4;				// 16-bit mode
	TIMER1->TAMR   = 0x0A;				// PWM, periodic
	TIMER1->TAPR   = 0;
	TIMER1->TAILR  = PERIOD - 1;
	pwm_set(level);						// Start almost off
	TIMER1->CTL   |= (1 << 0);			// Enable Timer1A

	SysTick_Config(SystemCoreClock / 100);	// SysTick interrupt every 10 ms

	while (1)
	{
	}
}


// Every 10 ms: one step brighter or dimmer. 99 steps each way = about 2 s per breath.
void SysTick_Handler(void)
{
	level += step;
	if (level >= 99) step = -1;
	if (level <= 1)  step = +1;
	pwm_set(level);
}
