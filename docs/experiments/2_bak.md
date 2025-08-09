---
outline: deep
lastUpdated: true
---

# ARM Data Processing Instructions  

## ARM Registers

ARM has 16 programmer-visible registers and a Current Program Status Register `0xCPSR`, `R0` to `R15`. These registers are used to store data and addresses. The first 13 registers are general-purpose registers, and the last three registers are used for special purposes.

### Special Purpose Registers

- **R13 (SP)**: Stack Pointer
- **R14 (LR)**: Link Register
- **R15 (PC)**: Program Counter

### Current Program Status Register (CPSR)

- **N**: Negative, set if the result is negative.
- **Z**: Zero, set if the result is zero.
- **C**: Carry, set if there is a carry out of the most significant bit.
- **V**: Overflow, set if the result causes an overflow.

## ARM Instructions

ARM Instructions is encoded in 32-bit words. The instructions are divided into three categories:

1. **Data Processing Instructions**: These instructions perform arithmetic operations on data.
2. **Load and Store Instructions**: These instructions are used to load and store data from memory.
3. **Control Flow Instructions**: These instructions are used to control the flow of the program.

And they have the general format as follows:

```asm
{LABEL}  OPCODE{CONDITION}  OPERANDS ; Comment
```

The `CONDITION` flag is optional and allows the instructions to be excuted only if a specified condition is met. Some of the common conditions are:

| Condition Code | Mnemonic                            | Explanation                                                                                                                    |
| -------------- | ----------------------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| EQ             | Equal                               | Executes if the Zero (Z) flag is set (result is zero).                                                                         |
| NE             | Not Equal                           | Executes if the Zero (Z) flag is clear (result is nonzero).                                                                    |
| CS / HS        | Carry Set / Unsigned Higher or Same | Executes if the Carry (C) flag is set (unsigned comparison: `>=`).                                                             |
| CC / LO        | Carry Clear / Unsigned Lower        | Executes if the Carry (C) flag is clear (unsigned comparison: `<`).                                                            |
| MI             | Negative                            | Executes if the Negative (N) flag is set (result is negative).                                                                 |
| PL             | Positive or Zero                    | Executes if the Negative (N) flag is clear (result is positive or zero).                                                       |
| VS             | Overflow Set                        | Executes if the Overflow (V) flag is set (arithmetic overflow occurred).                                                       |
| VC             | Overflow Clear                      | Executes if the Overflow (V) flag is clear (no overflow).                                                                      |
| HI             | Unsigned Higher                     | Executes if both the Carry (C) flag is set and the Zero (Z) flag is clear (unsigned comparison: `>`).                          |
| LS             | Unsigned Lower or Same              | Executes if the Carry (C) flag is clear or the Zero (Z) flag is set (unsigned comparison: `<=`).                               |
| GE             | Greater or Equal                    | Executes if the Negative (N) flag equals the Overflow (V) flag (signed comparison: `>=`).                                      |
| LT             | Less Than                           | Executes if the Negative (N) flag is not equal to the Overflow (V) flag (signed comparison: `<`).                              |
| GT             | Greater Than                        | Executes if the Zero (Z) flag is clear and the Negative (N) flag equals the Overflow (V) flag (signed comparison: `>`).        |
| LE             | Less Than or Equal                  | Executes if the Zero (Z) flag is set or the Negative (N) flag is not equal to the Overflow (V) flag (signed comparison: `<=`). |
| AL             | Always                              | Default condition; executes unconditionally.                                                                                   |

## Data Processing Instructions

Data processing instructions perform arithmetic, logical, and shift operations on registers.

### Arithmetic Instructions

These instructions perform basic mathematical operations like addition, subtraction, and multiplication.

| Instruction | Mnemonic                    | Assembler Syntax           | Description                                    |
| ----------- | --------------------------- | -------------------------- | ---------------------------------------------- |
| ADD         | Add                         | `ADD Rd, Rn, Rm`           | Adds two registers.                            |
| SUB         | Subtract                    | `SUB Rd, Rn, Rm`           | Subtracts one register from another.           |
| RSB         | Reverse Subtract            | `RSB Rd, Rn, Rm`           | Subtracts with reversed operands.              |
| ADC         | Add with Carry              | `ADC Rd, Rn, Rm`           | Adds two registers with carry.                 |
| SBC         | Subtract with Carry         | `SBC Rd, Rn, Rm`           | Subtracts with carry.                          |
| RSC         | Reverse Subtract with Carry | `RSC Rd, Rn, Rm`           | Reverse subtraction with carry.                |
| MUL         | Multiply                    | `MUL Rd, Rn, Rm`           | Multiplies two registers.                      |
| MLA         | Multiply Accumulate         | `MLA Rd, Rn, Rm, Ra`       | Multiplies and adds an accumulator.            |
| UMULL       | Unsigned Multiply Long      | `UMULL RdLo, RdHi, Rn, Rm` | Stores 64-bit result across `RdLo` and `RdHi`. |
| SMULL       | Signed Multiply Long        | `SMULL RdLo, RdHi, Rn, Rm` | Same as UMULL but for signed numbers.          |
| SDIV        | Signed Divide               | `SDIV Rd, Rn, Rm`          | Divides two signed numbers.                    |
| UDIV        | Unsigned Divide             | `UDIV Rd, Rn, Rm`          | Divides two unsigned numbers.                  |

### Logical Instructions

These instructions perform bitwise operations like AND, OR, and XOR.

| Instruction | Mnemonic      | Assembler Syntax | Description           |
| ----------- | ------------- | ---------------- | --------------------- |
| AND         | Bitwise AND   | `AND Rd, Rn, Rm` | Performs bitwise AND. |
| ORR         | Bitwise OR    | `ORR Rd, Rn, Rm` | Performs bitwise OR.  |
| EOR         | Bitwise XOR   | `EOR Rd, Rn, Rm` | Performs bitwise XOR. |
| BIC         | Bitwise Clear | `BIC Rd, Rn, Rm` | Clears specific bits. |
| MVN         | Bitwise NOT   | `MVN Rd, Rm`     | Inverts all bits.     |

### Shift Instructions

Shift instructions move bits left or right within a register.

| Instruction | Mnemonic                 | Assembler Syntax | Description                       |
| ----------- | ------------------------ | ---------------- | --------------------------------- |
| LSL         | Logical Shift Left       | `LSL Rd, Rm, #n` | Shifts left, filling with zeros.  |
| LSR         | Logical Shift Right      | `LSR Rd, Rm, #n` | Shifts right, filling with zeros. |
| ASR         | Arithmetic Shift Right   | `ASR Rd, Rm, #n` | Preserves the sign bit.           |
| ROR         | Rotate Right             | `ROR Rd, Rm, #n` | Rotates right by n bits.          |
| RRX         | Rotate Right with Extend | `RRX Rd, Rm`     | Rotates right with carry flag.    |

### Move Instructions

Move instructions copy data between registers.

| Instruction | Mnemonic       | Assembler Syntax  | Description                                 |
| ----------- | -------------- | ----------------- | ------------------------------------------- |
| MOV         | Move           | `MOV Rd, Rm`      | Moves a value from one register to another. |
| MOV        | Move immediate | `MOV Rd, #imm`   | Moves an immediate value to Rd       |
| MVN         | Move Negative  | `MVN Rd, Rm`      | Moves the bitwise NOT of a register.        |
| MOVT        | Move Top       | `MOVT Rd, #imm16` | Moves the top 16 bits of a 32-bit value.    |

## Load and Store Instructions

Load and store instructions transfer data between registers and memory.

| Instruction | Mnemonic       | Assembler Syntax | Description                                |
| ----------- | -------------- | ---------------- | ------------------------------------------ |
| LDR         | Load Register  | `LDR Rd, [Rn]`   | Loads a value from memory into a register. |
| STR         | Store Register | `STR Rd, [Rn]`   | Stores a register value into memory.       |

## Control Flow Instructions

Control flow instructions change the execution sequence of a program.

| Instruction | Mnemonic            | Assembler Syntax | Description                                             |
| ----------- | ------------------- | ---------------- | ------------------------------------------------------- |
| B           | Branch              | `B label`        | Jumps to a target address unconditionally.              |
| BL          | Branch with Link    | `BL label`       | Calls a subroutine (stores return address in LR).       |
| BX          | Branch and Exchange | `BX Rm`          | Branches to an address and switches ARM/Thumb mode.     |
| CMP         | Compare             | `CMP Rn, Rm`     | Compares two registers (`Rn - Rm`, updates flags only). |
| CMN         | Compare Negative    | `CMN Rn, Rm`     | Compares two registers (`Rn + Rm`, updates flags only). |
| TST         | Test Bits           | `TST Rn, Rm`     | Performs `Rn & Rm`, updates flags only.                 |
| TEQ         | Test Equivalence    | `TEQ Rn, Rm`     | Performs `Rn ^ Rm`, updates flags only.                 |

For more instructions and their details, refer to the [ARM Instruction Set Summary](https://developer.arm.com/documentation/100165/0201/Programmers-Model/Instruction-set-summary/Processor-instructions).

---

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
    MOV R0, #7 ; x = 7
    MUL R1, R0, R0 ; R1 = x^2
    MOV R4, #5
    MUL R1, R1, R4
    MOV R5, #6
    MUL R2, R0, R5 ; R2 = 6x
    SUB R3, R1, R2 ; R3 = 5x^2 - 6x
    ADD R3, R3, #8 ; R3 = 5x^2 - 6x + 8
    ALIGN
STOP
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
SUMP DCD SUM
NUM1 DCD 5
NUM2 DCD 7
    AREA MYDATA, DATA, READWRITE
SUM DCD 0
    AREA MYCODE, CODE, READONLY
    ENTRY
    EXPORT Reset_Handler
Reset_Handler
    LDR R1, NUM1
    LDR R2, NUM2
    MOV R0, #0
    ADD R0, R1, R2
    SUBS R0, R0, #1
    LSLS R3, R0, #2 ; Logical shift left by 2 bits with flag update
    LDR R4, SUMP
    STR R3, [R4]
    LDR R6, [R4]
    ALIGN
STOP
    B STOP
    END
```

## Lab Work

Write a program to conver temperatures from `Celisius` to `Fahrenhiet` and from `Fahrenhiet` to `Celsius` using the following formulas.

$$ C = \frac{5 \times (F - 32)}{9}$$
$$ F = \frac{9 \times C}{5} + 32$$

### Criteria

1. Use R0 to store the initial `Celsius` temperature, say 22.
2. Use R1 to store the initial `Fahrenhiet`, say 70.
3. Use R2 to store the converted temperature in `Fahrenhiet`.
4. Use R3 to store the converted temperature in `Celsius`.