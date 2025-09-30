    AREA RESET, CODE, READONLY
    EXPORT __Vectors
__Vectors
    DCD 0x20001000
    DCD Reset_Handler
    ALIGN

N       DCD 5           ; Sum numbers 1 to 5
    AREA MYDATA, DATA, READWRITE
RESULT  DCD 0           ; Store sum result

    AREA MYCODE, CODE, READONLY
    ENTRY
    EXPORT Reset_Handler

; Procedure to compute sum 1+2+...+N (input in R0), returns sum in R0
SumUp PROC
    MOV R1, #0          ; R1 = accumulator
LoopSum
    ADD R1, R1, R0      ; Add current number
    SUBS R0, R0, #1     ; Decrement counter
    BGT LoopSum         ; Continue if R0 > 0
    MOV R0, R1          ; Move result to R0
    BX LR               ; Return
ENDP

Reset_Handler
    LDR R0, N
    BL SumUp            ; Call sum procedure

    LDR R1, =RESULT
    STR R0, [R1]        ; Store result (should be 15)
	LDR R2, [R1]
STOP B STOP
	END