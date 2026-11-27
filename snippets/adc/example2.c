#include "TM4C123.h"

// Clocks
#define RCGC_ADC0       (1U << 0)       // RCGCADC bit for ADC0

// ADC0, sample sequencer 3
#define SS3             (1U << 3)       // ACTSS, IM, ISC: sequencer 3
#define EMUX_SS3        0xF000          // EMUX: sequencer 3's trigger field...
#define EMUX_SS3_ALWAYS 0xF000          // ...0xF: sample continuously
#define SSCTL_END0      (1U << 1)       // SSCTL3: the first sample is the last
#define SSCTL_IE0       (1U << 2)       // SSCTL3: interrupt after the first sample
#define SSCTL_TS0       (1U << 3)       // SSCTL3: sample the temperature sensor
#define ADC_MASK        0xFFF           // A result is 12 bits

volatile uint32_t adc_value;
volatile float temp;

void ADC0SS3_Handler(void)
{
	adc_value = ADC0->SSFIFO3 & ADC_MASK;                   // The 12-bit result
	temp = 147.5f - (75 * 3.3f * adc_value) / 4096;         // Degrees Celsius (datasheet formula)
	ADC0->ISC = SS3;                                        // Clear the interrupt flag
}

int main(void)
{
	SYSCTL->RCGCADC |= RCGC_ADC0;                   // Clock to ADC0
	while ((SYSCTL->PRADC & RCGC_ADC0) == 0);       // Wait until it's ready

	ADC0->ACTSS  &= ~SS3;                           // Sequencer 3 off while configuring
	ADC0->EMUX    = (ADC0->EMUX & ~EMUX_SS3) | EMUX_SS3_ALWAYS;     // Sample continuously
	ADC0->SSMUX3  = 0;                              // Channel field unused: TS0 picks the sensor
	ADC0->SSCTL3  = SSCTL_END0 | SSCTL_IE0 | SSCTL_TS0;     // One sample of the sensor, then interrupt

	ADC0->IM |= SS3;                                // Sequencer 3 interrupt...
	NVIC_EnableIRQ(ADC0SS3_IRQn);                   // ...and in the NVIC

	ADC0->ACTSS |= SS3;                             // Sequencer 3 on

	while (1)                                       // The handler does the work
	{
	}
}
