        AREA    RESET, CODE, READONLY
        EXPORT  __Vectors
__Vectors
        DCD     0x20001000            ; Initial SP (example)
        DCD     Reset_Handler         ; Reset vector
        AREA    MYCODE, CODE, READONLY
        ENTRY
        EXPORT  Reset_Handler
Reset_Handler
        ; Base address of the array
        LDR     R0, =ARRAY            ; R0 = &ARRAY
        ; ---------------------------------------------------------
        ; 1) Immediate Offset: EA = R0 + #8  (third element)
        ; ---------------------------------------------------------
        LDR     R1, [R0, #8]          ; R1 = ARRAY[2] = ? (expect 30)
        LDR     R7, =OUT
        STR     R1, [R7, #0]          ; OUT[0] = R1
        ; ---------------------------------------------------------
        ; 2) Pre-indexed: R2 = [R2 + #4], then R2 = R2 + #4
        ;    Use a scratch pointer so R0 remains the base.
        ; ---------------------------------------------------------
        MOV     R2, R0                 ; R2 = &ARRAY
        LDR     R3, [R2, #4]!          ; R2 -> &ARRAY[1], R3 = ARRAY[1] = ? (expect 20)
        STR     R3, [R7, #4]           ; OUT[1] = R3
        ; After this, R2 now points at ARRAY[1].
        ; ---------------------------------------------------------
        ; 3) Post-indexed: R4 = [R4], then R4 = R4 + #12
        ;    Load first element, then advance pointer to the 4th.
        ; ---------------------------------------------------------
        MOV     R4, R0                 ; R4 = &ARRAY
        LDR     R5, [R4], #12          ; R5 = ARRAY[0] = ? (expect 10), R4 -> &ARRAY[3]
        STR     R5, [R7, #8]           ; OUT[2] = R5
        ; ---------------------------------------------------------
        ; 4) Register Offset: EA = R0 + R6
        ;    Offset register holds byte offset (multiple of 4 for words).
        ; ---------------------------------------------------------
        MOV     R6, #12                ; byte offset to ARRAY[3]
        LDR     R8, [R0, R6]           ; R8 = ARRAY[3] = ? (expect 40)
        STR     R8, [R7, #12]          ; OUT[3] = R8
        ; read second element via register offset
        MOV     R6, #4                 ; byte offset to ARRAY[1]
        LDR     R9, [R0, R6]           ; R9 = ARRAY[1] = ? (expect 20)
STOP    B       STOP
        AREA    CONSTS, DATA, READONLY
ARRAY   DCD     10, 20, 30, 40        ; four words at consecutive addresses
        AREA    MYDATA, DATA, READWRITE
OUT     DCD     0, 0, 0, 0            ; capture buffer for observed loads. Could by replaced by space 16
        END
