#include "TM4C123.h"

// Clocks
#define RCGC_GPIOE    (1U << 4)         // RCGCGPIO bit for Port E (PE3 = AIN0)
#define RCGC_GPIOF    (1U << 5)         // RCGCGPIO bit for Port F (LED)
#define RCGC_ADC0     (1U << 0)         // RCGCADC bit for ADC0

// Pins
#define AIN0_PIN      (1U << 3)         // PE3
#define GREEN_LED     (1U << 3)         // PF3

// ADC0, sample sequencer 3
#define SS3           (1U << 3)         // ACTSS, PSSI, RIS, ISC, IM: sequencer 3
#define EMUX_SS3      0xF000            // EMUX: sequencer 3's trigger field (0 = software)
#define SSCTL_END0    (1U << 1)         // SSCTL3: the first sample is the last
#define SSCTL_IE0     (1U << 2)         // SSCTL3: set the flag after the first sample
#define HALF_SCALE    2048              // 12 bits: 0 to 4095, so 2048 is about 1.65 V

volatile uint32_t adc_value;

int main(void)
{
	SYSCTL->RCGCGPIO |= RCGC_GPIOE | RCGC_GPIOF;    // Clocks to Port E, Port F and ADC0
	SYSCTL->RCGCADC  |= RCGC_ADC0;
	while ((SYSCTL->PRGPIO & (RCGC_GPIOE | RCGC_GPIOF)) != (RCGC_GPIOE | RCGC_GPIOF));
	while ((SYSCTL->PRADC & RCGC_ADC0) == 0);

	GPIOE->AFSEL |= AIN0_PIN;                       // Alternate function
	GPIOE->DEN   &= ~AIN0_PIN;                      // Not digital
	GPIOE->AMSEL |= AIN0_PIN;                       // Analog
	GPIOE->DIR   &= ~AIN0_PIN;                      // Input (a don't-care once AMSEL is set)

	ADC0->ACTSS  &= ~SS3;                           // Sequencer 3 off while configuring
	ADC0->EMUX   &= ~EMUX_SS3;                      // Software trigger
	ADC0->SSMUX3  = 0;                              // Input: channel 0 (AIN0)
	ADC0->SSCTL3  = SSCTL_END0 | SSCTL_IE0;         // One sample, then set the flag
	ADC0->ACTSS  |= SS3;                            // Sequencer 3 on

	GPIOF->DIR |= GREEN_LED;                        // Green LED out
	GPIOF->DEN |= GREEN_LED;

	while (1)
	{
		ADC0->PSSI = SS3;                           // Start a conversion
		while ((ADC0->RIS & SS3) == 0);             // Wait until it's done
		adc_value = ADC0->SSFIFO3;                  // Read the result
		ADC0->ISC = SS3;                            // Clear the done flag

		if (adc_value >= HALF_SCALE)
			GPIOF->DATA |= GREEN_LED;               // Above half: green on
		else
			GPIOF->DATA &= ~GREEN_LED;              // Below half: green off
	}
}
