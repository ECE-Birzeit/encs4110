---
title: "Branching and Loops"
description: "This experiment introduces program flow control on the ARM Cortex-M4 processor."
outline: deep
lastUpdated: true
manual: "/manual/encs4110_manual.pdf#nameddest=chapter.3"
part: "Part 1: ARM Assembly"
---

# Branching and Loops

This experiment introduces program flow control on the ARM Cortex-M4 processor. You will compare values and test bits to set the condition flags, use conditional branches and conditional execution to make decisions, and combine them into `for` and `while` loops that walk through arrays and strings. By the end of this lab, you will be able to translate high-level `if`, `for`, and `while` constructs into assembly. Procedures and the stack build on this in Experiment 4.

## Objectives

- Set the condition flags with compare and test instructions (`CMP`, `CMN`, `TST`, `TEQ`).
- Implement conditional and unconditional branching using ARM branch instructions.
- Choose the correct signed or unsigned condition code for a comparison.
- Use conditional execution (and the `IT` block behind it) to avoid short branches.
- Design and implement loop constructs (`for`, `while`) that process arrays and null-terminated strings.

## Background

### Flow Control

Flow control instructions alter the sequential execution of instructions by changing the program counter (PC). These instructions enable the implementation of conditional statements, loops, and procedure calls that are fundamental to structured programming.

#### Condition Evaluation

Before implementing control flow, we must understand how to evaluate conditions and set processor flags. ARM provides dedicated instructions for comparing values and testing bit patterns that update the condition flags without storing results.

| **Instr.** | **Syntax** | **Description / Usage** |
|:---|:---|:---|
| CMP | `CMP Rn, Operand2` | Compare `Rn` with `Operand2` (`Rn - Operand2`); updates flags (Z, N, C, V). |
| CMN | `CMN Rn, Operand2` | Compare negative: computes `Rn + Operand2` and sets flags as if by `ADD`, i.e. tests whether `Rn == -Operand2`. |
| TST | `TST Rn, Operand2` | Logical AND test (`Rn AND Operand2`); sets Z if all tested bits are 0. |
| TEQ | `TEQ Rn, Operand2` | Logical XOR test (`Rn EOR Operand2`); sets Z if operands are equal. |

*ARM Cortex-M4 Compare and Test Instructions*

**Usage Notes:**
- These instructions only affect the condition flags (N, Z, C, V) — they do not store a result.
- `CMP`/`CMN` are arithmetic comparisons; `TST`/`TEQ` are logical bitwise comparisons.
- Many data-processing instructions can update flags by appending `S` (e.g., `ADDS`, `SUBS`).
- Common use: immediately followed by conditional branches such as `BEQ`, `BNE`, `BGT`, etc.

**Examples:**
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

| **Instr.** | **Syntax** | **Description / Usage** |
|:---|:---|:---|
| B | `B label` | Unconditional branch to `label` (always jumps) |
| B`<cond>` | `B<cond> label` | Conditional branch based on flags |
| BL | `BL label` | Branch with link: calls a subroutine, storing return address in `LR`. Procedures are covered in Experiment 4. |
| BX | `BX Rm` | Branch to address in register, often `BX LR` to return from a subroutine. |
| CBZ | `CBZ Rn, label` | Branch if `Rn == 0`. Example: `CBZ R0, Done`. |
| CBNZ | `CBNZ Rn, label` | Branch if `Rn != 0`. Example: loop until counter reaches zero. |

*ARM Cortex-M4 Branch Instructions*

##### `CBZ/CBNZ`

instructions have specific constraints:

- **Register**: operand must be a low register `R0--R7`.
- **Range**: branch is *forward-only*; the destination must be within 0–126 bytes after the instruction.
- **Flags**: does not update condition flags (`N, Z, C, V`).

For backward or longer jumps, use `CMP`/`TST` with conditional branches (`BEQ`, `BNE`, `BGT`, …).

#### Conditional Execution

ARM assembly supports conditional execution, where most instructions can be conditionally executed based on the current state of the condition flags. This feature allows for efficient implementation of conditional statements without explicit branching.

**Conditional Instruction Format:**
Most ARM instructions can be made conditional by appending a condition code suffix:
$$
\texttt{OPCODE\{<cond>\} Rd, Rn, Operand2}
$$

**Examples:**
- `ADDEQ R0, R1, R2` — Add only if equal (Z=1)
- `MOVNE R3, #10` — Move only if not equal (Z=0)
- `SUBGT R4, R4, #1` — Subtract only if greater than (signed)

**Advantages of Conditional Execution:**
- **Performance**: Eliminates branch instructions for simple conditional operations
- **Code density**: Reduces the number of instructions needed
- **Atomic operations**: Multiple related conditional operations can be grouped

**The IT block:**
Cortex-M4 executes only Thumb-2 code, where a condition suffix on a non-branch instruction (`MOVGT`, `ADDEQ`, …) is valid only inside an `IT` (If-Then) block. `IT` makes the next one to four instructions conditional: each one is marked `T` (execute if the condition holds) or `E` (execute if it does not), and every instruction in the block must carry the matching condition suffix.

```asm
CMP     R3, R2
    ITE     GT               ; next 2 instructions: Then, Else
    MOVGT   R2, R3           ; executes if R3 > R2
    MOVLE   R4, #0           ; executes otherwise
```

If you write a conditional instruction without an `IT`, the Keil assembler normally inserts one for you. Look for it in the Disassembly window when you single-step Example 1 below. Conditional *branches* (`BEQ`, `BGT`, …) are encoded differently and never need an `IT`.

#### How Branch Instructions Work

Branch instructions change the flow of execution by modifying the Program Counter (`PC`). When a branch is executed, the instruction encodes an *offset* which is added to the current value of the `PC`.

**Offset calculation:**
The branch instruction contains a signed immediate value (positive or negative). The processor adds this offset (aligned to halfword boundaries) to the current `PC`.

- A *positive offset* causes a **forward branch** (jump to a higher memory address, later in the program).
- A *negative offset* causes a **backward branch** (jump to a lower memory address, earlier in the program).

**Example:**
Suppose a branch instruction is located at address `0x100`, and the assembler encodes an immediate offset of `-0x08`. The effective target address will be:
$$
0x100 + 4 + (-0x08) = 0xFC
$$
This means the processor jumps **backward** to an earlier instruction. Such negative offsets are typically used to implement loops (e.g., repeat until zero).

#### Condition Codes

Conditional branches use condition codes that test the processor status flags (N, Z, C, V) set by previous instructions. These enable implementing high-level constructs like if-statements and loops.

| **Cond.** | **Meaning** | **Description** |
|:---|:---|:---|
| EQ | Equal | Execute if $Z=1$. |
| NE | Not equal | Execute if $Z=0$. |
| CS/HS | Carry set / Unsigned higher or same | Execute if $C=1$. |
| CC/LO | Carry clear / Unsigned lower | Execute if $C=0$. |
| MI | Minus (negative) | Execute if $N=1$. |
| PL | Plus (non-negative) | Execute if $N=0$. |
| VS | Overflow set | Execute if $V=1$. |
| VC | Overflow clear | Execute if $V=0$. |
| HI | Unsigned higher | Execute if $C=1$ *and* $Z=0$. |
| LS | Unsigned lower or same | Execute if $C=0$ *or* $Z=1$. |
| GE | Greater or equal (signed) | Execute if $N=V$. |
| LT | Less than (signed) | Execute if $N\neq V$. |
| GT | Greater than (signed) | Execute if $Z=0$ *and* $N=V$. |
| LE | Less or equal (signed) | Execute if $Z=1$ *or* $N\neq V$. |
| AL | Always | Always execute (default if no condition). |
| NV | Never | Reserved / do not use. |

*Common ARM Condition Codes*

### Loop Patterns

Loops are fundamental control structures that repeat a block of code based on conditions. ARM assembly implements loops using combinations of compare instructions, conditional branches, and counters.

#### For Loop Structure

A typical for loop has the structure: initialization, condition check, body execution, and increment/decrement. This type of loop executes a known number of times.

```asm
AREA M_DATA, DATA, READONLY
array   DCD 10, 20, 30, 40, 50   ; array of 5 integers
length  EQU 5                    ; number of elements (; just a constant, no memory)
```

```asm
; Initialization
        MOV     R0, #0                  ; i = 0
        MOV     R1, #0                  ; sum = 0
        LDR     R3, =array              ; load base address of array into R3

for_start
        ; Condition check
        CMP     R0, #length             ; compare i with length
        BGE     for_end                 ; if i >= length, exit loop

        ; Loop body: scaled register-offset addressing: effective address
        ; is Rn + (Rm shifted), here R3 + (R0 * 4); this is a distinct
        ; addressing mode from the plain register-offset form covered in
        ; Experiment 2's addressing-modes table
        LDR     R2, [R3, R0, LSL #2]    ; load array[i]; EA: R3 + (R0 * 4)
        ADD     R1, R1, R2              ; sum += array[i]

        ; Increment
        ADD     R0, R0, #1              ; i++
        B       for_start               ; repeat

for_end
```

#### While Loop Structure

While loops check the condition before executing the loop body, potentially executing zero times if the initial condition is false. This type of loop is useful when the number of iterations is not known in advance and depends on dynamic conditions.

```asm
AREA M_DATA, DATA, READONLY
mystring DCB "Hello World!", 0    ; null-terminated string
```

**Note:**
`0` and `'0'` are two different values, as the former is actually zero, while the latter is the ASCII code for the character ’0’ (which is 48 in decimal).

```asm
; Intialization
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

## Examples

::: info Setting up the project
This experiment is written in assembly and runs in Keil's simulator. If you haven't made a project yet, follow the [Assembly Project](/setup/assembly-project) guide first.
:::

### Example 1: Find Maximum Element in Array
This example demonstrates how to find the maximum element in an array using a standard for loop structure.

<<< @/snippets/assembly/exp3/example1.asm

**Check:** Verify that the maximum element is correctly identified and stored in `MAXRES`.

::: info IT blocks
`MOVGT` is a conditional non-branch instruction, which Thumb-2 only allows inside an `IT` (If-Then) block. The Keil assembler normally inserts the `IT` for you. Look for it in the Disassembly window when you single-step this example.
:::

### Example 2: Count Uppercase Letters in String
This example demonstrates how to process a null-terminated string and count the number of uppercase letters (A-Z) using a while-loop structure.

<<< @/snippets/assembly/exp3/example2.asm

**Check:** Verify that the program correctly counts the uppercase letters and stores the result in `UPPERCOUNT`.

## Lab work
### Lab work 1: Count Vowels and Non-Vowels in a String
Write a program that scans the following null-terminated string, stored with `DCB`:

```asm
MYSTR   DCB  "ARM assembly language is important to learn!", 0
```

- Use R0 as the pointer to the current character.
- Count vowels (a, e, i, o, u, **upper- or lowercase**) in R1.
- Count every other character (consonants, spaces, and punctuation, but not the terminating `0`) in R2.
- Run the program and record the final values of R1 and R2. As a self-check, R1 + R2 must equal the string length.

*Hint:* Converting a letter to lowercase first (`ORR R3, R3, #0x20`) halves the number of comparisons. Think about why this is safe for letters, and why it doesn't turn a space or `!` into a vowel.

### Lab work 2: Signed Array Statistics
Given the array of signed words:

```asm
ARR     DCD  7, -3, 12, -8, 0, 5, -1
LEN     EQU  7
```

Using a single loop over the array, compute:
- the sum of all elements in R1,
- the minimum element in R2,
- the number of negative elements in R3.

Then change the condition you used to update the minimum from the signed form (`LT`/`GE`) to the unsigned form (`LO`/`HS`) and run the program again. Report the new value of R2 and explain why it changed.
