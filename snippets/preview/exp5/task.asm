; Experiment 5 Task: a light show read from a table in memory.
; Each step of the show is two bytes: a colour, then how long to show it in tenths of a second.
; Provided: the names, PF_Init, SET_LEDS, WAIT_100MS, and a sample table.
; Write the three TODOs (highlighted).

; Register addresses
RCGCGPIO    EQU     0x400FE608          ; Clock gating for the GPIO ports
GPIOF_DIR   EQU     0x40025400          ; Port F direction
GPIOF_DEN   EQU     0x4002551C          ; Port F digital enable
GPIOF_RGB   EQU     0x40025038          ; Port F DATA, masked to PF1-PF3

; Bits and colours
PORTF       EQU     (1 << 5)            ; RCGCGPIO bit for Port F
RED         EQU     (1 << 1)            ; PF1
BLUE        EQU     (1 << 2)            ; PF2
GREEN       EQU     (1 << 3)            ; PF3
RGB_PINS    EQU     (RED + BLUE + GREEN)
YELLOW      EQU     (RED + GREEN)
CYAN        EQU     (GREEN + BLUE)
MAGENTA     EQU     (RED + BLUE)
WHITE       EQU     (RED + GREEN + BLUE)
OFF         EQU     0

END_MARK    EQU     0xFF                ; A colour byte of 0xFF ends the table

WAIT_COUNT  EQU     1000000             ; TODO 1: change this so WAIT_100MS takes 100 ms ; [!code highlight]

            AREA    MYCODE, CODE, READONLY
            THUMB
            EXPORT  __main              ; The startup file jumps here

; ---------------------------------------------------------------- provided: don't change
; PF_Init: clock to Port F, and PF1-PF3 as digital outputs
PF_Init     PROC
            LDR     R1, =RCGCGPIO
            LDR     R0, [R1]
            ORR     R0, R0, #PORTF
            STR     R0, [R1]
            NOP                         ; Give the clock time to start
            NOP
            NOP
            LDR     R1, =GPIOF_DIR
            LDR     R0, [R1]
            ORR     R0, R0, #RGB_PINS
            STR     R0, [R1]
            LDR     R1, =GPIOF_DEN
            LDR     R0, [R1]
            ORR     R0, R0, #RGB_PINS
            STR     R0, [R1]
            BX      LR
            ENDP

; SET_LEDS: light the colour in R0. Changes R1 only.
SET_LEDS    PROC
            LDR     R1, =GPIOF_RGB
            STR     R0, [R1]            ; Only PF1-PF3 change: the address is masked
            BX      LR
            ENDP

; DELAY: count R0 down to zero, about 3 clock cycles per count
DELAY       PROC
            SUBS    R0, R0, #1
            BNE     DELAY
            BX      LR
            ENDP

; WAIT_100MS: wait about 100 ms (once WAIT_COUNT is right). Changes R0 only.
WAIT_100MS  PROC
            PUSH    {R4, LR}            ; It calls DELAY, so save the return address
            LDR     R0, =WAIT_COUNT
            BL      DELAY
            POP     {R4, PC}
            ENDP
; ---------------------------------------------------------------- end of provided part

; The show, forever. R5 and R6 are yours: the subroutines above never change them.
__main      PROC
            BL      PF_Init
restart     LDR     R5, =TABLE          ; R5 points at the first step of the table
next
            ; [!code highlight:3]
            ; TODO 2: play one step. Read the colour (LDRB R0, [R5], #1); if it's END_MARK,
            ;         go back to restart. Read the duration (LDRB R6, [R5], #1), show the
            ;         colour with SET_LEDS, and call WAIT_100MS R6 times. Then go to next.
            B       next
            ENDP

; The show: one step per line, a colour then tenths of a second
            AREA    SHOWDATA, DATA, READONLY
TABLE       DCB     RED, 10             ; Red for 1 second
            DCB     GREEN, 10           ; Green for 1 second
            DCB     BLUE, 10            ; Blue for 1 second
            ; [!code highlight]
            ; TODO 3: replace the three steps above with your own show
            DCB     END_MARK            ; The end: back to the first step

            END
