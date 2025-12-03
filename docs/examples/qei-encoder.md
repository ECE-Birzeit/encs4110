---
outline: deep
lastUpdated: true
next: false
---
# Quadrature Encoder Interface (QEI) with Rotary Encoder

This example demonstrates how to use the TM4C123's Quadrature Encoder Interface (QEI) module to read position and direction from a rotary encoder such as the HW-040 or KY-040.

## Overview

The QEI module automatically decodes quadrature-encoded signals from rotary encoders, tracking both position and direction without CPU intervention. This is ideal for motor control, user interfaces (rotary knobs), and robotics applications.

## Hardware Setup

### Components Required
- TM4C123GH6PM LaunchPad
- HW-040 rotary encoder module
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

The TM4C123 has two QEI modules (QEI0 and QEI1). For this example, we'll use **QEI0**:

| Encoder Pin | TM4C123 Pin | GPIO Function |
|-------------|-------------|---------------|
| CLK (Phase A) | PC5 | PhA1 (QEI1 Phase A) |
| DT (Phase B) | PC6 | PhB1 (QEI1 Phase B) |
| SW (Button) | PD2 | GPIO Input (optional) |
| + (VCC) | 3.3V | Power |
| GND | GND | Ground |

::: warning Connection Notes
- **Use 3.3V**, not 5V, to avoid damaging the TM4C123
- The HW-040 typically has built-in pull-up resistors
- If using a bare encoder without pull-ups, enable internal pull-ups on PD6/PD7
:::

## How Quadrature Encoding Works

Rotary encoders generate two square wave signals (Phase A and Phase B) that are 90° out of phase:

```
Clockwise Rotation:
Phase A:  ___┌───┐___┌───┐___
Phase B:  ______┌───┐___┌───

Counter-Clockwise Rotation:
Phase A:  ___┌───┐___┌───┐___
Phase B:  ┌───┐___┌───┐______
```

The QEI module detects these phase relationships and automatically:
- **Increments** the position counter for clockwise rotation
- **Decrements** the position counter for counter-clockwise rotation
- Detects rotation **speed** based on edge timing


## Code Implementation

::: tip Connection Notes
- Don't forget to include the `uart.h` and `uart.c` files from Experiment 9 for serial output.
:::


::: code-group
<<< @/snippets/examples/qei/main.c
<<< @/snippets/examples/qei/main.h
:::