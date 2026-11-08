---
title: "Assembly Basics and Program Structure"
description: "This experiment introduces ARM Cortex-M4 assembly programming fundamentals, including register architecture, status flags, and memory organization."
outline: deep
lastUpdated: true
prev: false
manual: "/manual/encs4110_manual.pdf#nameddest=chapter.1"
part: "Part 1: ARM Assembly"
---

# Assembly Basics and Program Structure

This experiment introduces ARM Cortex-M4 assembly programming fundamentals, including register architecture, status flags, and memory organization. You will build minimal startup code with a vector table and Reset_Handler, practice data-movement and arithmetic instructions while tracking their effect on the condition flags, and use the Keil µVision5 debugger to single-step through code while inspecting registers and memory. By the end of this lab, you will understand the core building blocks of ARM assembly programming and be able to write, assemble, and debug low-level embedded code.

## Objectives

- Identify the main components of the ARM Cortex-M4 architecture, including general-purpose registers, stack pointer, link register, program counter, and program status register (xPSR).
- Write, assemble, and debug a minimal ARM assembly program containing a vector table and `Reset_Handler`.
- Use core data-movement (`MOV`, `MOVW`, `MOVT`) and arithmetic (`ADD`, `SUB`) instructions to manipulate register data.
- Predict how the condition flags (N, Z, C, V) change after arithmetic instructions with the `S` suffix (e.g., `ADDS`, `SUBS`).
- Debug assembly programs in **Keil uVision5** using breakpoints, single-stepping, and register/memory inspection to analyze instruction effects.

## Background

### Cortex-M4 Architecture

#### Registers Overview

The Cortex-M4 architecture includes a set of general-purpose registers (R0-R12), a stack pointer (SP), a link register (LR), a program counter (PC), and a program status register (xPSR), all of which are 32-bit registers. The general-purpose registers are used for data manipulation and temporary storage during program execution. The SP is used to manage the call stack, while the LR holds the return address for function calls. The PC points to the next instruction to be executed, and the xPSR contains flags and status information about the processor state.

##### Program Status Register (xPSR)

holds the current state of the processor, including condition flags (Negative, Zero, Carry, Overflow), interrupt status, and execution state. These flags are updated based on the results of arithmetic and logical operations, allowing for conditional branching and decision-making in programs.

##### How the flags are set

For an `S`-suffixed instruction such as `ADDS`/`SUBS`: **Z** is set when the result is exactly zero; **N** mirrors the sign (bit 31) of the result; for **addition**, **V** (signed overflow) is set when two operands of the *same* sign produce a result of the *opposite* sign (e.g., adding two large positive numbers wraps into a negative result), and **C** is set on an unsigned carry out of bit 31; for **subtraction**, `SUBS Rd, Rn, Op2` is computed as `Rn + (~Op2) + 1`, so **C** is set when *no borrow* was needed (i.e. `Rn` ≥ `Op2` unsigned) and cleared when a borrow occurred. Example 1 below asks you to predict exactly these outcomes.

#### Memory Mapping

The Cortex-M4 uses a flat memory model, where all memory locations are accessible through a single address space. This model simplifies programming and allows for efficient access to data and instructions. The memory is divided into several regions, including code memory (for storing instructions), data memory (for storing variables), and peripheral memory (for interfacing with hardware components). The architecture supports both little-endian and big-endian data formats, with little-endian being the default.

| **Region** | **Address Range** | **Description** |
|:---|:---|:---|
| Code | `0x00000000`–`0x1FFFFFFF` | Flash memory for program code |
| SRAM | `0x20000000`–`0x3FFFFFFF` | On-chip static RAM for data |
| Peripheral | `0x40000000`–`0x5FFFFFFF` | Memory-mapped peripheral registers |
| External RAM | `0x60000000`–`0x9FFFFFFF` | External RAM (if implemented) |
| External Device | `0xA0000000`–`0xDFFFFFFF` | External devices/memory (if implemented) |
| Private Peripheral Bus | `0xE0000000`–`0xE00FFFFF` | Cortex-M4 internal peripherals (NVIC, SysTick, MPU, SCB) |
| System | `0xE0100000`–`0xFFFFFFFF` | System region (reserved/system-level) |

*Cortex-M4 Memory Regions (ARMv7-M)*

### Assembly Language Basics

Assembly language is a low-level programming language that provides a direct correspondence between human-readable instructions and the machine code executed by the processor. Each instruction encodes a specific operation, such as moving data, performing arithmetic or logic, or altering control flow. Because it maps so closely to hardware, assembly allows precise control of system resources and is commonly used in performance-critical routines or when direct access to hardware is required.

Assembly programs are typically composed of three main elements: *instructions*, *directives*, and *labels*.

##### Instructions

Instructions are the executable commands that the CPU carries out; each one directly translates to one or more machine code opcodes and determines the actual behavior of the program. See *Instruction Set Overview* below for the main categories, with examples.

##### Directives

Directives are commands to the assembler that guide how source code is translated into machine code but do not generate instructions themselves. Common examples include `AREA` (define a code or data section), `ALIGN` (align data to memory boundaries), `DCD` (allocate and initialize a word of storage), and `EXPORT` (export a symbol for linking). Directives organize program layout, control memory allocation, and manage symbol visibility.

##### Labels

Labels are symbolic names that mark specific locations in code or data. They act as targets for jumps and branches or as references for data access. Labels improve program readability and maintainability by avoiding hard-coded addresses. For instance, a label like `loop_start` can be used as the destination of a branch instruction, and the assembler automatically computes the correct relative address.

#### Instruction Set Overview

The ARM Cortex-M4 instruction set is a subset of the ARMv7-M architecture, designed for efficient execution in embedded systems. It includes a variety of instructions for data processing, memory access, and control flow. Key categories of instructions include:

- **Data Processing Instructions**: These include arithmetic operations (e.g., `ADD`, `SUB`), logical operations (e.g., `AND`, `ORR`), and data movement instructions (e.g., `MOV`, `MVN`).
- **Load and Store Instructions**: Instructions like `LDR` (load register) and `STR` (store register) are used to transfer data between registers and memory.
- **Branch Instructions**: Control flow is managed using branch instructions such as `B` (branch), `BL` (branch with link), and conditional branches like `BEQ` (branch if equal).
- **Special Instructions**: These include instructions for system control, such as `NOP` (no operation), `WFI` (wait for interrupt), and instructions for manipulating the stack and handling exceptions.

#### General Instruction Format

Assembly source lines generally follow this shape:

$$
\texttt{[label]} \quad \texttt{OPCODE\{<cond>\}\{S\}} \quad \texttt{operands} \quad \texttt{; comment}
$$

where curly braces `{}` denote optional components, and:

- `label`: optional symbolic name marking the current address.
- `OPCODE`: instruction mnemonic (e.g., `ADD`, `MOV`, `B`).
- `<cond>`: optional condition code (e.g., `EQ`, `NE`, `LT`, `GE`) that predicates execution.
- `S`: optional suffix indicating whether to update the condition flags (e.g., `ADDS`).
- `operands`: registers, immediates, or memory operands (e.g., `R0, R1, #1` or `[R2]`).
- Anything after a semicolon (`;`) is a comment ignored by the assembler.

```asm
loop_start   ADDS   R0, R0, #1      ; R0 = R0 + 1, update flags (N,Z,C,V)
```

### Basic Program Template (Boilerplate)

The minimal skeleton below shows a valid vector table in a `READONLY` `RESET` area, a `READWRITE` data area for variables, and a code area with the `Reset_Handler` entry point.

```asm
AREA    RESET, CODE, READONLY     ; Vector table lives in read-only code
        EXPORT  __Vectors                 ; Make symbol visible to the linker
__Vectors
        DCD     0x20001000                ; Initial SP value (top of stack in SRAM)
        DCD     Reset_Handler             ; Reset vector: entry address
        ALIGN        
        ; -------------------- Read-Write Data --------------------
        AREA    M_DATA, DATA, READWRITE   ; Variables go here (RAM)
        EXPORT  COUNT                     ; Export if referenced by other modules
COUNT   SPACE   4                         ; RW variable (word), not initialized
BUF     SPACE   16                        ; Uninitialized RW buffer (16 bytes)
        ALIGN        
        ; -------------------- Application Code -------------------
        AREA    MYCODE, CODE, READONLY
        ENTRY
        EXPORT  Reset_Handler
Reset_Handler
        ; Example: COUNT++ and store to BUF[0]
        LDR     R0, =COUNT                ; R0 <- &COUNT
        MOV     R1, #0                    ; RAM starts with unknown contents,
        STR     R1, [R0]                  ; so set COUNT = 0 before using it
        LDR     R1, [R0]                  ; R1 <- COUNT
        ADDS    R1, R1, #1                ; R1 = R1 + 1 (update flags)
        STR     R1, [R0]                  ; COUNT <- R1        
        LDR     R2, =BUF                  ; R2 <- &BUF
        STRB    R1, [R2]                  ; BUF[0] <- (low byte of R1)
STOP    B       STOP                      ; Stay here forever
        END
```

##### What each directive does

- `AREA <name>, CODE|DATA, READONLY|READWRITE`: defines a section. Put the vector table and program text in `CODE, READONLY`; put variables in `DATA, READWRITE`.
- `EXPORT <symbol>`: makes a label visible to the linker/other modules.
- `DCD, DCW, DCB <values>`: allocate and initialize words, halfwords, or bytes.
- `SPACE <n>`: reserve *n* bytes of uninitialized storage in RAM.
- `ALIGN`: align to a suitable boundary (commonly 4 bytes for words).
- `ENTRY`: mark the entry point of the image for the toolchain.
- `END`: end of source file.

##### Notes

- The first two words in `__Vectors` must be the initial stack pointer value and the address of `Reset_Handler`.
- The assembler/linker places sections in appropriate memory regions based on the target device and linker script.
- Labels must start at the beginning of the line (no indentation), while instructions and directives should be indented for proper assembly.
- `READWRITE` data is **not initialized** in this lab’s projects. Initial values for RAM have to be copied from flash by startup code before `Reset_Handler` runs, and these projects have no such copy step. So a `DCD`/`DCB` value in a `READWRITE` area is *not* what RAM holds at run time; reserve RAM with `SPACE` and always write a variable before reading it (as `Reset_Handler` does with `COUNT` above). Constants, strings, and lookup tables with fixed values belong in a `READONLY` area, where their initial values are part of the program image.

## Example 1: Simple Arithmetic Operations and Flag Manipulation

::: info Setting up the project
This experiment is written in assembly and runs in Keil's simulator. If you haven't made a project yet, follow the [Assembly Project](/setup/assembly-project) guide first.
:::
The following example demonstrates basic arithmetic operations, flag manipulation.

<<< @/snippets/assembly/exp1/example1.asm
