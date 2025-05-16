---
outline: deep
---
# ARM Flow Control Instructions <Badge type="tip" text="Experiment 3" />

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
string1 DCB "Hello world!",0
    AREA MYCODE, CODE, READONLY
    ENTRY
    EXPORT Reset_Handler
Reset_Handler
    LDR R0, = string1 ; Load the address of string1 into the register R0
    MOV R1, #0 ; Initialize the counter counting the length of string1
loopCount
    LDRB R2, [R0] ; Load the character from the address R0 contains
    CMP R2, #0
    BEQ countDone   ; If it is zero...remember null terminated...
                    ; You are done with the string. The length is in R1.
    ADD R0, #1 ; Otherwise, increment index to the next character
    ADD R1, #1 ; increment the counter for length
    B loopCount
countDone
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
N DCD 5
    AREA MYDATA, DATA, READWRITE
SUM DCD 0
    AREA MYCODE, CODE, READONLY
    ENTRY
    EXPORT Reset_Handler
Reset_Handler
    LDR R1, N ;Load count into R1
    MOV R0, #0 ;Clear accumulator R0
LOOP
    ADD R0, R0, R1 ;Add number into R0
    SUBS R1, R1, #1 ;Decrement loop counter R1
    BGT LOOP ;Branch back if not done
    LDR R3, SUMP ;Load address of SUM to R3
    STR R0, [R3] ;Store SUM
    LDR R4, [R3]
STOP
    B STOP
    END
```

## Lab Work

Write an ARM Assembly progrom to count the number of vowels/non-vowels in a string.

> **Note:** The string should be stored in the memory and the program should read the string from the memory.
> "ARM Assembly language", 0

### Criteria

1. Use `R1` to hold the address of the string.
2. Use `R2` to be the counter for the number of vowels.
3. Use `R3` to be the counter for the number of non-vowels.
