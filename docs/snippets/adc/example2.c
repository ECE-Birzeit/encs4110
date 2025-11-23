#include "TM4C123.h"

volatile unsigned int adc_value;
volatile float temp;

void ADC0SS3_Handler(void)
{
    adc_value = ADC0->SSFIFO3 & 0xFFF; // Read 12-bit ADC value from FIFO
    temp = 147.5 - (75 * 3.3f * adc_value)/ 4096; // Convert to temperature in Celsius
    ADC0->ISC = 8; // Clear SS3 interrupt flag
}

int main(void)
{
    SYSCTL->RCGCGPIO |= (1 << 4); // Enable clock to Port E
    SYSCTL->RCGCADC |= (1 << 0);  // Enable clock to ADC0

    GPIOE->AFSEL |= (1 << 3);  // Enable alternate function on PE3
    GPIOE->DEN &= ~(1 << 3);   // Disable digital I/O on PE3
    GPIOE->AMSEL |= (1 << 3);  // Enable analog input on PE3

    ADC0->ACTSS &= ~(1 << 3);            	// Disable SS3 during configuration
    ADC0->EMUX |= 0xF000;               	// Set SS3 to continuous sampling mode
    ADC0->SSMUX3 = 0;                    	// Configure SS3 to sample AIN0 (PE3)
    ADC0->SSCTL3 |= (1 << 1) | (1 << 2) | (1 << 3); 	// Set end of sequence, enable interrupt, enable temperature sensor

    ADC0->IM |= (1 << 3);        // Enable SS3 interrupt mask
    NVIC_EnableIRQ(ADC0SS3_IRQn); // Enable ADC0 SS3 interrupt in NVIC
    
    ADC0->ACTSS |= (1 << 3);  // Enable SS3

    while (1) {} // Wait for interrupts
}
