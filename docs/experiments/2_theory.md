---
outline: deep
lastUpdated: true
---
# Experiment 2: Data Processing and Memory Operations
::: info Complete Lab Manual
For the complete experiment including learning objectives, theoretical background, and detailed explanations, download the PDF manual:
 [**Download Experiment 2 PDF**](https://raw.githubusercontent.com/mabushelbaia/encs4110/main/manual/encs4110_02.pdf)
:::

## Learning Objectives

After completing this experiment, you will be able to:

- Perform arithmetic, logical, and shift/rotate operations using data-processing instructions (including `Operand2` with the barrel shifter).
- Move data between registers and memory using load/store instructions with immediate, register-offset, and pre-/post-indexed addressing modes.
- Declare and initialize data objects (arrays, strings, buffers) with assembler directives, and use pointers to access and modify them.
- Trace how instructions affect registers and xPSR flags using the Keil uVision5 debugger (breakpoints, single-step, register/memory views).

## Experiment Overview

This experiment develops fluency with the ARM Cortex-M4 instruction set for data manipulation and memory access. You will practice arithmetic, logical, and shift/rotate instructions, apply various addressing modes for load/store operations, and define data structures with assembler directives. By the end of this lab, you will be able to write assembly routines that perform register-level computation and structured memory access, providing the foundation for flow control and procedure calls in later experiments.



## Theoretical Background

As mentioned in Experiment 1, assembly instructions are split into three main categories: data processing, load/store, and branch instructions. This experiment focuses on data processing instructions, load/store instructions and their addressing modes. Branch instructions and flow control will be covered in the next experiment.

### Data Processing Instructions

Data processing instructions perform arithmetic and logical operations on data stored in registers. They can also manipulate the condition flags in the xPSR based on the results of the operations. Common data processing instructions take the following form:

```
{LABEL}   OPCODE{<cond>}{S} Rd, Rn, Operand2
```

where:

- `LABEL`: optional label for branching.
- `OPCODE`: the operation to be performed (e.g., `ADD`, `SUB`, `AND`, `ORR`).
- `<cond>`: optional condition code that predicates execution.
- `S`: optional suffix indicating whether to update the condition flags.
- `Rd`: destination register where the result is stored.
- `Rn`: first operand register.
- `Operand2`: second operand, which can be an immediate value limited to 8 bits, a register, or a barrel shifter operation.

#### Arithmetic Instructions

Arithmetic instructions perform basic mathematical operations. Some common arithmetic instructions include addition, subtraction, multiplication, and their variants.

| Instr. | Syntax | Operation | Description |
|--------|--------|-----------|-------------|
| ADD | `ADD{S} Rd, Rn, Operand2` | $Rd \leftarrow Rn + Operand2$ | Operand2 may be a register, an immediate, or a shifted register. |
| ADC | `ADC{S} Rd, Rn, Operand2` | $Rd \leftarrow Rn + Operand2 + C$ | Adds carry-in C. |
| SUB | `SUB{S} Rd, Rn, Operand2` | $Rd \leftarrow Rn - Operand2$ | Standard subtraction. |
| SBC | `SBC{S} Rd, Rn, Operand2` | $Rd \leftarrow Rn - Operand2 - \overline{C}$ | Subtract with carry. If carry flag is clear, result is reduced by one. Used for multiword arithmetic. |
| RSB | `RSB{S} Rd, Rn, Operand2` | $Rd \leftarrow Operand2 - Rn$ | Reverse subtract. |
| MUL | `MUL{S} Rd, Rn, Rm` | $Rd \leftarrow (Rn \times Rm)_{[31:0]}$ | 32×32 → low 32 bits. |
| MLA | `MLA Rd, Rn, Rm, Ra` | $Rd \leftarrow (Rn \times Rm) + Ra$ | Multiply-accumulate. |
| MLS | `MLS Rd, Rn, Rm, Ra` | $Rd \leftarrow Ra - (Rn \times Rm)$ | Multiply-subtract. |
| UMULL | `UMULL RdLo, RdHi, Rn, Rm` | $\{RdHi,RdLo\} \leftarrow Rn \times Rm$ | Unsigned 32×32 → 64-bit product. |
| SMULL | `SMULL RdLo, RdHi, Rn, Rm` | $\{RdHi,RdLo\} \leftarrow Rn \times Rm$ | Signed 32×32 → 64-bit product. |
| UDIV | `UDIV Rd, Rn, Rm` | $Rd \leftarrow Rn \div Rm$ | Unsigned division. |
| SDIV | `SDIV Rd, Rn, Rm` | $Rd \leftarrow Rn \div Rm$ | Signed division. |

*Note: C denotes the carry flag in xPSR.*

#### Logical and Move Instructions

Logical instructions perform bitwise operations on data, while move instructions transfer data between registers or load immediate values.

| Instr. | Syntax | Operation | Description |
|--------|--------|-----------|-------------|
| AND | `AND Rd, Rn, Operand2` | $Rd \leftarrow Rn \,\&\, Operand2$ | Bitwise AND. |
| ORR | `ORR Rd, Rn, Operand2` | $Rd \leftarrow Rn \,|\, Operand2$ | Bitwise OR. |
| EOR | `EOR Rd, Rn, Operand2` | $Rd \leftarrow Rn \oplus Operand2$ | Bitwise XOR. |
| BIC | `BIC Rd, Rn, Operand2` | $Rd \leftarrow Rn \,\&\, \neg Operand2$ | Bit clear. |
| MVN | `MVN Rd, Operand2` | $Rd \leftarrow \neg Operand2$ | Bitwise NOT of operand. |
| MOV | `MOV Rd, Operand2` | $Rd \leftarrow Operand2$ | Register or immediate move. |
| MOVW | `MOVW Rd, #imm16` | $Rd[15:0] \leftarrow imm16$ | Write low halfword. |
| MOVT | `MOVT Rd, #imm16` | $Rd[31:16] \leftarrow imm16$ | Write high halfword (low preserved). |

In this experiment, you will work with bitwise logical instructions to manipulate individual bits within registers. Such operations are fundamental in microcontroller programming, where control and status registers often contain multiple configuration fields packed into a single 32-bit word.

**Set and Clear Bits:**

- `ORR Rd, Rn, #mask`: Sets bits in `Rd` where the corresponding bits in `mask` are 1.
- `BIC Rd, Rn, #mask`: Clears bits in `Rd` where the corresponding bits in `mask` are 1.
- `EOR Rd, Rn, #mask`: Toggles bits in `Rd` where the corresponding bits in `mask` are 1.

BIC is essentially an AND operation with the negated mask, i.e., `BIC Rd, Rn, #mask` is equivalent to `AND Rd, Rn, #~mask`.

**Check Bits:**

- `AND Rd, Rn, #mask`: Isolates bits in `Rn` where the corresponding bits in `mask` are 1.
- Use `CMP Rd, #0` to determine if the result is zero.
- Alternatively, use `TST Rn, #mask`, which performs the AND operation and updates the condition flags without storing the result.

#### Shift and Rotate Instructions

| Instr. | Syntax | Operation | Description |
|--------|--------|-----------|-------------|
| LSL | `LSL Rd, Rm, #sh\|Rs` | $Rd \leftarrow Rm \ll sh$ | Logical left shift by immediate or by register. |
| LSR | `LSR Rd, Rm, #sh\|Rs` | $Rd \leftarrow Rm \gg sh$ | Logical right shift (zero fill). |
| ASR | `ASR Rd, Rm, #sh\|Rs` | $Rd \leftarrow Rm \gg sh$ | Arithmetic right shift (sign fill). |
| ROR | `ROR Rd, Rm, #sh\|Rs` | $Rd \leftarrow \mathrm{ROR}(Rm, sh)$ | Rotate right by immediate or by register. |
| RRX | `RRX Rd, Rm` | $Rd \leftarrow \mathrm{ROR}_{C}(Rm, 1)$ | Rotate right 1 bit through carry. |

*Note: Shift amount can be an immediate #sh (0–31) or a register Rs (low 8 bits used).*

Not all shift/rotate instructions are explicitly present in the ARMv7-M ISA. For example, there is no ROL (rotate left) or ASL (arithmetic shift left) instruction, as these operations can be achieved using existing shift instructions.

#### Barrel Shifter

The barrel shifter is a hardware feature that allows for efficient shifting and rotating of register values as part of data processing instructions. It can perform operations such as logical shifts (left or right), arithmetic shifts, and rotations on the second operand (`Operand2`) before it is used in the instruction without wasting extra instructions or cycles.

Examples of barrel shifter usage:
```asm
    ADD    R0, R2, R1, LSL #2    ; R0 = R2 + (R1 << 2) using barrel shifter
    SUB    R3, R4, R5, LSR #1    ; R3 = R4 - (R5 >> 1) using barrel shifter
    ORR    R6, R7, R8, ROR #3    ; R6 = R7 | (R8 rotated right by 3)
```

### Load and Store Instructions

Since the ARM Cortex-M4 follows the RISC design philosophy, it uses a load/store architecture. This means that arithmetic and logical instructions operate only on registers. Any data in memory must first be loaded into a register before processing, and results must be stored back to memory if they need to be preserved.

| Instr. | Syntax Example | Description |
|--------|---------------|-------------|
| LDR / STR | `LDR/STR Rt, [Rn, #off]` | Load/store a 32-bit word. |
| LDRB / STRB | `LDRB/STRB Rt, [Rn, #off]` | Load/store an 8-bit byte. |
| LDRH / STRH | `LDRH/STRH Rt, [Rn, #off]` | Load/store a 16-bit halfword. |
| LDRSB / LDRSH | `LDRSB/LDRSH Rt, [Rn, #off]` | Load signed byte/halfword and sign-extend to 32 bits. |
| LDRD / STRD | `LDRD/STRD Rt, Rt2, [Rn, #off]` | Load/store a 64-bit doubleword (two registers). |

#### Declaring Data in Memory

Data in assembly is defined using **assembler directives** that reserve and optionally initialize memory. Common directives include `DCD`, `DCW`, and `DCB`, which define words, halfwords, and bytes, respectively.

- `DCD` — Define Constant Word (32 bits per element)
- `DCW` — Define Constant Halfword (16 bits per element)
- `DCB` — Define Constant Byte (8 bits per element)
- `SPACE` — Reserve uninitialized memory (in bytes)
- `FILL` — Fill memory with a specified value for a given length

```asm
            AREA    CONSTANTS, DATA, READONLY
NUMBERS     DCD     10, 20, 30, 40        ; array of 32-bit integers
BYTES       DCB     1, 2, 3, 4            ; array of bytes
TEXT        DCB     "HELLO",0             ; null-terminated ASCII string
            AREA    MYDATA, DATA, READWRITE
BUFFER      SPACE   64                    ; reserve 64 bytes (uninitialized)
PATTERN     FILL    0xFF, 64              ; fill 64 bytes with 0xFF
```

Each label marks the starting address of a data object in memory.

#### Understanding Pointer Declarations

The directive `YPTR DCD YVAL` reserves a 32-bit word at the label `YPTR` and initializes it with the address of `YVAL`. In other words, `YPTR` acts as a **pointer variable** that holds the address of another variable (`YVAL`). Executing `LDR Rn, YPTR` loads the 32-bit contents stored at `YPTR`—that is, the address of `YVAL`—into `Rn`.

| Address | Label | Contents |
|---------|-------|----------|
| 0x2000 | XVAL | 0x12345678 |
| 0x2004 | YPTR | 0x2008 (address of YVAL) |
| 0x2008 | YVAL | 0x00000000 |

#### Loading Addresses and Values: `LDR`, `LDR =`, and `ADR`

- **LDR Rn, label**: Loads the 32-bit *value* stored at the memory address identified by `label` into register `Rn`.
- **LDR Rn, =label**: Loads the *address* of `label` into `Rn`, rather than the data stored at that address.
- **ADR Rn, label**: Loads the *address* of `label` into `Rn` by computing it relative to the current program counter. This method requires no literal pool or memory access but only works for nearby addresses (about ±4 KB in Thumb mode).

### Addressing Modes

Addressing modes define how the effective address or operand value is obtained by an instruction. The ARM Cortex-M4 supports several common addressing modes:

| Mode | Syntax Example | Description |
|------|---------------|-------------|
| Immediate | `MOV R0, #10` | Operand is a constant value encoded in the instruction. |
| Register Direct | `MOV R0, R1` | Operand is taken directly from a register. |
| Register Indirect | `LDR R0, [R1]` | Register holds the address of the operand in memory. |
| Register Offset | `LDR R0, [R1, R2]` | Effective address = base register + offset register. |
| Immediate Offset | `LDR R0, [R1, #4]` | Effective address = base register + constant offset. |
| Pre-indexed | `LDR R0, [R1, #4]!` | Base updated first, then memory access. |
| Post-indexed | `LDR R0, [R1], #4` | Memory access first, then base register updated. |

```asm
; Immediate Offset
    LDR     R0, [R1, #4]     ; R0 = word at memory[R1 + 4]
; Register Offset
    LDR     R0, [R1, R2]    ; R0 = word at memory[R1 + R2]
; Pre-indexed
    LDR     R0, [R1, #4]!    ; R1 = R1 + 4, then load R0 = [R1]
; Post-indexed
    LDR     R0, [R1], #4     ; load R0 = [R1], then R1 = R1 + 4
```