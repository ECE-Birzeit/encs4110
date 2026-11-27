#include "TM4C123.h"

#define RCGC_GPIOF      (1U << 5)       // RCGCGPIO bit for Port F
#define GREEN_LED       (1U << 3)       // PF3

// SysTick
#define STCTRL_ENABLE   (1U << 0)       // CTRL: counter on
#define STCTRL_INTEN    (1U << 1)       // CTRL: interrupt when it reaches 0
#define STCTRL_CLK_SRC  (1U << 2)       // CTRL: count the system clock
#define TICKS_100MS     5000000         // 100 ms at 50 MHz

volatile uint32_t systick_counter = 0;  // Milliseconds, in steps of 100

int main(void)
{
	SYSCTL->RCGCGPIO |= RCGC_GPIOF;                 // Clock to Port F
	while ((SYSCTL->PRGPIO & RCGC_GPIOF) == 0);     // Wait until Port F is ready

	GPIOF->DIR |= GREEN_LED;                        // Green LED out
	GPIOF->DEN |= GREEN_LED;

	SysTick->LOAD = TICKS_100MS - 1;                // Reload value for 100 ms
	SysTick->VAL  = 0;                              // Clear the current value
	SysTick->CTRL = STCTRL_CLK_SRC | STCTRL_INTEN | STCTRL_ENABLE;  // On, with interrupt

	while (1)
	{
	}
}

void SysTick_Handler(void)
{
	systick_counter += 100;                         // 100 ms more
	GPIOF->DATA ^= GREEN_LED;                       // Toggle the green LED
}
