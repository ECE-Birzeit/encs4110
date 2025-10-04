        AREA    RESET, CODE, READONLY
        EXPORT  __Vectors
__Vectors
        DCD     0x20001000            ; Initial SP (example top-of-stack)
        DCD     Reset_Handler         ; Reset vector
        AREA    MYCODE, CODE, READONLY
        ENTRY
        EXPORT  Reset_Handler
Reset_Handler
; ========== Part A: Moving immediates (MOV, MOVW/MOVT, MOV32, LDR =) =========
        ; 1) Simple 8/12-bit immediate
        MOV     R2, #0x01             ; R2 = ?   (after step)
        ; 2) 16-bit low half into R5
        MOV     R5, #0x3210           ; R5 = ?   (low 16 bits set)
        ; 3) High half into R5: combine with (2)
        MOVT    R5, #0x7654           ; R5 = ?   (now 0x76543210)
        ; 4) 32-bit immediate with MOV32 macro (emits MOVW+MOVT)
        MOV32   R6, #0x87654321       ; R6 = ?
        ; 5) Literal load of a 32-bit immediate
        LDR     R7, =0x87654321       ; R7 = ?
; ========== Part B: ADD/SUB without and with flag updates =========
        MOV     R2, #0x02             ; R2 = ?
        MOV     R3, #0x03             ; R3 = ?
        ADD     R1, R2, R3            ; R1 = ?   (flags unchanged)
        ; Now set R3 to all ones, then add with flags
        MOV32   R3, #0xFFFFFFFF       ; R3 = ?
        ADDS    R1, R2, R3            ; R1 = ?   FLAGS? (N,Z,C,V)
        SUBS    R1, R2, R3            ; R1 = ?   FLAGS? (N,Z,C,V)
        ; Same add but without S (no flag update)
        MOV     R4, #0xFF             ; small value for contrast
        ADD     R1, R2, R4            ; R1 = ?   FLAGS? (should be unchanged)
        ; Now with S, so flags DO update
        ADDS    R1, R2, R4            ; R1 = ?   FLAGS? (N,Z,C,V)
; ========== Part C: Overflow / Zero / Negative flag demos =========
        ; Create an ADD overflow with two large positive numbers
        MOV32   R2, #0x7FFFFFFF       ; R2 = ?
        MOV32   R3, #0x7FFFFFFF       ; R3 = ?
        ADDS    R1, R2, R3            ; R1 = ?   Overflow expected? FLAGS?
        ; Create a ZERO result
        MOV     R2, #1
        SUBS    R1, R2, #1            ; R1 = ? (=0)  FLAGS? (Z should be 1)
        ; Create a NEGATIVE result
        SUBS    R1, R2, #2            ; R1 = ? (negative) FLAGS? (N should be 1)
STOP    B       STOP                   ; infinite loop
        END
