    PRESERVE8 
    THUMB 
; Constants
SYSCTL_RCGCGPIO_R   EQU 0x400FE608 
GPIO_PORTF_DIR_R    EQU 0x40025400 
GPIO_PORTF_DEN_R    EQU 0x4002551C 
GPIO_PORTF_DATA_R   EQU 0x400253FC 
DELAY               EQU 20000000  
    AREA |.text| , CODE, READONLY, ALIGN=2 
    ENTRY ; ENTRY marks the starting point of the code execution 
    EXPORT __main 
__main

    LDR R1 , =SYSCTL_RCGCGPIO_R 
    LDR R0 , [R1] 
    ORR R0 ,R0, #0x20 
    STR R0 , [R1] 
    NOP ; No operations for 3 cycles 
    NOP 
    NOP 
; set the direction for port f
    LDR R1 , =GPIO_PORTF_DIR_R 
    LDR R0 , [R1] 
    ORR R0 , #0x08 
    STR R0 , [R1] 

; Digital enable for PORT F 
    LDR R1 , =GPIO_PORTF_DEN_R 
    LDR R0 , [R1] 
    ORR R0 , #0x08 
    STR R0 , [R1] 

    ; Infinite loop LED flash 
LED_flash 
; Set the data for PORT F to turn LED on 
    LDR R1 , =GPIO_PORTF_DATA_R 
    LDR R0 , [R1] 
    ORR R0 , R0 , #0x08 
    STR R0 , [R1] 
  
; Delay loop 
    LDR R5 , =DELAY 
delay1 
    SUBS R5,#1 
    BNE delay1 

; Set the data for PORT F to turn LED off 
    LDR R1 , =GPIO_PORTF_DATA_R 
    LDR R0 , [R1] 
    AND R0 , R0 , #0xF7 
    STR R0 , [R1] 
    ; Delay loop 
    LDR R5 , =DELAY 
delay2 
    SUBS R5,#1 
    BNE delay2
        
    B LED_flash 
    ALIGN 
    END ; End of the program , matched with ENTRY keyword