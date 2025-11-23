#include "TM4C123.h"
#define GREEN_LED 1 << 3
volatile unsigned int adc_value;
int main(void)
{
    SYSCTL->RCGCGPIO |= (1 << 4); // Enable Clock to GPIOE or PE3/AN0
    SYSCTL->RCGCADC |= (1 << 0);  // AD0 clock enable
    
    GPIOE->AFSEL |= (1 << 3);  // enable alternate function
    GPIOE->DEN &= ~(1 << 3);   // disable digital function
    GPIOE->AMSEL |= (1 << 3);  // enable analog function
    
    ADC0->ACTSS &= ~(1 << 3);            // disable SS3 during configuration
    ADC0->EMUX &= ~0xF000;                // Software Trigger
    ADC0->SSMUX3 = 0;                    // get input from channel 0
    ADC0->SSCTL3 |= (1 << 1) | (1 << 2); // take one sample at a time, set flag at 1st sample
    ADC0->ACTSS |= (1 << 3);             // enable ADC0 sequencer 3

    SYSCTL->RCGCGPIO |= 0x20; // turn on bus clock for GPIOF
    GPIOF->DIR |= GREEN_LED;       // set GREEN pin as a digital output pin
    GPIOF->DEN |= GREEN_LED;       // Enable PF3 pin as a digital pin

    while (1)
    {
        ADC0->PSSI |= (1 << 3);            // initiate SS3 conversion
        while ((ADC0->RIS & 8) == 0);      // Wait untill sample conversion completed
        adc_value = ADC0->SSFIFO3;         // read adc coversion result from SS3 FIFO
        ADC0->ISC = 8;                     // clear coversion clear flag bit

        if (adc_value >= 2048)
            GPIOF->DATA |= GREEN_LED; // turn on green LED
        else if (adc_value < 2048)
            GPIOF->DATA &= ~GREEN_LED; // turn off green LED
    }
}
