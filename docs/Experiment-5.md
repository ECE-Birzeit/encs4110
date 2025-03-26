---
outline: deep
lastUpdated: true
---

# ARM Subroutine/ Procedure/ Function Calls

- ARM processors do not provide a fully automatic subroutine call/return mechanism like other processors.
- ARM's branch and link instruction, `BL`, automatically saves the return address in the register `R14` (Link Register, LR).
- At the end of the subroutine, you can use `MOV PC, LR` to return to the instruction after the subroutine call. 
- The subroutine is called using `BL SUBROUTINE_NAME`.
- `SUBROUTINE_NAME` is a label in the ARM program.
  
## Stack

- ARM stacks are flexible due to software implementation.
- The stack pointer is a register pointing to the top of the stack.
- Any general-purpose register can be used as a stack pointer in ARM.
- There are typically two types of stacks based on the growth direction.
  - Ascending Stack: The stack pointer increases as items are pushed, growing towards higher addresses.
  - Descending Stack: The stack pointer decreases as items are pushed, growing towards lower addresses.
- Stacks can also be categorized based on what the stack pointer points to:
  - Empty Stack: The stack pointer points to the location for the next item.
  - Full Stack: The stack pointer points to the location of the last item.

So there is 4 types of stacks based on the growth direction and what the stack pointer points to:

- Full Ascending Stack
- Empty Ascending Stack
- **Full Descending Stack**: used in ARM Cortex-M
- Empty Descending Stack

### Stack Operations

- **PUSH**:
  - Stores registers on the stack, with the lowest numbered register at the lowest memory address and the highest at the highest memory address.
  - Uses the value in the SP register minus four as the highest memory address.
  - Updates the SP register to point to the location of the lowest stored value after execution.
  
- **POP**:
  - Loads registers from the stack, following the same address scheme as PUSH.
  - Uses the value in the SP register as the lowest memory address.
  - Updates the SP register to point to the location immediately above the highest loaded value after execution.

## Examples

### Example 1:  Using A Subroutine Call

```asm
	PRESERVE8 
	THUMB
    AREA  RESET, DATA, READONLY
    EXPORT __Vectors
__Vectors
    DCD 0x20001000 
    DCD Reset_Handler
    ALIGN

SUMP 	DCD SUM
SUMP2 	DCD SUM2
N		DCD 5
    AREA MYDATA, DATA, READWRITE
SUM  	DCD 0
SUM2	DCD 0
    AREA MYCODE, CODE, READONLY
    ENTRY
    EXPORT Reset_Handler
		
SUMUP	PROC
	ADD  R0, R0, R1 ;Add number into R0 
	SUBS  R1, R1, #1;Decrement loop counter R1 
	BGT  SUMUP   	;Branch back if not done 
	;MOV  PC, LR 
	BX  LR 
	ENDP
Reset_Handler
	LDR  R1, N   	;Load count into R1 
	MOV  R0, #0 	;Clear accumulator R0 
	BL   SUMUP 

	LDR  R3, SUMP 	;Load address of SUM to R3 
	STR  R0, [R3] 	;Store SUM
	LDR  R4, [R3] 
	
	MOV	R7, #8
	LDR R5, SUMP2	; Load address of SUM2 to R5
	STR R7, [R5]	; Store SUM2
	
	LDR R6, [R5]
STOP
    B STOP
    END
```

### Example 2: Using Stack 


```asm
	PRESERVE8 
	THUMB
INITIAL_MSP EQU 0x20001000 ; Initial Main Stack Pointer Value   
	
    AREA  RESET, DATA, READONLY
    EXPORT __Vectors
__Vectors
    DCD INITIAL_MSP 
    DCD Reset_Handler
    ALIGN

SUMP 	DCD SUM
SUMP2 	DCD SUM2
N		DCD 5
    AREA MYDATA, DATA, READWRITE
SUM  	DCD 0
SUM2	DCD 0
    AREA MYCODE, CODE, READONLY
    ENTRY
    EXPORT Reset_Handler
		

function1 PROC  		;Using PROC and ENDP for procedures 
	PUSH  {R5,LR}       ;Save values in the stack 
	MOV R5,#8			;Set initial value for the delay loop 
delay 
	SUBS R5, R5, #1 
	BNE delay 

	POP {R5,PC} 		;pop out the saved value from the stack,  
						;check the value in the R5 and if it is the saved value 
	ENDP
	
Reset_Handler
	MOV  R0, #0x75 
	MOV  R3, #5 
	PUSH {R0, R3}  		;Notice the stack address is 0x200000FF8 
	MOV  R0, #6 
	MOV  R3, #7 
	POP {R0, R3}  		;Should be able to see R0 = #0x75 and R3 = #5 after pop 
 
 
Loop 
	ADD R0, R0, #1 
	CMP R0, #0x80 
	BNE Loop 
	MOV  R5, #9 ;; prepare for function call 
	BL  function1 
	MOV  R3, #12
STOP
    B STOP
    END        
```

## Lab Work

### Count Vowels and Non-Vowels Procedure

Write an ARM assembly language program `CountVowelsTwo.s` to count how many
vowels and how many non-vowels are in the following string.

> "ARM assembly language is important to learn!",0  

You are required to implement this by using a subroutine to check if a character is vowel or not, and count the vowels and non-vowels in the calling function. Recommendations for writing the program:

- Put the string in the memory by using DCB.
- Use R0 to hold the address of a character in the string.
- Use R2 to be the counter for vowels.
- Use R3 to be the counter for non-vowels.

### Factorial Procedure

Write an ARM assembly language program `Factorial.s` to calculate the factorial of a number.

The factorial of a number is the product of all the integers from 1 to that number. For example

- The factorial of 5 can be expressed as:
  - 5! = 5 x 4 x 3 x 2 x 1 = 120


You are required to implement this by using a subroutine to calculate the factorial of a number. Recommendations for writing the program:

- Use R1 to hold the number for which the factorial is to be calculated.
- Use R0 to hold the result of the factorial.
- Use stack to save the return address and the value of R0.
