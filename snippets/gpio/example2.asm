; Example 2: cycle the RGB LED through red, blue, green
RCGCGPIO    EQU     0x400FE608          ; Clock gating for the GPIO ports
GPIOF_DIR   EQU     0x40025400          ; Port F direction
GPIOF_DEN   EQU     0x4002551C          ; Port F digital enable
GPIOF_RGB   EQU     0x40025038          ; Port F DATA, masked to PF1-PF3: 0x40025000 + (0x0E << 2)

PORTF       EQU     (1 << 5)            ; RCGCGPIO bit for Port F
RED         EQU     (1 << 1)            ; PF1
BLUE        EQU     (1 << 2)            ; PF2
GREEN       EQU     (1 << 3)            ; PF3
RGB_PINS    EQU     (RED + BLUE + GREEN)
DELAY_COUNT EQU     10000000            ; About 0.6 s at 50 MHz

        AREA    RESET, CODE, READONLY    ; Code section
        THUMB                            ; Use Thumb instruction set
        EXPORT  __main                   ; Export symbol
__main  PROC
        BL      PF_Init                  ; Initialize Port F
        LDR     R1, =GPIOF_RGB           ; Writes here change PF1-PF3 only
loop
        MOV     R0, #RED                 ; Red
        STR     R0, [R1]
        LDR     R0, =DELAY_COUNT
        BL      DELAY

        MOV     R0, #BLUE                ; Blue
        STR     R0, [R1]
        LDR     R0, =DELAY_COUNT
        BL      DELAY

        MOV     R0, #GREEN               ; Green
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

PF_Init PROC
        LDR     R1, =RCGCGPIO
        LDR     R0, [R1]
        ORR     R0, R0, #PORTF           ; Enable Port F clock
        STR     R0, [R1]
        NOP                              ; Give the clock time to start
        NOP
        LDR     R1, =GPIOF_DIR
        LDR     R0, [R1]
        ORR     R0, R0, #RGB_PINS        ; PF1-PF3 as outputs
        STR     R0, [R1]

        LDR     R1, =GPIOF_DEN
        LDR     R0, [R1]
        ORR     R0, R0, #RGB_PINS        ; PF1-PF3 digital
        STR     R0, [R1]
        BX      LR                       ; Return
        ENDP
        END
