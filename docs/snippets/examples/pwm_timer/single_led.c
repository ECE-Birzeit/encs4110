#include "TM4C123.h"

#define PWM_PIN  (1 << 2)      // PF2 = T1CCP0
#define DUTY_CYCLE 0.2f       // 20%

void Timer1A_PWM_Init(void) 
{
    // 1. Enable clock to Timer1 and Port F
    SYSCTL->RCGCTIMER |= (1 << 1);   // Enable Timer1
    SYSCTL->RCGCGPIO  |= (1 << 5);   // Enable Port F
    __asm__("NOP"); __asm__("NOP"); __asm__("NOP");  // allow time for activation

    // 2. Configure PF2 for T1CCP0 (Timer1A CCP output)
    GPIOF->AFSEL |= PWM_PIN;         // Enable alternate function
    GPIOF->PCTL &= ~(0xF << 8);      // Clear PCTL bits for PF2
    GPIOF->PCTL |=  (0x7 << 8);      // Assign T1CCP0 function (value = 7)
    GPIOF->DEN   |= PWM_PIN;         // Digital enable

    // 3. Disable Timer1A before configuration
    TIMER1->CTL &= ~(1 << 0);        // TAEN = 0

    // 4. Configure Timer1A for PWM mode
    TIMER1->CFG  = 0x4;              // 16-bit timer
    TIMER1->TAMR = (0x2 | (1 << 3)); // TAMR = periodic mode, PWM enabled (bit3 = 1)

    // 5. Load PWM period
    uint32_t period = 50000 - 1;     // 1 kHz at 50 MHz clock
    TIMER1->TAILR = period;

    // 6. Load MATCH value for duty cycle
    uint32_t match = (uint32_t)(period * (1.0f - DUTY_CYCLE));
    TIMER1->TAMATCHR = match - 1;

    // 7. Enable Timer1A
    TIMER1->CTL |= (1 << 0);         // TAEN = 1
}

int main(void) 
{
    Timer1A_PWM_Init();

    while(1) {
        // nothing
    }
}
