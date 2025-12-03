#ifndef MAIN_H
#define MAIN_H

#include "TM4C123.h"
#include <stdint.h>

// System clock frequency
#define SystemCoreClock 50000000u
#define CYCLES_PER_US   (SystemCoreClock / 1000000u)

// QEI Pin definitions
#define PhA1 (1 << 6)  // PC6
#define PhB1 (1 << 5)  // PC5

// Encoder & QEI constants
#define CLOCK      50000000UL    // 50 MHz system clock
#define VELDIV     0             // no predivider
#define SAMPLE_HZ  50            // velocity sample frequency (Hz)
#define LOAD_VAL   (CLOCK / SAMPLE_HZ)  // 20ms window
#define PPR        30            // HW-040 pulses per rev
#define EDGES      4             // CAPMODE=1 counts both PhA+PhB edges
#define CPR        (PPR * EDGES) // Counts per revolution

// Precomputed RPM factor (integer-safe)
#define RPM_FACTOR ((60UL * (1UL << VELDIV) * CLOCK) / (LOAD_VAL * PPR * EDGES))

// Inline implementations for delay functions
static inline void SysTick_Init(void)
{
    SysTick->CTRL = 0;
    SysTick->LOAD = CYCLES_PER_US - 1;  // 1us delay at 50MHz
    SysTick->VAL = 0;
    SysTick->CTRL = 0x5;     // Enable with system clock
}

static inline void delay_us(int us)
{
    SysTick->LOAD = (CYCLES_PER_US * us) - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = 0x5; // Enable with system clock
    while ((SysTick->CTRL & 0x10000) == 0);
    SysTick->CTRL = 0;
}

static inline void delay_ms(int ms)
{
    while (ms--)
        delay_us(1000);
}

#endif // MAIN_H
