
    AREA RESET, CODE, READONLY
    EXPORT __Vectors
__Vectors
    DCD 0x20001000
    DCD Reset_Handler
    ALIGN
    AREA M_DATA, DATA, READONLY
N      DCD 5
ARRAY  DCD 3, -7, 2, -2, 10
PTR_A  DCD ARRAY

string1 DCB "Hello, ARM!", 0
PTR_S   DCD string1

    AREA MYCODE, CODE, READONLY
    ENTRY
    EXPORT Reset_Handler
Reset_Handler

; ---- Array loop using length counter ----
    LDR R1, N        ; Number of elements
    LDR R2, PTR_A    ; Pointer to array
    MOV R0, #0       ; Sum accumulator

arrayLoop
    LDR R3, [R2], #4 ; Load next element, post-increment pointer
    ADD R0, R0, R3
    SUBS R1, R1, #1
    BGT arrayLoop

; ---- String loop using null terminator ----
    LDR R4, PTR_S    ; Pointer to string
    MOV R5, #0       ; Character counter

stringLoop
    LDRB R6, [R4], #1 ; Load next char
    CBZ R6, stringDone
    ADD R5, R5, #1
    B stringLoop

stringDone
STOP B STOP
    END