---
outline: deep
lastUpdated: true
---

# Experiment 6: Timers and Timing Control

## Introduction

Time is everything in embedded systems. Whether you're blinking an LED, reading sensors, or creating delays, precise timing control is essential. The TM4C123 provides two powerful timing mechanisms:

1. **SysTick Timer** - A simple, built-in 24-bit timer that's part of the ARM Cortex-M4 core
2. **General Purpose Timer Module (GPTM)** - Sophisticated peripheral timers with advanced features

In this experiment, you'll learn to use both timer types, understand their strengths and limitations, and apply them to real-world scenarios.

## Part 1: Understanding SysTick Timer

### What is SysTick?

The SysTick timer is a 24-bit countdown timer integrated into every ARM Cortex-M processor. It's designed to provide a consistent time base for operating systems and applications.

**Key Features:**
- 24-bit countdown timer (0 to 16,777,215)
- Automatic reload when it reaches zero
- Can generate interrupts on timeout
- Clocked directly from the system clock
- Only three registers to configure

**SysTick Registers:**
```c
SysTick->LOAD   // Reload value (0 to 0xFFFFFF)
SysTick->VAL    // Current countdown value
SysTick->CTRL   // Control and status register
```

### SysTick Timing Calculations

The timing formula for SysTick is straightforward:

$$
\text{Time Period (seconds)} = \frac{\text{LOAD Value} + 1}{\text{System Clock Frequency}}
$$

**Example:** For a 50 MHz system clock and 1 ms delay:
- Required ticks = 50 MHz × 0.001 s = 50,000 ticks
- LOAD value = 50,000 - 1 = 49,999

### SysTick Programming Methods

**Method 1: Manual Configuration**
```c
void SysTick_Init(uint32_t ticks) {
    SysTick->LOAD = ticks - 1;      // Set reload value
    SysTick->VAL = 0;               // Clear current value
    SysTick->CTRL = 0x05;           // Enable timer, use processor clock, no interrupt
}

void SysTick_Delay_ms(uint32_t delay_ms) {
    SysTick_Init(50000);            // Configure for 1 ms at 50 MHz
    for(uint32_t i = 0; i < delay_ms; i++) {
        while((SysTick->CTRL & 0x10000) == 0); // Wait for COUNTFLAG
    }
}
```

**Method 2: CMSIS Function (Recommended)**
```c
#include "core_cm4.h"

volatile uint32_t msTicks = 0;

void SysTick_Handler(void) {
    msTicks++;                      // Increment millisecond counter
}

int main(void) {
    // Configure SysTick for 1 ms interrupts
    if (SysTick_Config(SystemCoreClock / 1000)) {
        // Handle configuration error
        while(1);
    }
    
    while(1) {
        // Your main code here
        // msTicks provides millisecond timebase
    }
}
```

## Part 2: General Purpose Timer Module (GPTM)

### GPTM Overview

The TM4C123 includes six 32-bit GPTM blocks (Timer0-Timer5) plus six 64-bit wide timers. Each GPTM can be configured as:
- Two 16-bit timers (Timer A and Timer B)
- One 32-bit timer
- Various specialized modes (PWM, input capture, etc.)

**Key Advantages:**
- Longer timing ranges with prescalers
- Multiple operating modes
- Independent Timer A and Timer B
- Advanced interrupt handling
- PWM generation capabilities

### GPTM Timing Calculations

For periodic mode with prescaler:

$$
\text{Time Period} = \frac{(\text{TAILR} + 1) \times (\text{TAPR} + 1)}{\text{System Clock}}
$$

Where:
- TAILR = Timer A Interval Load Register
- TAPR = Timer A Prescale Register

### GPTM Configuration Steps

1. **Enable Timer Clock**
```c
SYSCTL->RCGCTIMER |= (1 << timer_number);
```

2. **Disable Timer During Configuration**
```c
TIMER1->CTL = 0x00;
```

3. **Configure Timer Mode**
```c
TIMER1->CFG = 0x00;     // 32-bit mode
// or
TIMER1->CFG = 0x04;     // 16-bit mode
```

4. **Set Periodic Mode**
```c
TIMER1->TAMR = 0x02;    // Periodic mode
```

5. **Configure Timing Values**
```c
TIMER1->TAILR = reload_value;
TIMER1->TAPR = prescaler_value;
```

6. **Enable Interrupts (if needed)**
```c
TIMER1->IMR |= 0x01;           // Enable timeout interrupt
NVIC->ISER[0] |= (1 << 21);    // Enable Timer1A in NVIC
```

7. **Start Timer**
```c
TIMER1->CTL |= 0x01;
```

### Complete GPTM Example

<<< @/snippets/timers.c

## Part 3: Choosing the Right Timer

| Requirement | SysTick | GPTM |
|-------------|---------|------|
| Simple delays | ✅ Excellent | ⚠️ Overkill |
| System heartbeat | ✅ Perfect | ❌ Unnecessary |
| Long delays (>300ms) | ❌ Limited | ✅ Excellent |
| Multiple timers | ❌ Only one | ✅ Up to 12 |
| PWM generation | ❌ Not suitable | ✅ Built-in |
| Input capture | ❌ Not possible | ✅ Supported |
| Code complexity | ✅ Very simple | ⚠️ More complex |

## Practical Exercises

### Exercise 1: SysTick LED Blinker
Create a program that blinks the onboard LED using SysTick timer with different intervals:
- Red LED: 250 ms
- Blue LED: 500 ms  
- Green LED: 1000 ms

### Exercise 2: GPTM Long Delay
Use Timer1 to create a 5-second periodic interrupt that toggles an LED. Calculate the required prescaler and load values.

### Exercise 3: Multi-Timer System
Implement a system using both SysTick and GPTM:
- SysTick: 1 ms timebase for button debouncing
- Timer0: 2-second heartbeat LED
- Timer1: 10-second status update

### Exercise 4: Timing Accuracy Comparison
Compare the accuracy of 1-second delays using:
1. SysTick with counting method
2. GPTM Timer1 interrupt
3. Software delay loops

Measure actual timing with an oscilloscope or logic analyzer.

### Exercise 5: Real-Time Clock
Create a simple real-time clock that displays seconds, minutes, and hours:
- Use SysTick for 1 ms timebase
- Update display every second
- Implement time setting via push buttons

## Advanced Topics

### Timer Concatenation
Combine Timer A and Timer B for longer timing ranges:
```c
// Configure for 64-bit operation
TIMER0->CFG = 0x00;         // 32-bit mode
// Use both TAILR and TBILR for full 64-bit range
```

### Prescaler Usage
Extend timing range without using 32-bit mode:
```c
// 16-bit timer with 8-bit prescaler gives 24-bit range
TIMER1->CFG = 0x04;         // 16-bit mode
TIMER1->TAPR = 255;         // Maximum prescaler
TIMER1->TAILR = 65535;      // Maximum 16-bit value
```

### Power Considerations
- SysTick continues running in sleep modes
- GPTM can be configured to wake from sleep
- Consider timer usage in low-power applications

## Troubleshooting Common Issues

1. **Timer not starting**: Check clock enable and control register
2. **Wrong timing**: Verify clock frequency and calculation
3. **Interrupts not firing**: Check NVIC enable and priority
4. **Jittery timing**: Ensure interrupt handlers are kept short

## Summary

This experiment introduced you to the two main timing mechanisms in the TM4C123:

**SysTick Timer:**
- Simple 24-bit countdown timer
- Perfect for system heartbeat and simple delays
- Easy to configure with CMSIS functions
- Limited to ~300 ms maximum delay at 50 MHz

**General Purpose Timer Module (GPTM):**
- Flexible 16/32-bit timers with advanced features
- Suitable for long delays, PWM, and input capture
- More complex configuration but greater capabilities
- Multiple independent timers available

Choose SysTick for simplicity and GPTM for flexibility. Understanding both gives you the tools to handle any timing requirement in embedded systems.


