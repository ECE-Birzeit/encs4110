// Experiment 9 Task: a knob dimmer.
//   The potentiometer on PE3 sets the blue LED's brightness, from off to full.
//   Timer0A starts a conversion 100 times a second, with no code running at sample time.
//   Each result interrupts the CPU; main turns the newest reading into a PWM duty cycle.
// Provided: the pot pin and sequencer 3 (from the Activity), and the PWM on PF2 (from Experiment 8).
// Write the four TODOs (highlighted).
#include "TM4C123.h"

// Clocks
#define RCGC_GPIOE       (1U << 4)      // RCGCGPIO bit for Port E (the analog pin)
#define RCGC_GPIOF       (1U << 5)      // RCGCGPIO bit for Port F (the blue LED)
#define RCGC_ADC0        (1U << 0)      // RCGCADC bit for ADC0
#define RCGC_TIMER0      (1U << 0)      // RCGCTIMER bit for Timer0 (the ADC trigger)
#define RCGC_TIMER1      (1U << 1)      // RCGCTIMER bit for Timer1 (the PWM)

// Pins
#define AIN0             (1U << 3)      // PE3 is analog input 0
#define BLUE_LED         (1U << 2)      // PF2, also T1CCP0: Timer1A's output
#define PCTL_PF2_MASK    (0xFU << 8)    // PF2's field in GPIOF->PCTL: bits 11:8
#define PCTL_PF2_T1CCP0  (0x7U << 8)    // 7 selects T1CCP0

// ADC0, sample sequencer 3 (one sample per start)
#define SS3              (1U << 3)      // ACTSS, RIS, IM, ISC: sequencer 3's bit
#define EMUX_SS3         (0xFU << 12)   // EMUX: sequencer 3's trigger field
#define EMUX_SS3_TIMER   (0x5U << 12)   // EMUX: 0x5 in that field = started by a timer
#define SSCTL_END0       (1U << 1)      // SSCTL3: the first sample is the last
#define SSCTL_IE0        (1U << 2)      // SSCTL3: set the RIS flag when it's done

// Timer register values
#define CTL_TAEN         (1U << 0)      // CTL: Timer A enable
#define CTL_TAOTE        (1U << 5)      // CTL: Timer A output trigger: each time-out starts the ADC
#define CFG_32BIT        0x0            // CFG: one 32-bit timer
#define CFG_16BIT        0x4            // CFG: split into two 16-bit timers
#define TAMR_PERIODIC    0x2            // TAMR: periodic: count down, reload, repeat
#define TAMR_PWM         0x0A           // TAMR: TAAMS = 1 (PWM), TACMR = 0, TAMR = 2 (periodic)

// The numbers
#define SAMPLE_HZ        100            // ADC samples per second
#define PERIOD           50000          // PWM counts per period: 50 MHz / 50,000 = 1 kHz
#define VREF_MV          3300U          // Full scale: 3.3 V
#define ADC_MAX          4095U          // 12 bits: 0 to 4095

volatile uint32_t raw;                  // Newest reading, 0 to 4095 (from the ADC handler)
volatile uint32_t new_sample;           // 1 = the handler stored a reading main hasn't used yet
volatile uint32_t samples;              // How many readings so far: grows by SAMPLE_HZ a second
volatile uint32_t millivolts;           // raw in mV: 0 to 3300
volatile int32_t  duty;                 // The blue LED's brightness, percent


// ---------------------------------------------------------------- provided: don't change
// Experiment 8's PWM: set the duty cycle in percent, clamped to 1..99
void pwm_set(int32_t percent)
{
	if (percent < 1)  percent = 1;
	if (percent > 99) percent = 99;
	TIMER1->TAMATCHR = (PERIOD - 1) - (PERIOD / 100) * (uint32_t)percent;
}

// Timer1A: 1 kHz PWM on PF2 (Experiment 8's Activity)
void pwm_init(void)
{
	GPIOF->AFSEL |= BLUE_LED;
	GPIOF->PCTL   = (GPIOF->PCTL & ~PCTL_PF2_MASK) | PCTL_PF2_T1CCP0;
	GPIOF->DEN   |= BLUE_LED;

	TIMER1->CTL  &= ~CTL_TAEN;
	TIMER1->CFG   = CFG_16BIT;
	TIMER1->TAMR  = TAMR_PWM;
	TIMER1->TAPR  = 0;
	TIMER1->TAILR = PERIOD - 1;
	pwm_set(1);								// Start nearly off
	TIMER1->CTL  |= CTL_TAEN;
}
// ---------------------------------------------------------------- end of provided part


// TODO 3: one conversion is done. Read it from SSFIFO3 into raw (12 bits: & 0xFFF), // [!code highlight]
//         clear sequencer 3's flag in ISC, add one to samples, and set new_sample // [!code highlight]
void ADC0SS3_Handler(void)
{
}


int main(void)
{
	// Provided: clocks
	SYSCTL->RCGCGPIO  |= RCGC_GPIOE | RCGC_GPIOF;
	SYSCTL->RCGCADC   |= RCGC_ADC0;
	SYSCTL->RCGCTIMER |= RCGC_TIMER0 | RCGC_TIMER1;
	while ((SYSCTL->PRGPIO  & (RCGC_GPIOE | RCGC_GPIOF))   != (RCGC_GPIOE | RCGC_GPIOF));
	while ((SYSCTL->PRADC   & RCGC_ADC0) == 0);
	while ((SYSCTL->PRTIMER & (RCGC_TIMER0 | RCGC_TIMER1)) != (RCGC_TIMER0 | RCGC_TIMER1));

	// Provided: PE3 analog, as in the Activity
	GPIOE->DIR   &= ~AIN0;
	GPIOE->AFSEL |= AIN0;
	GPIOE->DEN   &= ~AIN0;
	GPIOE->AMSEL |= AIN0;

	pwm_init();								// Provided: the blue LED, by PWM

	// Sequencer 3: one sample of AIN0, as in the Activity, but started by the timer
	ADC0->ACTSS &= ~SS3;						// Off while configuring
	// TODO 2: trigger = timer (clear sequencer 3's EMUX field, then set it to 0x5), // [!code highlight]
	//         and the interrupt on: SS3 in IM, and ADC0SS3_IRQn in the NVIC // [!code highlight]
	ADC0->SSMUX3 = 0;							// Input: AIN0
	ADC0->SSCTL3 = SSCTL_END0 | SSCTL_IE0;		// One sample, flag when done
	ADC0->ACTSS |= SS3;							// On: waits for the timer

	// TODO 1: Timer0A as the trigger: stopped, 32-bit, periodic, SAMPLE_HZ time-outs a // [!code highlight]
	//         second (TAILR), then started with TAOTE and TAEN set. No timer interrupt. // [!code highlight]

	while (1)
	{
		if (new_sample)							// Once per reading: 100 times a second
		{
			new_sample = 0;
			millivolts = raw * VREF_MV / ADC_MAX;
			// TODO 4: map raw (0 to 4095) to duty (0 to 100 percent), and pass it to pwm_set() // [!code highlight]
		}
	}
}
