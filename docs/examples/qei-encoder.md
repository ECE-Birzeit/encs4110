---
outline: deep
lastUpdated: true
next: false
---
# Quadrature Encoder Interface (QEI) with Rotary Encoder

This example demonstrates how to use the TM4C123's Quadrature Encoder Interface (QEI) module to read position, direction, and velocity from a rotary encoder such as the HW-040.

## Overview

The TM4C123 has two dedicated QEI modules (QEI0 and QEI1) that automatically decode quadrature-encoded signals from rotary encoders. The QEI hardware tracks position and direction without CPU intervention, making it ideal for motor control, user interfaces (rotary knobs), and robotics applications.

**Key Features:**
- Automatic quadrature signal decoding (Phase A and Phase B)
- 32-bit position counter with configurable maximum value
- Direction detection (clockwise/counter-clockwise)
- Velocity capture using dedicated timer
- Digital input filtering for noise immunity
- Optional index pulse support for absolute positioning

## Quadrature Encoding Basics

### What is Quadrature Encoding?

Quadrature encoding uses two digital signals (Phase A and Phase B) that are 90° out of phase. By detecting which signal leads, the QEI module determines rotation direction:

```
Clockwise Rotation (Phase A leads Phase B):
Phase A:  ___┌───┐___┌───┐___
Phase B:  ______┌───┐___┌───
          →      →      →

Counter-Clockwise Rotation (Phase B leads Phase A):
Phase A:  ___┌───┐___┌───┐___
Phase B:  ┌───┐___┌───┐______
          ←      ←      ←
```

**Applications:**
- **Motor Feedback**: Track motor shaft position and speed
- **User Input**: Rotary knobs for volume, menu navigation
- **Robotics**: Wheel odometry, joint position sensing
- **CNC Machines**: Precise position control

### QEI Counting Modes

The TM4C123 QEI can count in two modes:

**Mode 1: PhA Edges Only (CAPMODE=0)**
- Counts only Phase A transitions
- Resolution: 1× (1 count per encoder pulse)

**Mode 2: PhA + PhB Edges (CAPMODE=1)** ✅ **Used in this example**
- Counts all transitions on both Phase A and Phase B
- Resolution: 4× (4 counts per encoder pulse)
- Provides maximum resolution

```
Encoder with 30 PPR (Pulses Per Revolution):
- CAPMODE=0: 30 counts/revolution
- CAPMODE=1: 120 counts/revolution (30 × 4)
```

### Velocity Measurement

The QEI module includes a dedicated timer that captures rotation speed:
1. **Load Timer** with sample period (e.g., 20ms for 50 Hz)
2. **Count edges** during this window
3. **Store count** in SPEED register when timer expires
4. **Reload and repeat** automatically

**Velocity Calculation:**
```
RPM = (SPEED × 60 × Clock) / (Load × PPR × Edges)

For HW-040 (30 PPR) at 50 Hz sampling:
RPM = (SPEED × 60 × 50MHz) / (1,000,000 × 30 × 4)
```

## Hardware Setup

### Components Required
- TM4C123GH6PM LaunchPad
- HW-040 rotary encoder module (30 PPR)
- Jumper wires

### HW-040 Rotary Encoder Pinout
| Pin | Function | Description |
|-----|----------|-------------|
| CLK | Phase A | Quadrature signal A |
| DT | Phase B | Quadrature signal B |
| SW | Switch | Push button (optional) |
| + | VCC | 3.3V or 5V power |
| GND | Ground | Ground connection |

### Wiring Connections

This example uses **QEI1** module on Port C:

| Encoder Pin | TM4C123 Pin | GPIO Function |
|-------------|-------------|---------------|
| CLK (Phase A) | **PC6** | PhA1 (QEI1 Phase A) |
| DT (Phase B) | **PC5** | PhB1 (QEI1 Phase B) |
| SW (Button) | Any GPIO | Digital input (optional) |
| + (VCC) | **3.3V** | Power |
| GND | GND | Ground |

::: warning Connection Notes
- **Always use 3.3V**, not 5V, to avoid damaging the TM4C123
- The HW-040 typically has built-in 10kΩ pull-up resistors
- If using a bare encoder, enable internal pull-ups on the TM4C123
- Keep wires short to minimize noise and signal integrity issues
:::

::: tip Alternative QEI Module
The TM4C123 also has **QEI0** available on:
- **PD6** (PhA0) and **PD7** (PhB0)

Just change the code to use `QEI0`, `GPIOD`, and adjust pin definitions.
:::

## Code Implementation

This example reads position, angle, and RPM from the encoder and outputs the data via UART every 20ms (50 Hz).

::: tip UART Dependency
This example requires `uart.h` and `uart.c` from **Experiment 9** for serial output. Make sure to include these files in your project.
:::

::: code-group
<<< @/snippets/examples/qei/main.c
<<< @/snippets/examples/qei/main.h
:::

## Understanding the Code

### 1. Clock and GPIO Initialization
```c
SYSCTL->RCGCQEI  |= (1<<1);    // Enable QEI1 module clock
SYSCTL->RCGCGPIO |= (1<<2);    // Enable Port C clock
```
Both the QEI module and GPIO port must be enabled before configuration.

### 2. GPIO Alternate Function Configuration
```c
GPIOC->AFSEL |= (PhA1 | PhB1);     // Enable alternate function
GPIOC->PCTL &= ~(0xFF << 20);      // Clear PC5 and PC6 PCTL bits
GPIOC->PCTL |=  (0x66 << 20);      // Assign QEI function (value = 6)
GPIOC->DEN |= (PhA1 | PhB1);       // Enable digital function
```
Configures **PC5** and **PC6** to receive QEI signals instead of regular GPIO.

**PCTL Calculation:**
- PC5 uses bits [23:20] = `0x6` (shifted left by 20)
- PC6 uses bits [27:24] = `0x6` (shifted left by 24)
- Combined: `0x66 << 20`

### 3. QEI Control Register Configuration
```c
QEI1->CTL = (1<<0)  |   // ENABLE: Enable QEI operation
            (1<<3)  |   // CAPMODE: Count PhA + PhB edges (4× resolution)
            (1<<5)  |   // VELEN: Enable velocity capture
            (1<<13);    // FILTEN: Enable digital filtering
```

**Control Bits Explained:**
- **ENABLE** (bit 0): Turns on the QEI module
- **CAPMODE** (bit 3): Enables 4× resolution counting
- **VELEN** (bit 5): Activates velocity timer
- **FILTEN** (bit 13): Reduces noise on encoder inputs

### 4. Position Counter Configuration
```c
QEI1->MAXPOS = CPR - 1;    // Set maximum position (120 - 1 = 119)
```
**CPR (Counts Per Revolution)** = PPR × EDGES = 30 × 4 = 120

The position counter wraps around:
- Clockwise: 0 → 1 → ... → 119 → 0
- Counter-clockwise: 0 → 119 → 118 → ... → 0

### 5. Velocity Timer Configuration
```c
QEI1->LOAD = LOAD_VAL;    // 50 MHz / 50 Hz = 1,000,000
```
Sets the velocity capture window:
- **50 Hz sampling** = 20ms window
- **LOAD_VAL** = 50,000,000 / 50 = 1,000,000 clock cycles

### 6. Reading QEI Values
```c
POS   = QEI1->POS;      // Current position (0 to CPR-1)
COUNT = QEI1->COUNT;    // Current velocity accumulator
SPEED = QEI1->SPEED;    // Last completed velocity sample
ANGLE = (POS * 360) / CPR;  // Convert to degrees
```

**Register Differences:**
- **POS**: Instantaneous position counter
- **COUNT**: Active velocity measurement (current window)
- **SPEED**: Captured velocity from previous window (used for RPM)

### 7. RPM Calculation
```c
uint32_t rpm_int = SPEED * RPM_FACTOR;
```

**RPM_FACTOR** is precomputed in `main.h`:
```c
RPM_FACTOR = (60 × Clock) / (LOAD_VAL × PPR × EDGES)
           = (60 × 50,000,000) / (1,000,000 × 30 × 4)
           = 25
```

**Formula Derivation:**
```
RPM = (Edges per Window × 60) / (PPR × Edges × Time)
    = (SPEED × 60 × Clock) / (LOAD × PPR × EDGES)
```

## Modifying QEI Parameters

### Change Encoder Resolution (PPR)

Different encoders have different pulses per revolution:

```c
#define PPR 100   // Higher resolution encoder (e.g., E6B2-CWZ6C)
#define PPR 600   // Industrial encoder
#define PPR 1024  // Precision encoder
```

**Note:** Update `CPR` and `RPM_FACTOR` automatically recalculate in `main.h`.

### Change Sampling Frequency

```c
#define SAMPLE_HZ  10    // 100ms updates - slower, less CPU
#define SAMPLE_HZ  50    // 20ms updates - balanced ✅ Used in example
#define SAMPLE_HZ  100   // 10ms updates - faster response
```

Higher sampling rates provide:
- ✅ Faster response to speed changes
- ✅ Better RPM accuracy at high speeds
- ❌ More CPU/UART overhead

### Disable Velocity Capture (Position Only)

If you only need position tracking:

```c
QEI1->CTL = (1<<0) |    // ENABLE
            (1<<3) |    // CAPMODE
            (1<<13);    // FILTEN
// Remove VELEN bit
```

### Use Index Pulse (Absolute Reference)

Some encoders have an index pulse (one pulse per revolution):

```c
// Connect index to PC4 (IDX1)
QEI1->CTL |= (1<<4);    // RESMODE: Reset on index pulse

// Reset position to 0 on each revolution
```

## Applications and Extensions

### 1. Motor Speed Control with PID
```c
// Simple proportional speed control
int target_rpm = 60;
int error = target_rpm - current_rpm;
int motor_pwm = base_speed + (error * Kp);

// Set PWM duty cycle (from PWM timer example)
TIMER1->TAMATCHR = calculate_match(motor_pwm);
```

### 2. Odometry for Mobile Robots
```c
// Track distance traveled by robot wheels
float wheel_diameter = 0.065f;  // 65mm wheels
float distance_per_count = (PI * wheel_diameter) / CPR;

int32_t left_pos = QEI0->POS;
int32_t right_pos = QEI1->POS;

float left_distance = left_pos * distance_per_count;
float right_distance = right_pos * distance_per_count;
```

### 3. User Interface Rotary Knob
```c
// Use position for menu selection
int menu_items = 5;
int selected_item = (QEI1->POS % menu_items);

// Update display when position changes
if (QEI1->POS != last_position) {
    update_display(selected_item);
    last_position = QEI1->POS;
}
```

### 4. Direction Detection
```c
// Check rotation direction
if (QEI1->STAT & (1<<1)) {
    UART0_WriteString("Clockwise\r\n");
} else {
    UART0_WriteString("Counter-Clockwise\r\n");
}
```

### 5. Position Limit Enforcement
```c
// Stop motor if position exceeds limits
#define MIN_POS 0
#define MAX_POS 90  // 90 degrees of travel

int32_t current_angle = (QEI1->POS * 360) / CPR;

if (current_angle < MIN_POS || current_angle > MAX_POS) {
    stop_motor();  // Disable motor PWM
}
```

## Troubleshooting

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| Position not changing | GPIO not configured for QEI | Check AFSEL, PCTL (value = 6), DEN settings |
| Position counting wrong direction | Phase A/B swapped | Swap PC5 and PC6 connections |
| Erratic position readings | Electrical noise | Enable FILTEN bit, add 0.1µF caps near encoder |
| Position wrapping unexpectedly | Wrong MAXPOS value | Set `MAXPOS = CPR - 1` correctly |
| SPEED always zero | Velocity capture disabled | Set VELEN bit in CTL register |
| RPM calculation incorrect | Wrong PPR or EDGES | Verify encoder specs (30 PPR for HW-040) |
| Position jumps randomly | Loose connections | Check wiring, use shorter cables |
| QEI not responding | Clock not enabled | Enable RCGCQEI before configuration |

## Key Takeaways

✅ **QEI uses dedicated hardware** - no CPU polling required  
✅ **4× resolution with CAPMODE=1** - maximum precision from encoder  
✅ **Position wraps at MAXPOS** - set to CPR - 1 for full rotations  
✅ **Velocity capture is automatic** - hardware timer handles sampling  
✅ **RPM calculation is precomputed** - multiply SPEED by RPM_FACTOR  
✅ **Digital filtering reduces noise** - enable FILTEN for reliable operation  
✅ **UART output for debugging** - monitor position, angle, and speed in real-time  

## References

- [TM4C123 Datasheet](https://www.ti.com/lit/ds/spms376e/spms376e.pdf) - Section 14 (QEI Module)
- [TM4C123 Data Sheet](https://www.ti.com/lit/ds/spms376e/spms376e.pdf) - Table 10-2 (GPIO Pins and Alternate Functions)
- Experiment 9: UART (for serial output)
- Experiment 6: Hardware Timers (for velocity timer concepts)