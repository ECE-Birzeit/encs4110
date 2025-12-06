#include "TM4C123.h"
#include <stdint.h>
#include <math.h>
#define RED_PIN   (1 << 1)    // PF1 = T0CCP1  (Timer0B)
#define GREEN_PIN (1 << 2)    // PF2 = T1CCP0  (Timer1A)
#define BLUE_PIN  (1 << 3)    // PF3 = T1CCP1  (Timer1B)

#define PWM_PINS (RED_PIN | GREEN_PIN | BLUE_PIN)
#define MAX_PWM 255
#define PI 3.14159265f

static uint32_t period = 50000 - 1;   // 1 kHz PWM at 50 MHz clock

// ======================================================
// RGB PWM INITIALIZATION
// ======================================================
void RGB_PWM_Init(void)
{
    // 1. Enable Timer0, Timer1, and Port F clocks
    SYSCTL->RCGCTIMER |= (1 << 0) | (1 << 1);  // Timer0 + Timer1
    SYSCTL->RCGCGPIO  |= (1 << 5);             // Port F
    __asm__("NOP"); __asm__("NOP"); __asm__("NOP");

    // 2. Configure PF1, PF2, PF3 as CCP pins
    GPIOF->DIR  |= PWM_PINS;           // output mode
    GPIOF->AFSEL |= PWM_PINS;          // enable alternate function
    GPIOF->PCTL &= ~0x0000FFF0;        // clear PF1/PF2/PF3
    GPIOF->PCTL |=  0x00007770;        // assign T0CCP1, T1CCP0, T1CCP1
    GPIOF->DEN   |= PWM_PINS;          // digital enable

    // 3. Disable timers during configuration
    TIMER0->CTL = 0;
    TIMER1->CTL = 0;

    // 4. Clear prescalers
    TIMER0->TBPR = 0;
    TIMER1->TAPR = 0;
    TIMER1->TBPR = 0;

    // ---- Timer0B (PF1, Red) ----
    TIMER0->CFG   = 0x4;                 // 16-bit
    TIMER0->TBMR  = (0x2 | (1 << 3));    // periodic + PWM
    TIMER0->TBILR = period;
    TIMER0->TBMATCHR = period;           // start OFF

    // ---- Timer1A (PF2, Blue) ----
    TIMER1->CFG   = 0x4;                 // 16-bit
    TIMER1->TAMR  = (0x2 | (1 << 3));    // periodic + PWM
    TIMER1->TAILR = period;
    TIMER1->TAMATCHR = period;           // start OFF

    // ---- Timer1B (PF3, Green) ----
    TIMER1->TBMR  = (0x2 | (1 << 3));    // periodic + PWM
    TIMER1->TBILR = period;
    TIMER1->TBMATCHR = period;           // start OFF

    // 5. Enable timers
    TIMER0->CTL |= (1 << 8);               // Timer0B
    TIMER1->CTL |= (1 << 0) | (1 << 8);    // Timer1A + Timer1B
}

// ======================================================
// SET RGB COLOR: values 0–255
// ======================================================
void set_color(uint8_t r, uint8_t g, uint8_t b)
{
    // Inverted logic for PWM (common cathode)
    TIMER0->TBMATCHR = period - ((period * r) / MAX_PWM) - 1;  // Red
    TIMER1->TBMATCHR = period - ((period * g) / MAX_PWM) - 1; // Green
    TIMER1->TAMATCHR = period - ((period * b) / MAX_PWM) - 1; // Blue
}

// ======================================================
// SIMPLE FADE FUNCTION
// ======================================================

void fade_colors(void)
{
    float t = 0.0f;       // time counter
    float dt = 0.02f;     // step increment, adjust speed

    while(1)
    {
        // Calculate RGB values using phase-shifted sine waves
        uint8_t r = (uint8_t)((sinf(t) * 0.5f + 0.5f) * 255);
        uint8_t g = (uint8_t)((sinf(t + 2.0f * PI / 3.0f) * 0.5f + 0.5f) * 255);
        uint8_t b = (uint8_t)((sinf(t + 4.0f * PI / 3.0f) * 0.5f + 0.5f) * 255);

        set_color(r, g, b);

        t += dt;
        if(t > 2.0f * PI) t -= 2.0f * PI;

        // Small delay for visible fading
        for(volatile int i=0; i<30000; i++);
    }
}

// ======================================================
int main(void)
{
    RGB_PWM_Init();
    fade_colors();
}
