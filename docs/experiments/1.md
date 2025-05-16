---
outline: deep
lastUpdated: true
prev: false
---

# Introduction to ARM Assembly Language and Keil uVision5 <Badge type="tip" text="Experiment 1" />

In this experiment, we will learn how to write ARM assembly programs using Keil uVision5 and simulate them using the built-in simulator.


## Installation

We will be using Keil uVision5 for this course. You can download it from the following link:
[Keil uVision5](https://www.keil.com/demo/eval/arm.htm)

After downloading and installing the software, you can start using it by creating a new project using the following steps:

1. Open Keil uVision5
2. Click on Project -> New uVision Project
3. Select the folder where you want to save the project **(Save the project in a new empty folder)**
4. Enter the project name and click on Save
5. Select the device you are using in our case it is **ARM Cortex-M4 (ARMCM4)** and click on OK
6. Right click on the Source Group 1 and click on Add New Item to Group 'Source Group 1'
7. Select the file type as "Asm File" and enter the file name and click on Add
8. Right click on Target 1 and click on Options for Target 'Target 1'
9. Click on the Debug tab and select the "Use Simulator" option

::: tip
In order to debug the program, you should build the project first by clicking on the "Build" button or pressing F7. After building the project, you can start debugging by clicking on the "Start/Stop Debug Session" button or pressing F5.
:::

## ARM Assembly Syntax

The ARM assembly language is a low-level programming language used for programming ARM processors. The syntax of ARM assembly language is similar to other assembly languages, with instructions and directives that tell the processor what to do.
:::tip
Directives are assembler instructions the help with defining sections, allocating memory, and managing data.
:::

### Directives

The following are some of the common directives used in ARM assembly programming:

| **Directive** | **Description**                                                                                                |
| ------------- | -------------------------------------------------------------------------------------------------------------- |
| `PRESERVE8`   | Ensures that the stack remains 8-byte aligned, required for ARM EABI compliance.                               |
| `THUMB`       | Specifies that the assembler should generate Thumb (16-bit) instructions instead of ARM (32-bit) instructions. |
| `AREA`        | Defines a section in memory. Example: `AREA MYCODE, CODE, READONLY` defines a read-only code section.          |
| `EXPORT`      | Makes a symbol (label or function) available to other files/modules.                                           |
| `DCD`         | Defines a word (32-bit) constant in memory. Used to store addresses or data.                                   |
| `DCW`         | Defines a half-word (16-bit) constant in memory.                                                               |
| `DCB`         | Defines a byte (8-bit) constant in memory.                                                                     |
| `ALIGN`       | Aligns the next data or instruction to the appropriate memory boundary.                                        |
| `ENTRY`       | Marks the entry point of the program.                                                                          |
| `END`         | Marks the end of the program.                                                                                  |

### Instructions

In this section, we will discuss some of the common ARM assembly instructions used in programming ARM processors. we will dive deeper into the instructions in the upcoming experiments.

| **Instruction**         | **Description**                                                                        |
| ----------------------- | -------------------------------------------------------------------------------------- |
| `MOV R0, #12`           | Moves the immediate value `12` into register `R0`.                                     |
| `ADD R0, R0, #4`        | Adds `4` to `R0` and stores the result in `R0`.                                        |
| `B STOP`                | Branches (jumps) to the label `STOP`, creating an infinite loop.                       |
| `MOV R2, #0x76`         | Moves the 8-bit hex value `0x76` into the lower portion of `R2`.                       |
| `MOV R2, #0x7654`       | Moves the 16-bit hex value `0x7654` into the lower portion of `R2`.                    |
| `MOVT R2, #0x7654`      | Moves the 16-bit hex value `0x7654` into the **upper** portion of `R2`.                |
| `MOV32 R2, #0x76543210` | Moves the full 32-bit hex value `0x76543210` into `R2`.                                |
| `LDR R2, =0x76543210`   | Loads `R2` with the 32-bit hex value `0x76543210` (alternative method to `MOV32`).     |
| `ADD R1, R2, R3`        | Adds the values in `R2` and `R3`, storing the result in `R1`.                          |
| `ADDS R1, R2, R3`       | Adds `R2` and `R3`, storing the result in `R1` and updating the condition flags.       |
| `SUB R1, R2, R3`        | Subtracts `R3` from `R2`, storing the result in `R1`.                                  |
| `SUBS R1, R2, R3`       | Subtracts `R3` from `R2`, storing the result in `R1` and updating the condition flags. |
| `B LABEL`               | Branches (jumps) to the label `LABEL`.                                                 |
| `B STOP`                | Branches to `STOP`, creating an infinite loop.                                         |

## Examples

### Example 1

```asm
    PRESERVE8
    THUMB
    AREA  RESET, DATA, READONLY
    EXPORT __Vectors
__Vectors
    DCD 0x20001000
    DCD Reset_Handler
    ALIGN
    AREA MYCODE, CODE, READONLY
    ENTRY
    EXPORT Reset_Handler
Reset_Handler
    MOV R0, #12

STOP
    ADD R0, R0, #4
    B STOP
    END

```

### Example 2

```asm
    PRESERVE8
    THUMB
    AREA  RESET, DATA, READONLY
    EXPORT __Vectors
__Vectors
    DCD 0x20001000
    DCD Reset_Handler
    ALIGN
    AREA MYCODE, CODE, READONLY
    ENTRY
    EXPORT Reset_Handler
Reset_Handler
    MOV R2, #0x76; Move the 8-bit Hex number 76 to the low portion of R2
    MOV R2, #0x7654; Move the 16-bit Hex number 7654 to the low portion of R2
    MOVT R2, #0x7654; Move the 16-bit Hex number 7654 to the high portion of R2
    MOV32 R2, #0x76543210 ; Move the 32-bit Hex number 76543210 to the R2
    LDR R2, = 0x76543210 ; Load R2 with the 32-bit Hex number 76543210
    ADD R1, R2, R3 ; R1 = R2 + R3
    ADDS R1, R2, R3 ; R1 = R2 + R3, and FLAGs are updated
    SUB R1, R2, R3 ; R1 = R2 - R3
    SUBS R1, R2, R3 ; R1 = R2 - R3, and FLAGs are updated
    B LABEL
LABEL
STOP
    B STOP
    END
```

## Lab Work

Write an ARM assembly program that performs the following instructions, and observe the results in the simulator:

```asm
    MOV R2, #0x01 ; R2 = ?
    MOV R3, #0x02 ; R3 = ?
    ;Other examples to move immediate values
    MOV R5, #0x3210 ; R5 = ?
    MOVT R5, #0x7654 ; R5 = ?
    MOV32 R6, #0x87654321 ; R6 = ?
    LDR R7, = 0x87654321 ; R7 = ?
    ADD R1,R2,R3 ; R1 = ?
    MOV32 R3, #0xFFFFFFFF ; R3 = ?
    ADDS R1,R2,R3 ; R1 = ?
    ; specify Condition Code updates
    SUBS R1,R2,R3 ; R1 = ?
    ; specify Condition Code updates
    MOV R4, #0xFFFFFFFF ; R4 = ?
    ADD R1,R2,R4 ; R1 = ?
    ; How did that operation affect the flags in CPSR?
    ADDS R1,R2,R4 ; R1 = ?
    ; Please specify Condition Code updates
    ; and now what happened to the flags in the CPSR?
    MOV R2, #0x00000002 ; R2 = ?
    ADDS R1,R2,R4 ; R1 = ?
    ; again, what happened to the flags?
    MOV R2, #0x00000001 ; R2 = ?
    MOV R3, #0x00000002 ; R3 = ?
    ADDS R1,R2,R3 ; R1 = ?
    ; Add some small numbers again
    ; and check the flags again......
    ; Add numbers that will create an overflow
    MOV R2, #0x7FFFFFFF ; R2 = ?
    MOV R3, #0x7FFFFFFF ; R3 = ?
    ADDS R1,R2,R3 ; R1 = ?
    ; Check the flags in the CPSR?
```
