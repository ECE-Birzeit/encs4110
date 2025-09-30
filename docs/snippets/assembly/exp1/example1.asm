    AREA  RESET, CODE, READONLY
    EXPORT __Vectors
__Vectors
    DCD 0x20001000
    DCD Reset_Handler
    ALIGN

; Data section
NUM1    DCD 50              ; First integer
NUM2    DCD 12              ; Second integer
RP      DCD RESULT          ; Pointer to RESULT variable

    AREA MYDATA, DATA, READWRITE
RESULT  DCD 0               ; Will hold the final computed value

    AREA MYCODE, CODE, READONLY
    ENTRY
    EXPORT Reset_Handler

Reset_Handler
    ; Load values from memory into registers
    LDR R1, NUM1            ; R1 = 50
    LDR R2, NUM2            ; R2 = 12

    ; Perform arithmetic
    ADD R3, R1, R2          ; R3 = 50 + 12 = 62
    SUB R3, R3, #4          ; R3 = 62 - 4 = 58
    MUL R4, R3, R2          ; R4 = 58 × 12 = 696

    ; Logical operations
    AND R5, R4, #0xFF       ; R5 = 696 & 0xFF = 0xB8 (184)
    ORR R5, R5, #0x01       ; R5 = 0xB8 | 0x01 = 0xB9 (185)
    BIC R5, R5, #0x08       ; R5 = 0xB9 & ~0x08 = 0xB1 (177)
    EOR R5, R5, #0x02       ; R5 = 0xB1 ^ 0x02 = 0xB3 (179)

    ; Store result in memory using a pointer
    LDR R6, RP              ; R6 = address of RESULT
    STR R5, [R6]            ; RESULT = R5

    ; Read back for verification
    LDR R7, [R6]            ; R7 = RESULT
STOP    B STOP

    END