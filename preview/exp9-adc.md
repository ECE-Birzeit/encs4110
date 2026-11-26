---
title: "Analog-to-Digital Converter (ADC)"
description: "This experiment introduces analog-to-digital conversion using the TM4C123’s dual 12-bit ADC modules."
outline: deep
lastUpdated: true
manual: "/manual/encs4110_manual.pdf#nameddest=chapter.9"
datasheet: "/datasheets/tm4c123gh6pm.pdf#page=799"
current: "/experiments/exp9-adc"
short: "ADC"
search: false
prev: false
next: false
head:
  - - meta
    - name: robots
      content: noindex
---

# Analog-to-Digital Converter (ADC)

This experiment introduces analog-to-digital conversion using the TM4C123’s dual 12-bit ADC modules. You will configure sample sequencers for automatic multi-channel data acquisition, implement both polling and interrupt-driven sampling modes, and interface with analog sensors such as potentiometers and the internal temperature sensor. By the end of this lab, you will understand how to convert continuous analog signals to discrete digital values, configure trigger sources for periodic sampling, and implement sensor interfacing applications that bridge the analog and digital domains.

::: warning Preview: new format
This is the reworked version of [Experiment 9](/experiments/exp9-adc), shared for TAs to test. It has the same background, then one Activity built step by step and one Task. **Students: use the [current version](/experiments/exp9-adc)** until this one replaces it.
:::


## Objectives

- Understand analog-to-digital conversion principles including sampling, quantization, and resolution.
- Recognize the architecture of the TM4C123 ADC modules including dual 12-bit converters and shared input channels.
- Configure ADC sample sequencers (SS0-SS3) with different depths and trigger sources.
- Configure GPIO pins for analog input using AFSEL, AMSEL, and DEN registers.
- Implement software-triggered and hardware-triggered ADC conversions.
- Read and interpret ADC conversion results from FIFO registers.
- Convert digital ADC values to physical quantities such as voltage and temperature.
- Implement both polling and interrupt-driven ADC operation modes.

## Background

### Introduction to Analog-to-Digital Conversion

Analog-to-Digital Converters (ADCs) are essential components in embedded systems, enabling microcontrollers to interface with the analog world by converting continuous analog signals (such as voltage from sensors) into discrete digital values for processing. They are widely used in applications like temperature sensing, light intensity measurement, and audio signal processing.

#### ADC Resolution and Sampling

ADCs are characterized by their **resolution**, which indicates the number of discrete levels they can represent, typically expressed in bits. For instance, a 12-bit ADC can represent $2^{12} = 4096$ discrete levels. The resolution determines the smallest detectable change in the analog input, known as the **least significant bit (LSB)**. The voltage corresponding to one LSB is calculated as:
$$
\text{LSB Voltage} = \frac{V_{\text{REFP}} - V_{\text{REFN}}}{2^{\text{Resolution}}}
$$
where $V_{\text{REFP}}$ is the positive reference voltage, $V_{\text{REFN}}$ is the negative reference voltage, and is the number of bits.

### TM4C123 ADC Architecture

The TM4C123GH6PM microcontroller features **two identical 12-bit ADC modules** (ADC0 and ADC1) that share 12 analog input channels. These modules provide high-precision conversion of continuous analog voltages to discrete digital numbers.

#### Key Features

The ADC modules offer the following capabilities:

- **Resolution**: 12-bit precision (0 to 4095 discrete levels)
- **Shared Input Channels**: 12 analog input channels (AIN0-AIN11) shared between both modules
- **Internal Temperature Sensor**: On-chip temperature measurement capability
- **Maximum Sample Rate**: 1 million samples per second (1 MSPS)
- **Reference Voltage**: VDDA = 3.3V (separate analog power supply)
- **Input Voltage Range**: 0V to 3.3V for single-ended inputs
- **Sample Sequencers**: Four programmable sequencers per module (SS0-SS3) with depths of 8, 4, 4, and 1 samples
- **Hardware Averaging**: Automatic averaging of up to 64 samples for noise reduction
- **Digital Comparators**: Eight digital comparators per module for threshold detection
- **DMA Support**: Dedicated μDMA channels for efficient data transfer with burst requests
- **Isolated Power**: Separate analog power and ground pins for improved signal integrity

#### ADC Input Channels

The TM4C123 provides 12 analog input channels (AIN0 to AIN11) that can be connected to various GPIO pins, shared between both ADC modules. The mapping is as follows:

| Channel | AIN0 | AIN1 | AIN2 | AIN3 | AIN4 | AIN5 | AIN6 | AIN7 | AIN8 | AIN9 | AIN10 | AIN11 |
|:--------|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:----:|:-----:|:-----:|
| Pin     | PE3  | PE2  | PE1  | PE0  | PD3  | PD2  | PD1  | PD0  | PE5  | PE4  |  PB4  |  PB5  |

*ADC Input Channel Pin Mapping*

#### Sample Sequencers

Each ADC module has **four independent sample sequencers** that manage sampling control and data capture autonomously. They differ only in the number of samples and FIFO depth.

| **Sequencer** | **Number of Samples** | **FIFO Depth** | **Priority** |
|:-------------:|:---------------------:|:--------------:|:------------:|
|      SS0      |           8           |   8 entries    |   Highest    |
|      SS1      |           4           |   4 entries    |     High     |
|      SS2      |           4           |   4 entries    |     Low      |
|      SS3      |           1           |    1 entry     |    Lowest    |

*Sample Sequencer Capabilities and FIFO Depths*

#### Trigger Sources

ADC conversions can be initiated by various trigger sources for flexibility:

- **Processor (Software)**: Manual trigger via ADCPSSI register
- **Analog Comparators**: Trigger on comparator output events
- **GPIO**: External signal on GPIO pins
- **General-Purpose Timers**: Periodic sampling synchronized with timer events
- **PWM Generators**: Synchronize sampling with PWM signals for motor control
- **Continuous**: Free-running continuous sampling mode

### Internal Temperature Sensor

The internal temperature sensor can be sampled by setting the TSn bit in the ADCSSCTLn register. Temperature (TEMP in °C) is calculated from the ADC reading (ADCCODE, 0 to 4095) as:
$$
\text{TEMP} = 147.5 - \left( \frac{75 \times (V_{\text{REFP}} - V_{\text{REFN}}) \times \text{ADCCODE}}{4096} \right)
$$

**Note**: This measures the microcontroller’s die temperature, not ambient, and is useful for thermal management but may require calibration.

### ADC Registers

The TM4C123 ADC modules are configured via registers. Refer to the datasheet for details. Key registers include:

#### RCGCADC — ADC Run Mode Clock Gating Control

Enables/disables clock for ADC modules. Bits correspond to modules (1 = enable, 0 = disable).

**RCGCADC Register — ADC Run Mode Clock Gating Control**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–2 | reserved |
| 1 | ADC1 |
| 0 | ADC0 |

#### ADCACTSS — ADC Active Sample Sequencer

Enables/disables sample sequencers. Bits correspond to sequencers (1 = enable, 0 = disable). BUSY indicates ADC status (1 = busy, 0 = idle).

**ADCACTSS Register — ADC Active Sample Sequencer**

| Bits | Field |
| --- | --- |
| 31–17 | reserved |
| 16 | BUSY |
| 15–4 | reserved |
| 3 | SS3 |
| 2 | SS2 |
| 1 | SS1 |
| 0 | SS0 |

#### ADCSSMUXn — ADC Sample Sequence Input Multiplexer Select

Selects input channels for each sample in a sequencer. Each 4-bit field (MUX0-MUX7) specifies the channel (0-11 for AIN0-AIN11).

**ADCSSMUXn Register — ADC Sample Sequence Input Multiplexer Select**

| Bits | Field |
| --- | --- |
| 31–28 | MUX7 |
| 27–24 | MUX6 |
| 23–20 | MUX5 |
| 19–16 | MUX4 |
| 15–12 | MUX3 |
| 11–8 | MUX2 |
| 7–4 | MUX1 |
| 3–0 | MUX0 |

#### ADCSSCTLn — ADC Sample Sequence Control

Configures each sample in a sequence. The END bit must be set for the final sample. This 32-bit register handles up to eight samples.

**ADCSSCTLn Register — ADC Sample Sequence Control**

| Bits | Field |
| --- | --- |
| 31 | TS7 |
| 30 | IE7 |
| 29 | END7 |
| 28 | D7 |
| 27 | TS6 |
| 26 | IE6 |
| 25 | END6 |
| 24 | D6 |
| 23 | TS5 |
| 22 | IE5 |
| 21 | END5 |
| 20 | D5 |
| 19 | TS4 |
| 18 | IE4 |
| 17 | END4 |
| 16 | D4 |
| 15 | TS3 |
| 14 | IE3 |
| 13 | END3 |
| 12 | D3 |
| 11 | TS2 |
| 10 | IE2 |
| 9 | END2 |
| 8 | D2 |
| 7 | TS1 |
| 6 | IE1 |
| 5 | END1 |
| 4 | D1 |
| 3 | TS0 |
| 2 | IE0 |
| 1 | END0 |
| 0 | D0 |

*Where:*

- TSx: Temperature sensor select (1 = sample temperature sensor)
- IEx: Interrupt enable (1 = enable interrupt on sample completion)
- ENDx: End of sequence (1 = last sample in sequence)
- Dx: Differential select (1 = differential input, 0 = single-ended)
- x: Sample index (0-7)

#### ADCEMUX — ADC Event Multiplexer Select

Selects trigger source for each sample sequencer. Each 4-bit field (EM0-EM3) specifies the trigger source.

**ADCEMUX Register — ADC Event Multiplexer Select**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–12 | EM3 |
| 11–8 | EM2 |
| 7–4 | EM1 |
| 3–0 | EM0 |

*The EMx field specifies the trigger source for sample sequencer x:*

| **Value** | **Trigger Source**                                 |
|:---------:|:---------------------------------------------------|
|    0x0    | Processor (software trigger via ADCPSSI register)  |
|    0x1    | Analog Comparator 0                                |
|    0x2    | Analog Comparator 1                                |
|    0x3    | Reserved                                           |
|    0x4    | External GPIO (connected to GPIO interrupt)        |
|    0x5    | Timer (requires TnOTE bit set in GPTMCTL register) |
|    0x6    | PWM Generator 0                                    |
|    0x7    | PWM Generator 1                                    |
|    0x8    | PWM Generator 2                                    |
|    0x9    | PWM Generator 3                                    |
|  0xA–0xE  | Reserved                                           |
|    0xF    | Continuous (always sampling)                       |

*ADC Event Multiplexer Trigger Sources*

**Timer-triggered sampling (EM = 0x5):**
this hooks a GPTM directly to the ADC without any CPU involvement at sample time. Configure the timer (as in Experiment 7) in periodic mode, then set its `TnOTE` bit (bit 5 of `TIMERx->CTL` for Timer A, bit 13 for Timer B — the same `CTL` register from Experiment 7’s GPTM section) so that each timeout pulses the ADC trigger instead of (or in addition to) generating a CPU interrupt. Set the corresponding sequencer’s `ADCEMUX` field to `0x5`, and the ADC samples automatically every time the timer expires — this is what Task 1 asks you to wire up.

#### ADCPSSI — ADC Processor Sample Sequence Initiate

Initiates sampling for the specified sample sequencer when triggered by the processor.

**ADCPSSI Register — ADC Processor Sample Sequence Initiate**

| Bits | Field |
| --- | --- |
| 31 | GSYNC |
| 30–28 | reserved |
| 27 | SYNCWAIT |
| 26–16 | reserved |
| 15–4 | reserved |
| 3 | SS3 |
| 2 | SS2 |
| 1 | SS1 |
| 0 | SS0 |

*Where:*

- **SSx**: Sample Sequencer x Initiate (write 1 to trigger conversion, self-clearing)
- **SYNCWAIT**: Synchronize Wait (1 = wait for synchronization signal before initiating)
- **GSYNC**: Global Synchronize (1 = generate synchronization signal for multiple ADC modules)

**Note**: The SSx bits are write-only and automatically clear after initiating the conversion. For synchronized sampling across modules, set GSYNC in one module to trigger all modules with SYNCWAIT enabled.

#### ADCSSFIFOn — ADC Sample Sequence FIFO

Holds the conversion results for the specified sample sequencer. Each read retrieves the next sample from the FIFO.

**ADCSSFIFOn Register — ADC Sample Sequence FIFO**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–12 | reserved |
| 11–0 | DATA |

*Where:*

- DATA: 12-bit conversion result (0 to 4095)
- reserved: Unused bits

You can read from the ADCSSFIFOn register to retrieve conversion results. Each read operation removes the oldest sample from the FIFO. Ensure that the FIFO is not empty before reading to avoid invalid data. You can check the FIFO status using the ADCSSSTAT register (refer to the datasheet for details).

#### ADCRIS — ADC Raw Interrupt Status

Holds the raw interrupt status for each sample sequencer.

**ADCRIS Register — ADC Raw Interrupt Status**

| Bits | Field |
| --- | --- |
| 31–17 | reserved |
| 16 | INRDC |
| 15–4 | reserved |
| 3 | SS3 |
| 2 | SS2 |
| 1 | SS1 |
| 0 | SS0 |

*Where:*

- SSx: Sample Sequencer x Interrupt Status (1 = interrupt pending, 0 = no interrupt)
- INRDC: Digital Comparator Raw Interrupt Status
- reserved: Unused bits

#### ADCIM — ADC Interrupt Mask

Masks/unmasks interrupts for each sample sequencer.

**ADCIM Register — ADC Interrupt Mask**

| Bits | Field |
| --- | --- |
| 31–20 | reserved |
| 19 | DCONSS3 |
| 18 | DCONSS2 |
| 17 | DCONSS1 |
| 16 | DCONSS0 |
| 15–4 | reserved |
| 3 | MASK3 |
| 2 | MASK2 |
| 1 | MASK1 |
| 0 | MASK0 |

*Where:*

- MASKx: Sample Sequencer x Interrupt Mask (1 = unmask interrupt, 0 = mask interrupt)
- DCONSSx: Digital Comparator Interrupt Mask for Sample Sequencer x (1 = unmask comparator interrupt, 0 = mask comparator interrupt)
- x: Sample sequencer index (0-3)
- reserved: Unused bits

#### ADCISC — ADC Interrupt Status and Clear

Holds and clears the interrupt status for each sample sequencer.

**ADCISC Register — ADC Interrupt Status and Clear**

| Bits | Field |
| --- | --- |
| 31–20 | reserved |
| 19 | DCINSS3 |
| 18 | DCINSS2 |
| 17 | DCINSS1 |
| 16 | DCINSS0 |
| 15–4 | reserved |
| 3 | IN3 |
| 2 | IN2 |
| 1 | IN1 |
| 0 | IN0 |

*Where:*

- SSx: Sample Sequencer x Interrupt Clear (write 1 to clear interrupt)
- DCINSSx: Digital Comparator Interrupt Clear for Sample Sequencer x (write 1 to clear comparator interrupt)
- reserved: Unused bits

### GPIO Registers

To configure GPIO pins for ADC input, you will need to set the appropriate GPIO registers. Below are the key registers involved in configuring GPIO pins for analog input:

#### GPIODEN — GPIO Digital Enable

Enables/disables digital functionality for GPIO pins. To use a pin as an analog input, its corresponding bit in this register must be cleared (0 = disable digital function).

**GPIODEN Register (Port x) — Digital Enable**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | Px7 |
| 6 | Px6 |
| 5 | Px5 |
| 4 | Px4 |
| 3 | Px3 |
| 2 | Px2 |
| 1 | Px1 |
| 0 | Px0 |

*Where:*

- Pxn: Digital Enable for Pin n of Port x (1 = enable digital function, 0 = disable digital function)
- reserved: Unused bits
- x: Port letter (A-F)
- n: Pin number (0-7)

Note: To configure a pin for analog input, clear its corresponding bit in the GPIODEN register.

#### GPIOAFSEL — GPIO Alternate Function Select

Selects alternate functions for GPIO pins. To use a pin for ADC input, its corresponding bit in this register must be set (1 = alternate function enabled).

**GPIOAFSEL Register (Port x) — Alternate Function Select**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | Px7 |
| 6 | Px6 |
| 5 | Px5 |
| 4 | Px4 |
| 3 | Px3 |
| 2 | Px2 |
| 1 | Px1 |
| 0 | Px0 |

*Where:*

- Pxn: Alternate Function Select for Pin n of Port x (1 = enable alternate function, 0 = GPIO function)
- reserved: Unused bits
- x: Port letter (A-F)
- n: Pin number (0-7)

Note: To configure a pin for ADC input, set its corresponding bit in the GPIOAFSEL register.

#### GPIOAMSEL — GPIO Analog Mode Select

Enables/disables analog functionality for GPIO pins. To use a pin as an analog input, its corresponding bit in this register must be set (1 = enable analog function).

**GPIOAMSEL Register (Port x) — Analog Mode Select**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | Px7 |
| 6 | Px6 |
| 5 | Px5 |
| 4 | Px4 |
| 3 | Px3 |
| 2 | Px2 |
| 1 | Px1 |
| 0 | Px0 |

*Where:*

- Pxn: Analog Mode Select for Pin n of Port x (1 = enable analog function, 0 = disable analog function)
- reserved: Unused bits
- x: Port letter (A-F)
- n: Pin number (0-7)

Note: To configure a pin for ADC input, set its corresponding bit in the GPIOAMSEL register.

## Activity

Read a potentiometer with the ADC and show its voltage two ways: as **millivolts** in the Watch window, and as a **colour zone** on the RGB LED (red below 1.1 V, green up to 2.2 V, blue above). The ADC converts one sample each time the program asks, and the program asks as fast as it can.

::: info Setting up the project
This experiment runs on the TM4C123 LaunchPad. If you haven't made a project yet, follow the [Tiva LaunchPad Project](/setup/tiva-project) guide first, choosing **C**, and make sure its blinking-LED test works. All the code goes in that project's `main.c`.
:::

### Step 1: Wire the potentiometer

| Potentiometer pin | LaunchPad |
| --- | --- |
| One outer pin | **3.3V** |
| Middle pin (the wiper) | **PE3** (AIN0) |
| Other outer pin | **GND** |

Use **3.3V**, not VBUS: the ADC measures 0 to 3.3 V, and PE3 must never see more.

![Potentiometer wiring: outer pins to 3.3 V and GND, wiper to PE3](/images/adc-connection.jpg)

### Step 2: Add the definitions

Replace the contents of `main.c` with the include, the definitions, and the variables:

<<< @/snippets/preview/exp9/main.c#defines{c}

- **`SS3`** is bit 3, sequencer 3's bit in four registers at once: `ACTSS` (enable), `PSSI` (start), `RIS` (done), and `ISC` (clear). Sequencer 3 takes one sample per start, the simplest of the [four](#sample-sequencers).
- **`EMUX_SS3`** is sequencer 3's 4-bit field in [ADCEMUX](#adcemux-—-adc-event-multiplexer-select). 0 there means "started by software".
- **`SSCTL_END0`** and **`SSCTL_IE0`** describe the one-sample sequence in [ADCSSCTL3](#adcssctln-—-adc-sample-sequence-control): sample 0 is the last, and its completion sets the flag in `RIS`.

### Step 3: Add `adc_read()`

Add the read function after the definitions:

<<< @/snippets/preview/exp9/main.c#read{c}

One conversion is four register accesses: start it (`PSSI`), wait for it (`RIS`), read it (`SSFIFO3`), clear the flag (`ISC`). The wait is short, about 1 µs at the ADC's 1 Msps, so polling is fine here.

### Step 4: Write `main`

Add `main` at the end of the file:

<<< @/snippets/preview/exp9/main.c#main{c}

1. **PE3 becomes analog.** An analog pin needs its digital circuit disconnected (`DEN` off) and the analog one connected (`AMSEL` on). `AFSEL` hands the pin to a peripheral, as for any alternate function.
2. **Sequencer 3** is configured while it's off: software trigger, input AIN0, one sample. Then it's enabled.
3. **The loop** reads, converts, and picks the zone. $\text{mV} = \text{raw} \times 3300 / 4095$, so 0 is 0 mV and 4095 is 3300 mV. The multiplication comes first, so integer division doesn't lose the result.

**Check:** build, flash, and run, and add `raw` and `millivolts` to the Watch window. Turn the knob from one end to the other: `raw` goes from about 0 to about 4095, `millivolts` from 0 to about 3300, and the LED goes red, green, blue.

**Then try:**

- Turn the knob slowly until the LED sits right at the red-green border, at 1.1 V. The LED flickers between the two colours, and `raw` jumps by a few counts even with your hand off the knob. That's **noise**: every reading is a little different. The [Task](#task) fixes both.
- Read the chip's **internal temperature sensor** instead: add `(1U << 3)` (the `TS0` bit) to the `SSCTL3` write, and compute the temperature in tenths of a degree from the [formula above](#internal-temperature-sensor): `temp_x10 = 1475 - (2475 * raw) / 4096`. Put a finger on the chip and watch it rise slowly.

::: details The complete program
<<< @/snippets/preview/exp9/main.c#full{c}
:::

## Common problems

| Symptom | Cause and fix |
| --- | --- |
| `raw` is stuck at 0 or 4095 | Wiring: the wiper isn't on PE3, or an outer pin is unconnected. |
| `raw` changes, but in big random jumps | `DEN` is still set on PE3, or `AMSEL` isn't: the pin is still partly digital. |
| Program hangs in `adc_read()` | The sequencer isn't enabled (`ACTSS`), or `SSCTL3` lacks `IE0`, so `RIS` never sets. |
| Program stops in `HardFault_Handler` | An ADC0 register was written before its clock was ready: keep the `PRADC` wait. |
| `millivolts` wraps to small values near the top | The formula overflowed: keep `raw * VREF_MV` in a `uint32_t` (4095 × 3300 fits easily). |

## Task

Turn the reading into a steady **level meter**. The ADC samples **1000 times a second by itself**, started by a timer, with no code running at sample time. The program **averages** the samples to cut the noise, and uses **hysteresis** so the LED never flickers at a zone border.

**Requirements:**
- Trigger sequencer 3 from **Timer0A**, periodic at **1 kHz**, as described under [ADCEMUX](#adcemux-—-adc-event-multiplexer-select): `EMUX` field 0x5, and the timer's `TAOTE` bit set. Don't enable Timer0A's own interrupt: the timer only starts conversions.
- Enable sequencer 3's interrupt (`ADC0->IM`, and `ADC0SS3_IRQn` in the NVIC). Its handler, `ADC0SS3_Handler`, reads the result and adds it to a **running average of the last 64 samples**.
- In the main loop, convert the average to millivolts and show the zone on the LED, as in the Activity, but with **100 mV of hysteresis**: once in a zone, the voltage must go 100 mV past a border before the colour changes.
- Add `raw`, the average, and `millivolts` to the Watch window. With your hand off the knob, `raw` should still jump by a few counts while the average barely moves.
- At a border, the LED must not flicker, however slowly you turn the knob.

::: details Hint: a running average without a big sum
Keep the last 64 samples in an array used as a ring, and a running total. Each new sample replaces the oldest: subtract the old one from the total, add the new one, and move the ring index on with `(i + 1) % 64`. The average is `total / 64`. 64 samples of up to 4095 sum to at most 262,080, so the total fits in a `uint32_t` with room to spare.
:::

::: details Hint: hysteresis
Remember the current zone. To leave zone 0 (red), the voltage must rise above 1100 + 100 mV; to come back from zone 1 (green), it must fall below 1100 − 100 mV. Between those, nothing changes. Write it as a `switch` on the current zone: each zone only checks its own two exits.
:::
