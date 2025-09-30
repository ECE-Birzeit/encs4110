#include "TM4C123.h"

#define GREEN_LED 0x08					// PF3 - Green LED

volatile uint32_t systick_counter = 0;	// Global counter for SysTick interrupts

int main(void)
{
	SYSCTL->RCGCGPIO |= (1<<5);			// Enable clock to GPIOF
	
	__asm__("NOP");
	__asm__("NOP");
	__asm__("NOP");
	
	GPIOF->DIR |= GREEN_LED;			// Set green LED as output pin
	GPIOF->DEN |= GREEN_LED;			// Enable digital function for green LED
	

	SysTick->LOAD = 5000000 - 1;			// Set reload value for 1ms
	SysTick->VAL = 0;						// Clear current value
	SysTick->CTRL = 0x07;					// Enable SysTick with processor clock and interrupt
	
	while(1)								// Main loop
	{
	}
}

void SysTick_Handler(void)
{
	systick_counter++;					// Increment counter every 1ms
	GPIOF->DATA ^= GREEN_LED;			// Toggle green LED
}