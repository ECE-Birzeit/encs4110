---
outline: deep
lastUpdated: true
---
# Experiment 3: Control Flow and Subroutines
::: info Complete Lab Manual
For the complete experiment including learning objectives, theoretical background, and detailed explanations, download the PDF manual:
 [**Download Experiment 3 PDF**](https://raw.githubusercontent.com/mabushelbaia/encs4110/main/manual/encs4110_03.pdf)
:::


## Learning Objectives

After completing this experiment, you will be able to:

- Implement conditional and unconditional branching using ARM branch instructions.
- Design and implement loop constructs (`for`, `while`) using compare and branch instructions.
- Create and call procedures with proper parameter passing and return mechanisms.
- Manage the stack for local variables, parameter passing, and nested procedure calls.
- Apply the ARM Procedure Call Standard (AAPCS) to ensure correct register usage and calling conventions.

## Experiment Overview

This experiment introduces program control flow, procedure implementation, and stack management on the ARM Cortex-M4 processor. You will implement conditional branches and loops, write procedures following the ARM Procedure Call Standard (AAPCS), and handle nested calls with parameter passing using the stack. By the end of this lab, you will understand how to create modular assembly programs with proper control flow, calling conventions, and stack management.



## Theoretical Background

### Flow Control

Flow control instructions alter the sequential execution of instructions by changing the program counter (PC). These instructions enable the implementation of conditional statements, loops, and procedure calls.

#### Condition Evaluation

Before implementing control flow, we must understand how to evaluate conditions and set processor flags. ARM provides dedicated instructions for comparing values and testing bit patterns that update the condition flags without storing results.

| Instr. | Syntax | Description / Usage |
|--------|--------|---------------------|
| CMP | `CMP Rn, Operand2` | Compare `Rn` with `Operand2` (`Rn - Operand2`); updates flags (Z, N, C, V). |
| CMN | `CMN Rn, Operand2` | Compare negative (`Rn + Operand2`); used for checking against negative values. |
| TST | `TST Rn, Operand2` | Logical AND test (`Rn AND Operand2`); sets Z if all tested bits are 0. |
| TEQ | `TEQ Rn, Operand2` | Logical XOR test (`Rn EOR Operand2`); sets Z if operands are equal. |

**Usage Notes:**
- These instructions only affect the condition flags (N, Z, C, V) — they do not store a result.
- `CMP`/`CMN` are arithmetic comparisons; `TST`/`TEQ` are logical bitwise comparisons.
- Many data-processing instructions can update flags by appending `S` (e.g., `ADDS`, `SUBS`).
- Common use: immediately followed by conditional branches such as `BEQ`, `BNE`, `BGT`, etc.

Examples:
```asm
    CMP     R0, #10          ; Compare R0 with 10
    BLT     LessThan10       ; Branch if R0 < 10
    BGE     GreaterOrEqual   ; Otherwise, R0 >= 10
```

```asm
    MOV     R1, #0x12        ; R1 = 0001 0010b
    TST     R1, #0x10        ; Test if bit 4 is set
    BEQ     BitClear         ; Branch if bit 4 = 0 (Z=1)
    BNE     BitSet           ; Branch if bit 4 = 1 (Z=0)
```

```asm
    MOV     R2, #0x55
    MOV     R3, #0x55
    TEQ     R2, R3           ; XOR -> result 0, sets Z=1
    BEQ     ValuesEqual      ; Branch if equal
```

#### Conditional Branching

Branch instructions are the primary mechanism for implementing flow control in ARM assembly. They modify the program counter to jump to different parts of the code based on conditions or unconditionally.

| Instr. | Syntax | Description / Usage |
|--------|--------|---------------------|
| B | `B label` | Unconditional branch to `label` (always jumps) |
| B\<cond\> | `B<cond> label` | Conditional branch based on flags |
| BL | `BL label` | Branch with link: calls a subroutine, storing return address in `LR`. |
| BX | `BX Rm` | Branch to address in register, often `BX LR` to return from a subroutine. |
| CBZ | `CBZ Rn, label` | Branch if `Rn == 0`. Example: `CBZ R0, Done`. |
| CBNZ | `CBNZ Rn, label` | Branch if `Rn != 0`. |

`CBZ`/`CBNZ` instructions have specific constraints:
- **Register**: operand must be a low register R0–R7.
- **Range**: branch is *forward-only*; the destination must be within 0–126 bytes after the instruction.
- **Flags**: does not update condition flags (N, Z, C, V).

For backward or longer jumps, use `CMP`/`TST` with conditional branches (`BEQ`, `BNE`, `BGT`, …).

#### Conditional Execution

ARM assembly supports conditional execution, where most instructions can be conditionally executed based on the current state of the condition flags. This feature allows for efficient implementation of conditional statements without explicit branching.

**Conditional Instruction Format:**
Most ARM instructions can be made conditional by appending a condition code suffix:
```
OPCODE{<cond>} Rd, Rn, Operand2
```

**Examples:**
- `ADDEQ R0, R1, R2` — Add only if equal (Z=1)
- `MOVNE R3, #10` — Move only if not equal (Z=0)
- `SUBGT R4, R4, #1` — Subtract only if greater than (signed)

**Advantages of Conditional Execution:**
- **Performance**: Eliminates branch instructions for simple conditional operations
- **Code density**: Reduces the number of instructions needed
- **Pipeline efficiency**: Avoids branch prediction penalties for simple conditions
- **Atomic operations**: Multiple related conditional operations can be grouped

#### How Branch Instructions Work

Branch instructions change the flow of execution by modifying the Program Counter (`PC`). When a branch is executed, the instruction encodes an *offset* which is added to the current value of the `PC`.

**Offset calculation:**
The branch instruction contains a signed immediate value (positive or negative). The processor adds this offset (aligned to halfword boundaries) to the current `PC`.

- A *positive offset* causes a **forward branch** (jump to a higher memory address, later in the program).
- A *negative offset* causes a **backward branch** (jump to a lower memory address, earlier in the program).

**Example:**
Suppose a branch instruction is located at address `0x100`, and the assembler encodes an immediate offset of `-0x08`. The effective target address will be:
```
0x100 + 4 + (-0x08) = 0xFC
```

#### Condition Codes

Conditional branches use condition codes that test the processor status flags (N, Z, C, V) set by previous instructions.

| Cond. | Meaning | Description |
|-------|---------|-------------|
| EQ | Equal | Execute if Z=1. |
| NE | Not equal | Execute if Z=0. |
| CS/HS | Carry set / Unsigned higher or same | Execute if C=1. |
| CC/LO | Carry clear / Unsigned lower | Execute if C=0. |
| MI | Minus (negative) | Execute if N=1. |
| PL | Plus (non-negative) | Execute if N=0. |
| VS | Overflow set | Execute if V=1. |
| VC | Overflow clear | Execute if V=0. |
| HI | Unsigned higher | Execute if C=1 and Z=0. |
| LS | Unsigned lower or same | Execute if C=0 or Z=1. |
| GE | Greater or equal (signed) | Execute if N=V. |
| LT | Less than (signed) | Execute if N≠V. |
| GT | Greater than (signed) | Execute if Z=0 and N=V. |
| LE | Less or equal (signed) | Execute if Z=1 or N≠V. |
| AL | Always | Always execute (default if no condition). |
| NV | Never | Reserved / do not use. |

### Loop Patterns

Loops are fundamental control structures that repeat a block of code based on conditions. ARM assembly implements loops using combinations of compare instructions, conditional branches, and counters.

#### For Loop Structure

A typical for loop has the structure: initialization, condition check, body execution, and increment/decrement.

```asm
        ; Initialization
        MOV     R0, #0                  ; i = 0
        MOV     R1, #0                  ; sum = 0
        LDR     R3, =array              ; load base address of array into R3

for_start
        ; Condition check
        CMP     R0, #length             ; compare i with length
        BGE     for_end                 ; if i >= length, exit loop

        ; Loop body
        LDR     R2, [R3, R0, LSL #2]    ; load array[i]; EA: R3 + (R0 * 4)
        ADD     R1, R1, R2              ; sum += array[i]

        ; Increment
        ADD     R0, R0, #1              ; i++
        B       for_start               ; repeat

for_end
```

#### While Loop Structure

While loops check the condition before executing the loop body, potentially executing zero times if the initial condition is false.

```asm
    ; Initialization
    LDR     R0, =mystring   ; pointer to string
    MOV     R1, #0          ; character count = 0

while_start
                            ; Condition check
    LDRB    R2, [R0], #1    ; load current character and post-increment pointer
    CMP     R2, #0          ; check for null terminator
    BEQ     while_end       ; if zero, exit loop
    
                            ; Loop body - do something with R2

    B       while_start     ; repeat
while_end
```

Note: `0` and `'0'` are two different values, as the former is actually zero, while the latter is the ASCII code for the character '0' (which is 48 in decimal).

### Procedures and Stack

#### ARM Architecture Procedure Call Standard (AAPCS)

Procedures are reusable blocks of code that encapsulate a specific task. They promote modular design, code reuse, and clearer program structure. In ARM assembly, procedures are implemented using branch-and-link instructions along with register usage conventions defined by the ARM Architecture Procedure Call Standard (AAPCS).

The AAPCS is the set of rules that define how functions exchange data and how registers must be preserved during a procedure call:

- **R0–R3**: Hold the first four parameters. `R0` also holds the return value. Caller-saved.
- **Stack**: Any additional parameters beyond the first four are passed on the stack.
- **R4–R11**: Must be preserved by the callee. If a procedure uses them, it must save and restore them.
- **SP (R13)**: Stack pointer, always points to the current top of the stack.
- **LR (R14)**: Link register holds the return address. Caller-saved.

*Note: Callees are the procedures being called, while callers are the ones calling the procedure.*

#### Procedure Templates

A procedure is entered with a `BL` (branch-with-link) instruction, which stores the return address in the link register `LR`. The callee returns by branching to `LR` (e.g., `BX LR`). By the AAPCS, the first four arguments are passed in R0–R3 and the primary return value is placed in R0.

**Basic Procedure Template:**
```asm
AddTwo  PROC
        ADD R0, R0, R1   ; return R0+R1 in R0
        BX  LR
        ENDP
```

*Note: The `PROC` and `ENDP` directives help define the start and end of a procedure and they could be safely omitted.*

**Procedure with Preserved Registers:**
When a procedure uses callee-saved registers (R4-R11), it must preserve their original values by saving them on the stack and restoring them before returning:

```asm
ProcessArray PROC
        PUSH    {R4-R6, LR}     ; Save used registers and LR
        MOV     R4, R0          ; Save array pointer
        MOV     R5, R1          ; Save array length  
        MOV     R6, #0          ; Initialize counter
        
        ; Process array using R4, R5, R6...
        
        MOV     R0, R6          ; Return counter value
        POP     {R4-R6, PC}     ; Restore registers and return
        ENDP
```

**Nested Procedure Calls:**
When one procedure calls another, the link register (`LR`) must be preserved, otherwise the return address would be lost.

```asm
OuterProc
        PUSH    {LR}            ; Save return address
        BL      InnerProc       ; Call inner procedure
        MOV     R1, R0          ; Use return value
        POP     {PC}            ; Return to caller

InnerProc 
        MOV     R0, #42         ; Return value
        BX      LR              ; Return
```

#### Stack Model (Full, Descending)

There are four common stack models based on two characteristics: whether the stack is full or empty, and whether it grows up (ascending) or down (descending).

- **Full Ascending**: SP points to the last used location; stack grows toward higher addresses.
- **Empty Ascending**: SP points to the next free location; stack grows toward higher addresses.
- **Empty Descending**: SP points to the next free location; stack grows toward lower addresses.
- **Full Descending**: SP points to the last used location; stack grows toward lower addresses.

The ARM Cortex-M4 uses a **full descending stack**, meaning:
- The stack pointer (`SP`) points to the last used location (full).
- The stack grows toward lower memory addresses (descending).

#### Stack Operations

The Cortex-M4 provides `PUSH` and `POP` instructions that automatically update the stack pointer (`SP`) and allow saving or restoring multiple registers in a single instruction.

**PUSH:**
The `PUSH` instruction saves one or more registers onto the stack in a single operation. When executed, the stack pointer (`SP`) is decremented to reserve space, and the specified registers are written to consecutive memory locations.

**Rule:** `PUSH` stores registers on the stack, with the **lowest-numbered register** placed at the **lowest memory address** and the **highest-numbered register** placed at the **highest memory address**.

Example:
```asm
    PUSH {R4, R0, R2, LR}
```

Even if the list appears unordered, values are laid out by register number: R0 (lowest address), then R2, R4, and LR (highest address). Each register occupies 4 bytes, so `SP` decreases by 16 bytes in total.

**POP:**
The `POP` instruction restores one or more registers from the stack. After all specified registers are restored, `SP` has increased by 4 bytes per register.

**Rule:** `POP` loads registers from the stack such that the **lowest-numbered register** is restored from the **lowest memory address**, and the **highest-numbered register** from the **highest memory address**.

If `PC` is included in the list, the loaded value becomes the new program counter, returning from the current procedure.

Example:
```asm
    POP {R6-R8}
```

This restores R6, then R7, then R8 from successively higher addresses; the stack pointer increases by 12 bytes overall.