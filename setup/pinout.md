---
title: "Board Pinout"
short: "Board Pinout"
order: 0
description: "The EK-TM4C123GXL LaunchPad's J1-J4 headers from above and below, what the board already wires to its pins, which pins are free, and which pins each experiment uses."
---

# Board Pinout

One page to check before you wire anything: where each pin is on the LaunchPad's four headers, what the board already connects to its pins, which pins are free, and which pins every experiment uses. The full details are in TI's [EK-TM4C123GXL user's guide (SPMU296)](https://www.ti.com/lit/ug/spmu296/spmu296.pdf) and the [TM4C123GH6PM datasheet](/datasheets/tm4c123gh6pm.pdf){target="_blank"}.

## The headers

The LaunchPad has four 10-pin headers, J1 to J4, in two pairs along its long edges. With the debug USB connector at the top, **J1** (outer) and **J3** (inner) are on the left, and **J4** (inner) and **J2** (outer) on the right. Pin 1 of every header is at the USB end.

### Top view (component side)

Use this when you look at the board the normal way up, LEDs and buttons facing you.

[![EK-TM4C123GXL J1 to J4 header pinout, top view](/images/pinout/launchpad-top.svg)](/images/pinout/launchpad-top.svg){target="_blank"}

### Bottom view (solder side)

Use this when you wire from underneath, for example with female jumper wires on the header pins below the board. The board is flipped left-to-right: J2 and J4 are now on the left, J3 and J1 on the right.

[![EK-TM4C123GXL J1 to J4 header pinout, bottom view](/images/pinout/launchpad-bottom.svg)](/images/pinout/launchpad-bottom.svg){target="_blank"}

<small>Click a drawing to open it full size, or download the [top view](/images/pinout/launchpad-top.svg){target="_self" download=""} and [bottom view](/images/pinout/launchpad-bottom.svg){target="_self" download=""}. Drawn from SPMU296's BoosterPack connector tables by `python3 scripts/draw-pinout.py`.</small>

## Wired on the board

These pins already connect to something on the LaunchPad. Use them for that part, or check the note before reusing them.

| Part | Pins | Note |
| --- | --- | --- |
| RGB LED (red, blue, green) | PF1, PF2, PF3 | Active high. Also the timer outputs T0CCP1, T1CCP0, T1CCP1 ([Experiment 11](/experiments/exp11-pwm-timers)). |
| SW1, SW2 | PF4, PF0 | Active **low**: they connect to GND, so enable the pull-ups. PF0 is locked; unlock it first ([Experiment 6](/experiments/exp6-gpio-interrupts)). |
| Virtual COM port (UART0) | PA0 (U0Rx), PA1 (U0Tx) | Go to the debugger's USB link, not to the headers ([Experiment 10](/experiments/exp10-uart)). |
| JTAG debug | PC0 to PC3 | Not on the headers. Never reconfigure them: the debugger loses the board. |
| USB device connector | PD4, PD5 | Not on the headers; 3.6 V max. |
| R9 and R10 | PB6 = PD0, PB7 = PD1 | Two 0 Ω resistors join these pairs, for compatibility with older BoosterPacks. Use one pin of each pair, or remove the resistor. |

::: danger Never touch PC0 to PC3
They carry JTAG, the connection Keil uses to flash and debug the board. Change Port C only with `|=`, `&= ~`, or masked writes. If the board stops accepting new programs, use **LM Flash Programmer → Other Utilities → Debug Port Unlock** to recover it.
:::

## Free for your circuits

Nothing on the board connects to these, so they're the safest picks for LEDs, buttons, and sensors:

| Port | Free pins on the headers |
| --- | --- |
| A | PA2 to PA7 |
| B | PB0 to PB5 (and PB6, PB7 if PD0, PD1 stay unused) |
| C | PC4 to PC7 |
| D | PD2, PD3, PD6, PD7 (PD7 is locked like PF0) |
| E | PE0 to PE5 |

Free pins with ADC inputs: PE3 (AIN0), PE2 (AIN1), PE1 (AIN2), PE0 (AIN3), PD3 (AIN4), PD2 (AIN5), PE5 (AIN8), PE4 (AIN9), PB4 (AIN10), PB5 (AIN11). PD0 and PD1 are AIN7 and AIN6, minding R9 and R10.

## Pins used in each experiment

| Experiment | Signal | Pins |
| --- | --- | --- |
| [5. GPIO Output](/experiments/exp5-gpio-output) | RGB LED | PF1, PF2, PF3 |
| [6. GPIO Interrupts](/experiments/exp6-gpio-interrupts) | SW1, SW2; LEDs | PF4, PF0; PF1 to PF3 |
| [7. Timers](/experiments/exp7-timers) | LEDs, SW1 | PF1 to PF3, PF4 |
| [8. Parallel LCD](/experiments/exp8-lcd) | RS, RW, E; D4 to D7 | PB0, PB1, PB2; PB4 to PB7 |
| [9. ADC](/experiments/exp9-adc) | Potentiometer | PE3 (AIN0) |
| [10. UART](/experiments/exp10-uart) | Virtual COM port | PA0, PA1 (on board, no wiring) |
| [11. PWM](/experiments/exp11-pwm-timers) | RGB LED by PWM; potentiometer (Task) | PF1, PF2, PF3; PE3 |
| [12. QEI](/experiments/exp12-qei) | HW-040 CLK, DT, SW | PC5 (PhA1), PC6 (PhB1), PC4 |
| [13. I2C LCD](/experiments/exp13-i2c-lcd) | SCL, SDA | PB2, PB3 |

::: tip PB6 and PB7 in Experiment 8
The parallel LCD's D6 and D7 are on PB6 and PB7, which R9 and R10 join to PD0 and PD1. That's fine as long as nothing drives PD0 or PD1: they're inputs after reset, so leave them alone.
:::
