---
outline: deep
lastUpdated: true
next: true
---
# PWM Signal Generation Using Timers

This example demonstrates how to generate PWM (Pulse Width Modulation) signals using the TM4C123's general-purpose timers in PWM mode to control LED brightness or motor speed.

## Overview

The TM4C123 has six 16/32-bit general-purpose timers (Timer0-Timer5), each with two timer blocks (A and B) that can be configured for PWM output. PWM mode allows precise control of average power delivered to devices like LEDs, motors, and servos by varying the duty cycle.

**Key Features:**
- Independent PWM control on each timer block (TimerA and TimerB)
- Configurable frequency and duty cycle
- Hardware-based signal generation (no CPU overhead)
- Direct connection to GPIO pins via alternate functions

## PWM Basics

### What is PWM?

PWM is a technique that creates an analog-like output using a digital signal by rapidly switching between HIGH and LOW states:

```
Duty Cycle = (Ton / Period) × 100%

High Duty Cycle (75%):
  ████████████░░░░████████████░░░░
  |--- Ton ---|   |--- Ton ---|

Low Duty Cycle (25%):
  ███░░░░░░░░░░███░░░░░░░░░░
  |-|           |-|

Period = Ton + Toff
Frequency = 1 / Period
```

**Applications:**
- **LED Brightness Control**: Higher duty cycle = brighter LED
- **Motor Speed Control**: Higher duty cycle = faster rotation
- **Servo Position Control**: Duty cycle determines angle (1-2ms pulses)
- **Audio Generation**: Varying frequency creates tones

### Timer PWM Mode Operation

In PWM mode, the timer:
1. **Counts down** from TAILR (load value) to 0
2. **Reloads** automatically when reaching 0 (periodic mode)
3. **Outputs HIGH** when counter > TAMATCHR (match value)
4. **Outputs LOW** when counter ≤ TAMATCHR


**Key Registers:**
- **TAILR**: Load value (sets PWM period)
- **TAMATCHR**: Match value (sets duty cycle)
- **Duty Cycle** = `(TAILR - TAMATCHR) / TAILR`

## Hardware Setup

### Components Required
- TM4C123GH6PM LaunchPad
- LED (or use onboard RGB LED on PF1/PF2/PF3)
- 330Ω resistor (if using external LED)
- Breadboard and jumper wires

### Timer PWM Pin Mapping

Each timer can output PWM on specific GPIO pins via alternate functions:

| Timer | Pin | Alternate Function | PCTL Value |
|-------|-----|-------------------|------------|
| Timer0A | PB6, PF0 | T0CCP0 | 7 |
| Timer0B | PB7, PF1 | T0CCP1 | 7 |
| Timer1A | **PF2**, PB4 | **T1CCP0** | **7** |
| Timer1B | PF3, PB5 | T1CCP1 | 7 |
| Timer2A | PB0, PF4 | T2CCP0 | 7 |
| Timer2B | PB1 | T2CCP1 | 7 |
| Timer3A | PB2 | T3CCP0 | 7 |
| Timer3B | PB3 | T3CCP1 | 7 |

::: tip Using Onboard RGB LED
The TM4C123 LaunchPad has an RGB LED connected to:
- **PF1** (Red) - Can use Timer0B (alternate mapping)
- **PF2** (Blue) - Timer1A (T1CCP0) ✅ **Used in this example**
- **PF3** (Green) - Timer1B (T1CCP1)

No external components needed when using these pins!
:::

### Wiring for External LED

If using an external LED on **PF2**:

| Component | Connection |
|-----------|------------|
| LED Anode (+) | 330Ω resistor → PF2 |
| LED Cathode (-) | GND |

## Code Implementation

### Single LED Example

This example uses **Timer1A** to generate PWM on **PF2** (blue LED) with a 33% duty cycle at 1 kHz frequency.

### RGB LED Example

The RGB example demonstrates controlling all three colors (Red, Green, Blue) simultaneously using three timers:
- **Timer0B** (PF1) - Red LED
- **Timer1A** (PF2) - Blue LED  
- **Timer1B** (PF3) - Green LED

The `set_color()` function allows easy color mixing by accepting RGB values from 0 to 255.

::: code-group
<<< @/snippets/examples/pwm_timer/single_led.c
<<< @/snippets/examples/pwm_timer/rgb_leds.c
:::

## Understanding the Code

### 1. Clock Initialization
```c
SYSCTL->RCGCTIMER |= (1 << 1);   // Enable Timer1 clock
SYSCTL->RCGCGPIO  |= (1 << 5);   // Enable Port F clock
```
Timers and GPIO ports must have their clocks enabled before use.

### 2. GPIO Alternate Function Configuration
```c
GPIOF->AFSEL |= PWM_PIN;         // Enable alternate function on PF2
GPIOF->PCTL &= ~(0xF << 8);      // Clear PCTL bits [11:8]
GPIOF->PCTL |=  (0x7 << 8);      // Set T1CCP0 function (value = 7)
GPIOF->DEN   |= PWM_PIN;         // Enable digital function
```
Configures **PF2** to output Timer1A's PWM signal instead of regular GPIO.

### 3. Timer Configuration
```c
TIMER1->CFG  = 0x4;              // 16-bit mode (split timer)
TIMER1->TAMR = (0x2 | (1 << 3)); // Periodic mode + PWM enable
```
- **CFG = 0x4**: Splits the 32-bit timer into two independent 16-bit timers (A and B)
- **TAMR = 0x0A**: Sets periodic countdown mode with PWM output enabled

### 4. PWM Period (Frequency)
```c
uint32_t period = 50000 - 1;     // 1 kHz at 50 MHz system clock
TIMER1->TAILR = period;
```
**Calculation:**
```
PWM Frequency = System Clock / (TAILR + 1)
1 kHz = 50 MHz / 50000
Period = (50 MHz / 1 kHz) - 1 = 49999
```

To change frequency, adjust `period`:
- **10 kHz**: `period = 5000 - 1`


### 5. Duty Cycle (Brightness)
```c
uint32_t match = (uint32_t)(period * (1.0f - DUTY_CYCLE));
TIMER1->TAMATCHR =  - 1;
```
**Calculation:**
```
TAMATCHR = TAILR × (1 - Duty Cycle)
For 33% duty cycle:
TAMATCHR = 49999 × (1 - 0.33) = 33499
```

**Important:** The formula uses `(1 - Duty Cycle)` because the timer counts **down** and outputs HIGH when counter > MATCH.

## RGB LED Control

### Timer Configuration for RGB

The RGB example uses three independent timer blocks to control each color channel:

| LED Color | Pin | Timer | Configuration Registers |
|-----------|-----|-------|------------------------|
| Red | PF1 | Timer0B | TBMR, TBILR, TBMATCHR |
| Green | PF2 | Timer1A | TAMR, TAILR, TAMATCHR |
| Blue | PF3 | Timer1B | TBMR, TBILR, TBMATCHR |

### Understanding the RGB Code

#### 1. Multi-Timer Initialization
```c
// Enable both Timer0 and Timer1
SYSCTL->RCGCTIMER |= (1 << 0) | (1 << 1);

// Configure all three pins with their respective timer functions
GPIOF->PCTL |= 0x00007770;  // PF1=T0CCP1, PF2=T1CCP0, PF3=T1CCP1
```

#### 2. Setting RGB Colors (0-255)
The `set_color(r, g, b)` function converts 8-bit RGB values (0-255) to PWM match values:

```c
void set_color(uint8_t r, uint8_t g, uint8_t b)
{
    TIMER0->TBMATCHR = period - ((period * r) / MAX_PWM) - 1;  // Red
    TIMER1->TBMATCHR = period - ((period * g) / MAX_PWM) - 1;  // Green
    TIMER1->TAMATCHR = period - ((period * b) / MAX_PWM) - 1;  // Blue
}
```

**Example Usage:**
```c
set_color(255, 0, 0);    // Pure red
set_color(0, 255, 0);    // Pure green
set_color(0, 0, 255);    // Pure blue
set_color(255, 255, 0);  // Yellow (red + green)
set_color(0, 255, 255);  // Cyan (green + blue)
set_color(255, 0, 255);  // Magenta (red + blue)
set_color(255, 255, 255);// White (all colors)
set_color(128, 128, 128);// Gray (50% brightness)
```

#### 3. Smooth Color Fading with Sine Waves
The `fade_colors()` function creates a smooth rainbow effect using phase-shifted sine waves:

```c
// Three sine waves, 120° apart (2π/3 radians)
uint8_t r = (sinf(t) * 0.5f + 0.5f) * 255;                    // Red phase
uint8_t g = (sinf(t + 2.0f * PI / 3.0f) * 0.5f + 0.5f) * 255; // Green phase (+120°)
uint8_t b = (sinf(t + 4.0f * PI / 3.0f) * 0.5f + 0.5f) * 255; // Blue phase (+240°)
```

This creates a continuous color wheel effect:
- `sinf(t)` oscillates between -1 and +1
- `* 0.5f + 0.5f` scales to 0.0 to 1.0 range
- `* 255` converts to 0-255 byte range
- Phase shifts ensure colors blend smoothly

## Modifying PWM Parameters

### Change Duty Cycle (Brightness)

For the single LED example:
```c
#define DUTY_CYCLE 0.10f   // 10% - very dim
#define DUTY_CYCLE 0.50f   // 50% - medium brightness
#define DUTY_CYCLE 0.90f   // 90% - very bright
```

For RGB LEDs, adjust individual color intensities:
```c
set_color(25, 0, 0);    // Dim red (10% of 255)
set_color(128, 0, 0);   // Medium red (50% of 255)
set_color(230, 0, 0);   // Bright red (90% of 255)
```



## References

- [TM4C123 Datasheet](https://www.ti.com/lit/ds/spms376e/spms376e.pdf) - Section 11 (General-Purpose Timers)
- [TM4C123 Data Sheet](https://www.ti.com/lit/ds/spms376e/spms376e.pdf) - Table 10-2 (GPIO Pins and Alternate Functions)
- Experiment 6: Hardware Timers (for timer basics)
- Experiment 4: GPIO Configuration (for pin setup)
