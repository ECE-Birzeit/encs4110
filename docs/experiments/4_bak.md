---
outline: deep
lastUpdated: true
---


# Arm Addressing Modes 

| Name                               | ARM Examples                               | Explanation                                                              |
|------------------------------------|--------------------------------------------|--------------------------------------------------------------------------|
| Register direct                    | `MOV R0, R1`                               | The operand is in a register.                                             |
| Direct                             | `LDR R0, MEM`                              | The operand is stored in memory.                                          |
| Immediate                          | `MOV R0, #15`                              | The operand is a constant value.                                          |
|                                    | `ADD R1, R2, #12`                          | Adds a constant value to the content of a register.                       |
| Register indirect                  | `LDR R0, [R1]`                             | The operand is a memory address stored in a register.                     |
| Register indirect with offset      | `LDR R0, [R1, #4]`                         | The operand is a memory address in a register with an offset.             |
| Register indirect pre-incrementing | `LDR R0, [R1, #4]!`                        | The operand is a memory address in a register, pre-incremented by an offset. |
| Register indirect post-increment   | `LDR R0, [R1], #4`                         | The operand is a memory address in a register, post-incremented by an offset. |
| Register indexed                   | `LDR R0, [R1, R2]`                         | The operand is a memory address, indexed by another register value.       |
| Register indirect indexed with scaling | `LDR R0, [R1, R2, LSL #2]`               | The operand is indexed by a register value, with scaling applied to the index. |
| Program counter relative           | `LDR R0, [PC, #offset]`                    | The operand is an address relative to the current program counter (PC), with an offset. |

1. Register Indirect Addressing with an Offset:
    - The effective address is computed by adding the content of a register and a literal offset coded into the instruction.
    - Example: `LDR R0, [R1, #20]` computes the effective address as R1 + 20 and loads the value from that address into R0.
2. ARM's Autoindexing Pre-indexed Addressing Mode:
    - The base register holds the starting address, and an offset is added to compute the effective address. The pointer register is then updated.
    - Example: `LDR R0, [R1, #4]!` loads the value from R1 + 4 into R0, then updates R1 by adding 4.
3. ARM's Autoindexing Post-indexing Addressing Mode:

    - First, the operand is accessed using the base register, and afterward, the base register is incremented by an offset.
    - Example: `LDR R0, [R1], #4` loads the value from R1 into R0, then increments R1 by 4.
4. Program Counter Relative (PC Relative) Addressing Mode:
    - The operand is accessed relative to the current program counter (R15 or PC). The offset is added to the program counter to compute the effective address.
    - Example: `LDR R0, [R15, #24]` computes the effective address as R15 + 24 and loads the value from that address into R0.

## Examples

### Example 1:  Using Post-indexing Mode

```asm
    PRESERVE8
    THUMB
    AREA  RESET, DATA, READONLY
    EXPORT __Vectors
__Vectors
    DCD 0x20001000 
    DCD Reset_Handler
    ALIGN
SUMP    DCD SUM 
N  DCD 5 
NUM1  DCD 3, -7, 2, -2, 10 
POINTER DCD NUM1 
 AREA    MYRAM, DATA, READWRITE 
SUM  DCD 0 

    AREA MYCODE, CODE, READONLY
    ENTRY
    EXPORT Reset_Handler
Reset_Handler
    LDR R1, N           ; load size of array -  
                        ; a counter for how many elements are left to process 
   LDR R2, POINTER      ; load base pointer of array 
   MOV R0, #0           ; initialize accumulator 
LOOP   
    LDR R3, [R2], #4    ; load value from array,  
                        ; increment array pointer to next word 
    ADD R0, R0, R3      ; add value from array to accumulator 
    SUBS R1, R1, #1     ; decrement work counter 
    BGT LOOP            ; keep looping until counter is zero 
    LDR R4, SUMP        ; get memory address to store sum 
    STR R0, [R4]        ; store answer 
    LDR R6, [R4]        ; Check the value in the SUM
STOP
    B STOP
    END        

```

### Example 2:  Using Post-indexing Mode

```asm
        PRESERVE8
        THUMB
        AREA  RESET, DATA, READONLY
        EXPORT __Vectors
__Vectors
        DCD 0x20001000 
        DCD Reset_Handler
        ALIGN
string1 DCB "Hello world!",0
    AREA    MYRAM, DATA, READWRITE 
SUM  DCD 0 

        AREA MYCODE, CODE, READONLY
        ENTRY
        EXPORT Reset_Handler
Reset_Handler
    LDR   R0, = string1  ; Load the address of string1 into the register R0  
    MOV R1, #0           ; Initialize the counter counting the length of string1 

loopCount 
    LDRB R2, [R0], #1    ; Load the character from the address R0 contains 
                            ; and update the pointer R0   
                            ; using Post-indexed addressing mode 
    CBZ R2, countDone    ; If it is zero...remember null terminated...  
                            ; You are done with the string. The length is in R1. 
    ADD R1, #1;          ; increment the counter for length 
    B loopCount 
    
    
countDone 
    B countDone 
    END        
```

## Lab Work

### Program 1: Greater than 5 Sum

Complete the following code to add up all the numbers that are greater than 5 in the number array NUM1.

```asm
    PRESERVE8
    THUMB
    AREA  RESET, DATA, READONLY
    EXPORT __Vectors
__Vectors
    DCD 0x20001000 
    DCD Reset_Handler
    ALIGN
N DCD 7
NUM1  DCD 3, -7, 2, -2, 10, 20, 30 
POINTER DCD NUM1 
SUMP    DCD SUM 
    AREA MYDATA, DATA, READWRITE
SUM  DCD 0
    AREA MYCODE, CODE, READONLY
    ENTRY
    EXPORT Reset_Handler
Reset_Handler

; Your code goes here

STOP
    B STOP
    END        
```

### Program 2: Find Min and Max

Complete the following code to find the minimum and maximum values in the number array NUM1.

```asm
    PRESERVE8
    THUMB
    AREA  RESET, DATA, READONLY
    EXPORT __Vectors
__Vectors
    DCD 0x20001000 
    DCD Reset_Handler
    ALIGN
N   DCD 12 
NUM1  DCD 3, -7, 2, -2, 10, 20, 30, 15, 32, 8, 64, 66  
POINTER DCD NUM1 
MINP   DCD MIN  
MAXP   DCD MAX
    AREA MYDATA, DATA, READWRITE
MIN DCD 0
MAX DCD 0
    AREA MYCODE, CODE, READONLY
    ENTRY
    EXPORT Reset_Handler
Reset_Handler

; Your code goes here


STOP
    B STOP
    END        
```
