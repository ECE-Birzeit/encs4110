        AREA    RESET, CODE, READONLY    ; Code section
        THUMB                            ; Use Thumb instruction set
        EXPORT  __main                   ; Export symbol

__main  PROC
        LDR     R1, =0x400FE608          ; RCGCGPIO address
        LDR     R0, [R1]                 ; Read current value
        ORR     R0, R0, #0x20            ; Enable Port F clock
        STR     R0, [R1]                 ; Write back
        NOP                              ; Small delay
        NOP

        LDR     R1, =0x40025400          ; GPIODIR address
        LDR     R0, [R1]                 ; Read current value
        ORR     R0, R0, #0x02            ; Set PF1 as output
        STR     R0, [R1]                 ; Write back

        LDR     R1, =0x4002551C          ; GPIODEN address
        LDR     R0, [R1]                 ; Read current value
        ORR     R0, R0, #0x02            ; Enable digital function for PF1
        STR     R0, [R1]                 ; Write back

loop    LDR     R1, =0x40025008          ; Masked DATA address for PF1
        MOV     R0, #0x02                ; Turn ON red LED (PF1)
        STR     R0, [R1]                 ; Write to GPIODATA

        LDR     R0, =1000000             ; Load delay count
        BL      DELAY                    ; Call delay

        MOV     R0, #0x00                ; Turn OFF red LED (PF1)
        STR     R0, [R1]                 ; Write to GPIODATA

        LDR     R0, =1000000             ; Load delay count
        BL      DELAY                    ; Call delay

        B       loop                     ; Repeat
        ENDP

DELAY   PROC
        SUBS    R0, R0, #1               ; Decrement counter
        BNE     DELAY                    ; Loop until zero
        BX      LR                       ; Return
        ENDP
        END
