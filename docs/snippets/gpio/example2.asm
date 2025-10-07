        AREA    RESET, CODE, READONLY    ; Code section
        THUMB                            ; Use Thumb instruction set
        EXPORT  __main                   ; Export symbol
__main  PROC
        BL      PF_Init                  ; Initialize Port F
loop    
        LDR     R1, =0x40025038          ; Masked DATA address for PF1-PF3
        
        MOV     R0, #0x02                ; Turn ON red LED (PF1)
        STR     R0, [R1]                 ; Write to GPIODATA	
        LDR     R0, =10000000            ; Load delay count
        BL      DELAY                    ; Call delay

        MOV     R0, #0x04                ; Turn ON blue LED (PF2)
        STR     R0, [R1]
        LDR     R0, =10000000
        BL      DELAY

        MOV     R0, #0x08                ; Turn ON green LED (PF3)
        STR     R0, [R1]
        LDR     R0, =10000000
        BL      DELAY
        B       loop                     ; Repeat
        ENDP

DELAY   PROC
        SUBS    R0, R0, #1               ; Decrement counter
        BNE     DELAY                    ; Loop until zero
        BX      LR                       ; Return
        ENDP

PF_Init PROC
        LDR     R1, =0x400FE608          ; RCGCGPIO address
        LDR     R0, [R1]                 ; Read current value
        ORR     R0, R0, #0x20            ; Enable Port F clock
        STR     R0, [R1]
        NOP                              ; Small delay
        NOP
        LDR     R1, =0x40025400          ; GPIODIR address
        LDR     R0, [R1]
        ORR     R0, R0, #0x0E            ; Set PF1-PF3 as outputs
        STR     R0, [R1]

        LDR     R1, =0x4002551C          ; GPIODEN address
        LDR     R0, [R1]
        ORR     R0, R0, #0x0E            ; Enable digital function for PF1-PF3
        STR     R0, [R1]
        BX      LR                       ; Return
        ENDP
        END
