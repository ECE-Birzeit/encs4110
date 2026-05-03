---
outline: deep
lastUpdated: true
prev:
  text: "2. Data Processing and Memory Operations - Theory"
  link: /experiments/2-theory
next:
  text: "3. Control Flow and Subroutines - Theory"
  link: /experiments/3-theory
---

# Data Processing and Memory Operations


## Examples

### Example 1: Data Processing Instructions
This example demonstrates various arithmetic and bitwise operations on registers.

<<< @/snippets/assembly/exp2/example1.asm

### Example 2: Load/Store with Different Addressing Modes
This example demonstrates load and store instructions using various addressing modes.

<<< @/snippets/assembly/exp2/example3.asm

## Tasks

### Task 1: Bitwise Register Manipulation
Start with `R0 = 0x12345678`. Perform the following operations and observe the results in the debugger:
- Clear bits 4--7 (second hex nibble).
- Set bits 8--11 (force nibble to `F`).
- Toggle bits 28--31 (highest nibble).

*Hint:* Use `BIC`, `ORR`, and `EOR` with appropriate masks.

### Task 2: Addressing Modes with an Array
Given the following array:
```asm
ARRAY   DCD     0x11, 0x22, 0x33, 0x44
OUT     SPACE   16
```
Load each element using a different addressing mode, then store to `OUT`:
- 0x11 via immediate offset
- 0x22 via pre-indexed
- 0x33 via post-indexed (load first, then increment pointer)
- 0x44 via register offset

Hint: Put ARRAY's base in R1 (e.g., `LDR R1, =ARRAY`). Verify `OUT` in memory after execution.