
        AREA    RESET, CODE, READONLY
        EXPORT  __Vectors
__Vectors
        DCD     0x20001000            ; Initial SP (example top-of-stack)
        DCD     Reset_Handler         ; Reset vector
        AREA    MYCODE, CODE, READONLY
        ENTRY
        EXPORT  Reset_Handler
Reset_Handler

; ========== Part A Barrel shifter ============
        MOV     R3, #3
        ADD     R4, R3, R3, LSL #2    ; R4 = 3 + (3 << 2) = 15
                                      ; R4 = ?  (confirm)  FLAGS? (unchanged)
        ADD     R4, R3, R3, LSR #1    ; R4 = 3 + (3 >> 1) = 4
                                      ; R4 = ?  (confirm)  FLAGS? (unchanged)
; ========== Part E: Conditional execution  ===========
        ; Compare and update either R6 or R7 based on Z flag
        MOV     R6, #0
        MOV     R7, #0

        MOV     R0, #0                ; try #0 first, then change to #5 and rebuild
        CMP     R0, #0
        ADDEQ   R6, R6, #1            ; if Z==1: R6++
        ADDNE   R7, R7, #1            ; if Z==0: R7++
        ; After running once, edit "MOV R0, #0" -> "MOV R0, #5", rebuild, run again.


STOP    B       STOP                   ; infinite loop

        END
