        AREA    RESET, CODE, READONLY
        EXPORT  __Vectors
__Vectors
        DCD     0x20001000          ; Initial SP
        DCD     Reset_Handler       ; Reset vector
        ALIGN
        AREA    MYCODE, CODE, READONLY
        ENTRY
        EXPORT  Reset_Handler
UIntToStr                           ; UIntToStr(R0 = value, R1 = buf) -> R0 = digit count
        PUSH    {R4, LR}            ; save callee-saved R4 and the return address
        MOV     R2, #0              ; R2 = digit count
        MOV     R3, #10             ; R3 = divisor
its_div
        UDIV    R4, R0, R3          ; R4 = value / 10
        MLS     R12, R4, R3, R0     ; R12 = value - (value / 10) * 10 = value % 10
        ADD     R12, R12, #'0'      ; digit -> ASCII character
        PUSH    {R12}               ; digits are produced least-significant first...
        ADD     R2, R2, #1          ; count++
        MOV     R0, R4              ; value = value / 10
        CMP     R0, #0
        BNE     its_div             ; repeat until value is 0 (value 0 still gives "0")
        MOV     R0, R2              ; return value = digit count
its_pop
        POP     {R12}               ; ...so popping returns them most-significant first
        STRB    R12, [R1], #1       ; *buf++ = character
        SUBS    R2, R2, #1          ; count--
        BNE     its_pop
        MOV     R12, #0
        STRB    R12, [R1]           ; null terminator
        POP     {R4, PC}            ; restore R4 and return (SP is back where it started)
Reset_Handler
        LDR     R0, =40213          ; value to convert
        LDR     R1, =NUMSTR         ; destination buffer in RAM
        BL      UIntToStr           ; R0 = 5, NUMSTR = "40213"
STOP    B       STOP
        AREA    MYDATA, DATA, READWRITE
NUMSTR  SPACE   12                  ; up to 10 digits + null (written before it is read)
        END
