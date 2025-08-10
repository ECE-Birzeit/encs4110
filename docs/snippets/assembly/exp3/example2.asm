 AREA RESET, CODE, READONLY
    EXPORT __Vectors
__Vectors
    DCD 0x20001000             ; Initial Stack Pointer value
    DCD Reset_Handler          ; Reset handler address
    ALIGN

ARR     DCD 23, 25, 9, 11, 6, 90  ; Array of 6 integers
LEN     DCB 6                     ; Length of the array (6 elements)
N       DCB 3                     ; Modulus value (3)

    AREA MYDATA, DATA, READWRITE
RESULT  DCD 0                     ; Variable to store the final result (sum)

    AREA MYCODE, CODE, READONLY
    ENTRY
    EXPORT Reset_Handler

Reset_Handler
    LDR R0, =ARR                  ; R0 = address of array
    LDRB R1, LEN                  ; R1 = length of array
    LDRB R2, N                    ; R2 = modulus N

    BL SUM_MOD_N                  ; Call procedure: sum modulo elements

    LDR R3, =RESULT               ; R3 = address of RESULT
    STR R0, [R3]                  ; Store sum result in RESULT

    ; Infinite loop to halt here
STOP
    B STOP                        ; Loop forever

;-----------------------------------------------
; Procedure: SUM_MOD_N
; Sum elements of an integer array after applying modulo N to each element.
; Inputs:
;   R0 - pointer to start of array
;   R1 - number of elements
;   R2 - modulus value N
; Output:
;   R0 - sum of (array[i] % N)
;-----------------------------------------------
SUM_MOD_N
    PUSH {R4, R5, R6, LR}   ; Save registers
    MOV R4, #0              ; accumulator
    MOV R5, R0              ; pointer to array
    MOV R6, R1              ; save LEN

SUM_LOOP
    CBZ R6, SUM_MOD_N_EXIT
    LDR R3, [R5], #4        ; load element

    MOV R0, R3              ; dividend
    MOV R1, R2              ; divisor
    BL MOD_N

    ADD R4, R4, R0
    SUBS R6, R6, #1         ; decrement loop counter
    BNE SUM_LOOP

SUM_MOD_N_EXIT
    MOV R0, R4
    POP {R4, R5, R6, PC}

;-----------------------------------------------
; Procedure: MOD_N
; Compute remainder R0 = R0 % R1 (modulus operation).
; Inputs:
;   R0 - dividend
;   R1 - divisor
; Output:
;   R0 - remainder
;-----------------------------------------------
MOD_N
    PUSH {R4, LR}                ; Save callee-saved registers and LR

    UDIV R4, R0, R2             ; R4 = quotient = dividend / divisor
    MUL R4, R4, R2              ; R4 = quotient * divisor
    SUB R0, R0, R4              ; R0 = dividend - quotient * divisor (remainder)

    POP {R4, PC}                ; Restore R4 and return (pop PC = pop LR)

    END