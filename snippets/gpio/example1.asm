; Example 1: blink the red LED (PF1)
RCGCGPIO    EQU     0x400FE608          ; Clock gating for the GPIO ports
GPIOF_DIR   EQU     0x40025400          ; Port F direction
GPIOF_DEN   EQU     0x4002551C          ; Port F digital enable
GPIOF_PF1   EQU     0x40025008          ; Port F DATA, masked to PF1: 0x40025000 + (0x02 << 2)

PORTF       EQU     (1 << 5)            ; RCGCGPIO bit for Port F
RED         EQU     (1 << 1)            ; PF1
DELAY_COUNT EQU     1000000             ; About 60 ms at 50 MHz

        AREA    RESET, CODE, READONLY    ; Code section
        THUMB                            ; Use Thumb instruction set
        EXPORT  __main                   ; Export symbol

__main  PROC
        LDR     R1, =RCGCGPIO
        LDR     R0, [R1]                 ; Read current value
        ORR     R0, R0, #PORTF           ; Enable Port F clock
        STR     R0, [R1]                 ; Write back
        NOP                              ; Give the clock time to start
        NOP

        LDR     R1, =GPIOF_DIR
        LDR     R0, [R1]
        ORR     R0, R0, #RED             ; PF1 as output
        STR     R0, [R1]

        LDR     R1, =GPIOF_DEN
        LDR     R0, [R1]
        ORR     R0, R0, #RED             ; PF1 digital
        STR     R0, [R1]

        LDR     R1, =GPIOF_PF1           ; Writes here change PF1 only
loop
        MOV     R0, #RED                 ; Red LED on
        STR     R0, [R1]
        LDR     R0, =DELAY_COUNT
        BL      DELAY

        MOV     R0, #0                   ; Red LED off
        STR     R0, [R1]
        LDR     R0, =DELAY_COUNT
        BL      DELAY

        B       loop                     ; Repeat
        ENDP

DELAY   PROC
        SUBS    R0, R0, #1               ; Decrement counter
        BNE     DELAY                    ; Loop until zero
        BX      LR                       ; Return
        ENDP
        END
