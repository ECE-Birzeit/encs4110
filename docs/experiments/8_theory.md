---
outline: deep
lastUpdated: true
---

# Experiment 8: Analog-to-Digital Converter (ADC)
::: info Complete Lab Manual
For the complete experiment including learning objectives, theoretical background, and detailed explanations, download the PDF manual:
 [**Download Experiment 8 PDF**](https://raw.githubusercontent.com/mabushelbaia/encs4110/main/manual/encs4110_08.pdf)
:::

## Learning Objectives

After completing this experiment, you will be able to:

- Understand analog-to-digital conversion principles including sampling, quantization, and resolution.
- Recognize the architecture of the TM4C123 ADC modules including dual 12-bit converters and shared input channels.
- Configure ADC sample sequencers (SS0-SS3) with different depths and trigger sources.
- Configure GPIO pins for analog input using AFSEL, AMSEL, and DEN registers.
- Implement software-triggered and hardware-triggered ADC conversions.
- Read and interpret ADC conversion results from FIFO registers.
- Convert digital ADC values to physical quantities such as voltage and temperature.
- Implement both polling and interrupt-driven ADC operation modes.
- Utilize hardware averaging and multi-channel sampling for improved measurement accuracy.

## Experiment Overview

This experiment introduces analog-to-digital conversion using the TM4C123's dual 12-bit ADC modules. You will configure sample sequencers for automatic multi-channel data acquisition, implement both polling and interrupt-driven sampling modes, and interface with analog sensors such as potentiometers and the internal temperature sensor. By the end of this lab, you will understand how to convert continuous analog signals to discrete digital values, configure trigger sources for periodic sampling, and implement sensor interfacing applications that bridge the analog and digital domains.



## Theoretical Background

### Introduction to Analog-to-Digital Conversion

Analog-to-Digital Converters (ADCs) are essential components in embedded systems, enabling microcontrollers to interface with the analog world by converting continuous analog signals (such as voltage from sensors) into discrete digital values for processing. They are widely used in applications like temperature sensing, light intensity measurement, and audio signal processing.

#### ADC Resolution and Sampling

ADCs are characterized by their **resolution**, which indicates the number of discrete levels they can represent, typically expressed in bits. For instance, a 12-bit ADC can represent 2^12 = 4096 discrete levels. The resolution determines the smallest detectable change in the analog input, known as the **least significant bit (LSB)**. The voltage corresponding to one LSB is calculated as:

```
LSB Voltage = (V_REFP - V_REFN) / 2^Resolution
```

where V_REFP is the positive reference voltage, V_REFN is the negative reference voltage, and Resolution is the number of bits.

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
|---------|------|------|------|------|------|------|------|------|------|------|-------|-------|
| Pin | PE3 | PE2 | PE1 | PE0 | PD3 | PD2 | PD1 | PD0 | PE5 | PE4 | PB4 | PB5 |

#### Sample Sequencers

Each ADC module has **four independent sample sequencers** that manage sampling control and data capture autonomously. They differ only in the number of samples and FIFO depth.

| Sequencer | Number of Samples | FIFO Depth | Priority |
|-----------|-------------------|------------|----------|
| SS0 | 8 | 8 entries | Highest |
| SS1 | 4 | 4 entries | High |
| SS2 | 4 | 4 entries | Low |
| SS3 | 1 | 1 entry | Lowest |

#### Trigger Sources

ADC conversions can be initiated by various trigger sources for flexibility:

- **Processor (Software)**: Manual trigger via ADCPSSI register
- **Analog Comparators**: Trigger on comparator output events
- **GPIO**: External signal on GPIO pins
- **General-Purpose Timers**: Periodic sampling synchronized with timer events
- **PWM Generators**: Synchronize sampling with PWM signals for motor control
- **Continuous**: Free-running continuous sampling mode

Both ADC modules can use independent or shared triggers. When shared, a programmable phase shifter allows delaying sampling by a phase angle for time-interleaved sampling to achieve higher effective rates.

### Internal Temperature Sensor

The internal temperature sensor can be sampled by setting the TSn bit in the ADCSSCTLn register. Temperature (TEMP in °C) is calculated from the ADC reading (ADCCODE, 0 to 4095) as:

```
TEMP = 147.5 - (75 × (V_REFP - V_REFN) × ADCCODE / 4096)
```

**Note**: This measures the microcontroller's die temperature, not ambient, and is useful for thermal management but may require calibration.

### ADC Registers

The TM4C123 ADC modules are configured via registers. Key registers include:

#### RCGCADC — ADC Run Mode Clock Gating Control

Enables/disables clock for ADC modules. Bits correspond to modules (1 = enable, 0 = disable).

#### ADCACTSS — ADC Active Sample Sequencer

Enables/disables sample sequencers. Bits correspond to sequencers (1 = enable, 0 = disable). BUSY indicates ADC status (1 = busy, 0 = idle).

#### ADCSSMUXn — ADC Sample Sequence Input Multiplexer Select

Selects input channels for each sample in a sequencer. Each 4-bit field (MUX0-MUX7) specifies the channel (0-11 for AIN0-AIN11).

#### ADCSSCTLn — ADC Sample Sequence Control

Configures each sample in a sequence. The END bit must be set for the final sample. This 32-bit register handles up to eight samples.

Where:

- TSx: Temperature sensor select (1 = sample temperature sensor)
- IEx: Interrupt enable (1 = enable interrupt on sample completion)
- ENDx: End of sequence (1 = last sample in sequence)
- Dx: Differential select (1 = differential input, 0 = single-ended)
- x: Sample index (0-7)

#### ADCEMUX — ADC Event Multiplexer Select

Selects trigger source for each sample sequencer. Each 4-bit field (EM0-EM3) specifies the trigger source.

The EMx field specifies the trigger source for sample sequencer x:

| Value | Trigger Source |
|-------|----------------|
| 0x0 | Processor (software trigger via ADCPSSI register) |
| 0x1 | Analog Comparator 0 |
| 0x2 | Analog Comparator 1 |
| 0x3 | Reserved |
| 0x4 | External GPIO (connected to GPIO interrupt) |
| 0x5 | Timer (requires TnOTE bit set in GPTMCTL register) |
| 0x6 | PWM Generator 0 |
| 0x7 | PWM Generator 1 |
| 0x8 | PWM Generator 2 |
| 0x9 | PWM Generator 3 |
| 0xA-0xE | Reserved |
| 0xF | Continuous (always sampling) |

#### ADCPSSI — ADC Processor Sample Sequence Initiate

Initiates sampling for the specified sample sequencer when triggered by the processor.

Where:

- **SSx**: Sample Sequencer x Initiate (write 1 to trigger conversion, self-clearing)
- **SYNCWAIT**: Synchronize Wait (1 = wait for synchronization signal before initiating)
- **GSYNC**: Global Synchronize (1 = generate synchronization signal for multiple ADC modules)

**Note**: The SSx bits are write-only and automatically clear after initiating the conversion. For synchronized sampling across modules, set GSYNC in one module to trigger all modules with SYNCWAIT enabled.

#### ADCSSFIFOn — ADC Sample Sequence FIFO

Holds the conversion results for the specified sample sequencer. Each read retrieves the next sample from the FIFO.

Where:

- DATA: 12-bit conversion result (0 to 4095)
- reserved: Unused bits

You can read from the ADCSSFIFOn register to retrieve conversion results. Each read operation removes the oldest sample from the FIFO. Ensure that the FIFO is not empty before reading to avoid invalid data.

#### ADCRIS — ADC Raw Interrupt Status

Holds the raw interrupt status for each sample sequencer.

Where:

- SSx: Sample Sequencer x Interrupt Status (1 = interrupt pending, 0 = no interrupt)
- INRDC: Digital Comparator Raw Interrupt Status

#### ADCIM — ADC Interrupt Mask

Masks/unmasks interrupts for each sample sequencer.

Where:

- MASKx: Sample Sequencer x Interrupt Mask (1 = unmask interrupt, 0 = mask interrupt)
- DCONSSx: Digital Comparator Interrupt Mask for Sample Sequencer x

#### ADCISC — ADC Interrupt Status and Clear

Holds and clears the interrupt status for each sample sequencer.

Where:

- SSx: Sample Sequencer x Interrupt Clear (write 1 to clear interrupt)
- DCINSSx: Digital Comparator Interrupt Clear for Sample Sequencer x

### GPIO Registers

To configure GPIO pins for analog input, you will need to set the appropriate GPIO registers.

#### GPIODEN — GPIO Digital Enable

Enables/disables digital functionality for GPIO pins. To use a pin as an analog input, its corresponding bit in this register must be cleared (0 = disable digital function).

Note: To configure a pin for analog input, clear its corresponding bit in the GPIODEN register.

#### GPIOAFSEL — GPIO Alternate Function Select

Selects alternate functions for GPIO pins. To use a pin for ADC input, its corresponding bit in this register must be set (1 = alternate function enabled).

Note: To configure a pin for ADC input, set its corresponding bit in the GPIOAFSEL register.

#### GPIOAMSEL — GPIO Analog Mode Select

Enables/disables analog functionality for GPIO pins. To use a pin as an analog input, its corresponding bit in this register must be set (1 = enable analog function).

Note: To configure a pin for ADC input, set its corresponding bit in the GPIOAMSEL register.

### Configuration Steps

Configuring a GPIO pin for analog input on the TM4C123 microcontroller involves three stages:

#### GPIO Pin Configuration

To prepare a GPIO pin for analog input:

1. Enable the clock for the corresponding GPIO port by setting the appropriate bit in the `RCGCGPIO` register.
2. Enable the pin's alternate function using the `GPIOAFSEL` register.
3. Disable its digital function by clearing the pin's bit in the `GPIODEN` register.
4. Enable the analog function by setting the pin's bit in the `GPIOAMSEL` register.
5. Set the pin direction as input by clearing its bit in the `GPIODIR` register.

#### ADC Sampling Sequencer Setup

After the pin is configured, the ADC module and sampling sequencer must be set up:

1. Enable the ADC clock by setting the correct bit in the `RCGCADC` register.
2. Disable the chosen sample sequencer by clearing its bit in the `ADCACTSS` register.
3. Select the input channel(s) by configuring the `ADCSSMUXn` register.
4. Set the control options for each sample (e.g., END, IE) in the `ADCSSCTLn` register, ensuring the END bit is set for the final sample.
5. Choose the trigger source by configuring the `ADCEMUX` register.
6. Re-enable the sequencer by setting its bit in `ADCACTSS`.
7. If processor-triggered sampling is used, start a conversion using the `ADCPSSI` register.

#### Interrupt Configuration

If interrupts are required:

1. Enable interrupts for the selected sequencer by setting its bit in the `ADCIM` register.
2. Enable the corresponding interrupt in the NVIC by setting the appropriate bit in the `NVIC_ISER` register.
3. In the ISR, read the conversion result from the `ADCSSFIFO` register.
4. Clear the interrupt flag by writing to the appropriate bit in the `ADCISC` register.