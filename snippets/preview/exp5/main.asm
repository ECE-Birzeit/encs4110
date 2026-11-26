; #region full
; #region defines
; Register addresses
RCGCGPIO    EQU     0x400FE608          ; Clock gating for the GPIO ports
GPIOF_DIR   EQU     0x40025400          ; Port F direction
GPIOF_DEN   EQU     0x4002551C          ; Port F digital enable
GPIOF_RGB   EQU     0x40025038          ; Port F DATA, masked to PF1-PF3: 0x40025000 + (0x0E << 2)

; Bits
PORTF       EQU     (1 << 5)            ; RCGCGPIO bit for Port F
RED         EQU     (1 << 1)            ; PF1
BLUE        EQU     (1 << 2)            ; PF2
GREEN       EQU     (1 << 3)            ; PF3
RGB_PINS    EQU     (RED + BLUE + GREEN)

; Mixed colours
YELLOW      EQU     (RED + GREEN)
CYAN        EQU     (GREEN + BLUE)
MAGENTA     EQU     (RED + BLUE)
WHITE       EQU     (RED + GREEN + BLUE)

DELAY_COUNT EQU     5000000             ; About a third of a second at 50 MHz

            AREA    MYCODE, CODE, READONLY
            THUMB
            EXPORT  __main              ; The startup file jumps here
; #endregion defines

; #region functions
; PF_Init: clock to Port F, and PF1-PF3 as digital outputs
PF_Init     PROC
            LDR     R1, =RCGCGPIO
            LDR     R0, [R1]
            ORR     R0, R0, #PORTF      ; Clock to Port F
            STR     R0, [R1]
            NOP                         ; Give the clock time to start
            NOP
            NOP

            LDR     R1, =GPIOF_DIR
            LDR     R0, [R1]
            ORR     R0, R0, #RGB_PINS   ; PF1-PF3 are outputs
            STR     R0, [R1]

            LDR     R1, =GPIOF_DEN
            LDR     R0, [R1]
            ORR     R0, R0, #RGB_PINS   ; PF1-PF3 are digital pins
            STR     R0, [R1]
            BX      LR
            ENDP

; DELAY: count R0 down to zero
DELAY       PROC
            SUBS    R0, R0, #1
            BNE     DELAY
            BX      LR
            ENDP

; SHOW: light the colour in R0, then wait
SHOW        PROC
            PUSH    {R4, LR}            ; SHOW calls DELAY, so save the return address
            LDR     R1, =GPIOF_RGB
            STR     R0, [R1]            ; Only PF1-PF3 change: the address is masked
            LDR     R0, =DELAY_COUNT
            BL      DELAY
            POP     {R4, PC}            ; Return
            ENDP
; #endregion functions

; #region main
__main      PROC
            BL      PF_Init

loop        MOV     R0, #RED
            BL      SHOW
            MOV     R0, #YELLOW
            BL      SHOW
            MOV     R0, #GREEN
            BL      SHOW
            MOV     R0, #CYAN
            BL      SHOW
            MOV     R0, #BLUE
            BL      SHOW
            MOV     R0, #MAGENTA
            BL      SHOW
            MOV     R0, #WHITE
            BL      SHOW
            B       loop
            ENDP
; #endregion main

            END
; #endregion full
