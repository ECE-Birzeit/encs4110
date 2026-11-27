#include "TM4C123.h"

#define RCGC_GPIOF   (1U << 5)          // RCGCGPIO bit for Port F
#define RED_LED      (1U << 1)          // PF1
#define SW1          (1U << 4)          // PF4: pressed reads 0

int main(void)
{
	SYSCTL->RCGCGPIO |= RCGC_GPIOF;                 // Clock to Port F
	while ((SYSCTL->PRGPIO & RCGC_GPIOF) == 0);     // Wait until Port F is ready

	GPIOF->PUR |= SW1;                              // Pull-up on SW1
	GPIOF->DIR  = (GPIOF->DIR | RED_LED) & ~SW1;    // LED out, SW1 in
	GPIOF->DEN |= RED_LED | SW1;                    // Both digital

	while (1)
	{
		if ((GPIOF->DATA & SW1) == 0)               // Pressed (the pull-up makes it active low)
			GPIOF->DATA |= RED_LED;                 // LED on
		else
			GPIOF->DATA &= ~RED_LED;                // LED off
	}
}
