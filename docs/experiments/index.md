---
outline: false
---
# Experiment Navigator

Welcome to the Experiment Navigator! This document serves as a guide to the various experiments and resources available for ARM assembly language programming and embedded systems development using the TM4C123G LaunchPad.
### ARM Cortex-M4 Assembly Language Basics
| Experiment                                  | Description                                                                                  |
|---------------------------------------------|----------------------------------------------------------------------------------------------|
|  [**Experiment 1: Assembly Basics and Program Structure**](/experiments/1)           | Learn ARM Cortex-M4 architecture, registers, memory mapping, instruction formats, and assembly programming fundamentals. |
|  [**Experiment 2: Data Processing and Memory Operations**](/experiments/2) | Master data-processing instructions (arithmetic, logical, barrel shifter), load/store operations, memory addressing modes, and assembler directives for data declaration. |
|  [**Experiment 3: Control Flow and Subroutines**](/experiments/3)     | Implement branching, conditional execution, loop structures (for/while), procedure calls with AAPCS, stack operations (PUSH/POP), and nested procedures.      |
#### EK-TM4C123GXL Microcontroller
Make sure to have the TM4C123G LaunchPad connected to your computer and the Keil uVision5 IDE is configured and set up for development. To set up the IDE, follow the instructions in the [Setup Guide](/setup.md).
| Experiment                                  | Description                                                                                  |
|---------------------------------------------|----------------------------------------------------------------------------------------------|
| [**Experiment 4: Microcontroller Architecture and GPIO Output**](/experiments/4) | Explore TM4C123 architecture, core peripherals (NVIC, SysTick), memory map, and GPIO configuration for controlling on-board LEDs. | 
| [**Experiment 5: GPIO Inputs and Interrupt Handling**](/experiments/5) | Configure GPIO pins as inputs, implement edge-triggered interrupts, handle button debouncing, and manage interrupt priorities using NVIC. | 
| [**Experiment 6: Hardware Timers and Timing Control**](/experiments/6) | Configure SysTick for periodic interrupts and delays, program GPTM modules in one-shot and periodic modes, and implement accurate timing for embedded applications. |
| [**Experiment 7: Character LCD Display Interface**](/experiments/7) | Interface with character LCD modules using GPIO, implement initialization sequences, and display text and custom characters. | 
| [**Experiment 8: Analog-to-Digital Converter (ADC)**](/experiments/8) | Configure 12-bit ADC channels, perform single and continuous sampling, read analog sensors, and implement software averaging. |
| [**Experiment 9: Universal Asynchronous Receiver-Transmitter (UART)**](/experiments/9) | Configure UART for serial communication, implement transmit and receive functions, handle interrupts, and interface with PC terminal applications. |

###  Additional Resources
-  [Keil uVision5 IDE](https://www.keil.com/demo/eval/arm.htm) - Setup and development environment for ARM programming.
-  [ARM Documentation](https://developer.arm.com/documentation/dui0041/latest/) - Comprehensive documentation for ARM architecture.
-  [ARM Instruction Set Summary](https://developer.arm.com/documentation/100165/0201/Programmers-Model/Instruction-set-summary/Processor-instructions) - Detailed instruction set reference.
-  [TM4C123GH6PM Microcontroller Datasheet](https://www.ti.com/lit/ds/symlink/tm4c123gh6pm.pdf) - Technical specifications and features of the TM4C123G microcontroller.