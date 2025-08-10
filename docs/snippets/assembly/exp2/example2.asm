 AREA RESET, CODE, READONLY
    EXPORT __Vectors
__Vectors
    DCD 0x20001000          
    DCD Reset_Handler
    ALIGN
string  DCB "Arm Assembly!", 0


    AREA M_DATA, DATA, READWRITE
conv_str SPACE 13           ; Buffer for converted string (same length as input)

    AREA MYCODE, CODE, READONLY
    ENTRY
    EXPORT Reset_Handler
Reset_Handler

    LDR R0, =string         ; Pointer to input string
    LDR R1, =conv_str       ; Pointer to output buffer

caseConvLoop
    LDRB R2, [R0], #1       ; Load byte from input and post-increment R0
    CBZ R2, done            ; If zero (end of string), jump to done

    CMP R2, #'a'            ; Compare char to 'a'
    BLT storeChar           ; If less, skip conversion, just store

    CMP R2, #'z'            ; Compare char to 'z'
    BGT storeChar           ; If greater, skip conversion, just store

    BIC R2, R2, #32         ; Convert to uppercase by clearing bit 5

storeChar
    STRB R2, [R1], #1       ; Store char to output buffer and post-increment
    B caseConvLoop          ; Loop back

done
    MOV R2, #0
    STRB R2, [R1]           ; Null-terminate the output string

STOP
    B STOP                  ; Infinite loop

    END