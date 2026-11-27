// #region full
// #region defines
#include "TM4C123.h"

// Clocks
#define RCGC_GPIOE    (1U << 4)         // RCGCGPIO bit for Port E (the analog pin)
#define RCGC_GPIOF    (1U << 5)         // RCGCGPIO bit for Port F (LEDs)
#define RCGC_ADC0     (1U << 0)         // RCGCADC bit for ADC0

// Pins
#define AIN0          (1U << 3)         // PE3 is analog input 0
#define RED_LED       (1U << 1)         // PF1
#define BLUE_LED      (1U << 2)         // PF2
#define GREEN_LED     (1U << 3)         // PF3
#define LEDS          (RED_LED | BLUE_LED | GREEN_LED)

// ADC0, sample sequencer 3 (one sample per start)
#define SS3           (1U << 3)         // ACTSS, PSSI, RIS, ISC: sequencer 3's bit
#define EMUX_SS3      (0xFU << 12)      // EMUX: sequencer 3's trigger field (0 = software)
#define SSCTL_END0    (1U << 1)         // SSCTL3: the first sample is the last
#define SSCTL_IE0     (1U << 2)         // SSCTL3: set the RIS flag when it's done

// The conversion
#define VREF_MV       3300U             // Full scale: 3.3 V
#define ADC_MAX       4095U             // 12 bits: 0 to 4095

volatile uint32_t raw;                  // Last reading, 0 to 4095
volatile uint32_t millivolts;           // The same, in mV: 0 to 3300
// #endregion defines


// #region read
// Start one conversion on sequencer 3, wait for it, and return the 12-bit result
uint32_t adc_read(void)
{
	uint32_t value;

	ADC0->PSSI = SS3;							// Start: software trigger
	while ((ADC0->RIS & SS3) == 0);				// Wait for the conversion (about 1 us)
	value = ADC0->SSFIFO3 & 0xFFF;				// The result, 12 bits
	ADC0->ISC = SS3;							// Clear the flag for next time
	return value;
}
// #endregion read


// #region main
int main(void)
{
	SYSCTL->RCGCGPIO |= RCGC_GPIOE | RCGC_GPIOF;
	SYSCTL->RCGCADC  |= RCGC_ADC0;
	while ((SYSCTL->PRGPIO & (RCGC_GPIOE | RCGC_GPIOF)) != (RCGC_GPIOE | RCGC_GPIOF));
	while ((SYSCTL->PRADC  & RCGC_ADC0) == 0);

	// PE3: analog. Disconnect the digital circuit, connect the analog one.
	GPIOE->DIR   &= ~AIN0;
	GPIOE->AFSEL |= AIN0;
	GPIOE->DEN   &= ~AIN0;
	GPIOE->AMSEL |= AIN0;

	GPIOF->DIR |= LEDS;
	GPIOF->DEN |= LEDS;

	// Sequencer 3: one sample of AIN0, started by software
	ADC0->ACTSS &= ~SS3;						// Off while configuring
	ADC0->EMUX  &= ~EMUX_SS3;					// Trigger: software (PSSI)
	ADC0->SSMUX3 = 0;							// Input: AIN0
	ADC0->SSCTL3 = SSCTL_END0 | SSCTL_IE0;		// One sample, flag when done
	ADC0->ACTSS |= SS3;							// On

	while (1)
	{
		raw        = adc_read();
		millivolts = raw * VREF_MV / ADC_MAX;

		// Three zones: red below 1.1 V, green from 1.1 V to 2.2 V, blue above
		if (millivolts < 1100)
			GPIOF->DATA = (GPIOF->DATA & ~LEDS) | RED_LED;
		else if (millivolts < 2200)
			GPIOF->DATA = (GPIOF->DATA & ~LEDS) | GREEN_LED;
		else
			GPIOF->DATA = (GPIOF->DATA & ~LEDS) | BLUE_LED;
	}
}
// #endregion main
// #endregion full
