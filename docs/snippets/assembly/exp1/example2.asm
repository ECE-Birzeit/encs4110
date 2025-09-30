    AREA  RESET, CODE, READONLY
    EXPORT __Vectors
__Vectors
    DCD 0x20001000
    DCD Reset_Handler
    ALIGN

    AREA MYCODE, CODE, READONLY
    ENTRY
    EXPORT Reset_Handler

Reset_Handler
    ; Set up registers
    MOVS R0, #10         ; R0 = 10, updates flags (N=0, Z=0)
    MOVS R1, #10         ; R1 = 10, updates flags

    ; Compare R0 and R1 using SUBS (R0 - R1)
    SUBS R2, R0, R1      ; R2 = 10 - 10 = 0
    ; Flags after SUBS:
    ; Z=1 (result zero), N=0, C=1 (no borrow), V=0

    ; Compare with immediate using TST (bitwise AND, updates flags)
    MOV R3, #0x0F        ; R3 = 0x0F (binary 00001111)
    TST R3, #0x08        ; Test bit 3
    ; Flags:
    ; Z=0 (bit 3 is set), N=0

    TST R3, #0x10        ; Test bit 4
    ; Flags:
    ; Z=1 (bit 4 not set), N=0

    ; Test equivalence using TEQ (bitwise XOR, updates flags)
    MOV R4, #0x55        ; 0x55 = 01010101b
    MOV R5, #0x55        ; same value
    TEQ R4, R5           ; R4 XOR R5 = 0
    ; Flags:
    ; Z=1 (equal), N=0

    MOV R6, #0x33        ; 0x33 = 00110011b
    TEQ R4, R6           ; 0x55 XOR 0x33 != 0
    ; Flags:
    ; Z=0, N=0

    ; Negative result example with ADDS
    MOVS R7, #5          ; R7 = 5
    SUBS R7, R7, #10     ; R7 = 5 - 10 = -5 (two's complement)
    ; Flags:
    ; N=1 (negative), Z=0, C=0, V=0
STOP    B STOP
    END