---
outline: deep
lastUpdated: true
---

# GPIO (General Purpose Input/Output) Interface Using TM4C123G Boards <Badge type="tip" text="Experiment 6" />

## Introduction to TM4C123G LaunchPad

The TM4C123G LaunchPad Evaluation Kit is a low-cost evaluation platform for ARM Cortex-M4F based microcontrollers from Texas Instruments. The design of the TM4C123G LaunchPad highlights the TM4C123GH6PM microcontroller with a micro USB device interface and hibernation module. The EK-TM4C123GXL also features programmable user buttons and an RGB LED for custom applications.

**Features:**

- Micro AB USB port
- Onboard RGB LED
- Reset button
- Two user push buttons
- 3.3V regulator
- 16MHz main oscillator crystal
- 32.768kHz Real Time Clock (RTC) crystal
- 0-43 GPIO pins (depending on configuration)

## GPIO (General Purpose Input/Output) Interface

The LaunchPad has 43 GPIO pins, each can be used as an external edge/level triggered interrupt source. The GPIO pins can be configured as digital inputs or outputs, and can be used to interface with external devices. each pin has a tolerance voltage of 5V. and each pin has a weak pull-up, pull-down and open drain.
The GPIO pins are divided into 6 ports, each port is associated with many registers that control the behavior of the pins. And they have designated addresses in the memory map of the microcontroller.

| Port Name   | Lower Address | Upper Address |
|-------------|---------------|---------------|
| GPIO port A | 0x40004000    | 0x40004FFF    |
| GPIO port B | 0x40005000    | 0x40005FFF    |
| GPIO port C | 0x40006000    | 0x40006FFF    |
| GPIO port D | 0x40007000    | 0x40007FFF    |
| GPIO port E | 0x40024000    | 0x40024FFF    |
| GPIO port F | 0x40025000    | 0x40025FFF    |

### GPIO Registers

#### GPIO Data Register (GPIODATA)

The GPIO Data register is used to read or write the value of the GPIO pins. The GPIODATA register is a 32-bit register, each bit in the register corresponds to a pin in the port. Writing a 1 to a bit in the register will set the corresponding pin to high, and writing a 0 will set the pin to low. Reading the register will return the current value of the pins.

#### GPIO Direction Register (GPIODIR)

The GPIO Direction register is used to set the direction of the pins in the port. Writing a 1 to a bit in the register will set the corresponding pin to output, and writing a 0 will set the pin to input.

#### GPIO Digital Enable Register (GPIODEN)

The GPIO Digital Enable register is used to enable the digital functionality of the pins in the port. Writing a 1 to a bit in the register will enable the digital functionality of the corresponding pin, and writing a 0 will disable the digital functionality.

#### GPIO  Run Mode Clock Gating Control Register (RCGCGPIO)

The GPIO Run Mode Clock Gating Control register is used to enable the clock to the GPIO ports. Writing a 1 to a bit in the register will enable the clock to the corresponding port, and writing a 0 will disable the clock to the port.

## LEDs and Switches on TM4C123G LaunchPad

The TM4C123G LaunchPad has 3 LEDs (Red, Blue, Green) in a single package, and 2 switches. The LEDs are connected to the GPIO pins of port F, and the switches are connected to the GPIO pins of port F as shown in the following table.

| Device   | Port | Pin  | Function |
|----------|------|------|----------|
| Red LED  | F    | 1    | Output   |
| Blue LED | F    | 2    | Output   |
| Green LED| F    | 3    | Output   |
| PUSH 1   | F    | 4    | Input    |
| PUSH 2   | F    | 0    | Input    |

## Examples

### Example 1:  Blinking LED using Assembly

```asm
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
```

### Example 2:  Blinking LED using C

```c

#define SYSCTL_RCGCGPIO_R (*((volatile unsigned long *)0x400FE608))

#define GPIO_PORTF_DATA_R (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_DEN_R (*((volatile unsigned long *)0x4002551C))

#define DELAY 200000

int main(void)
{
    volatile unsigned long ulLoop;

    // Enable the GPIO port that is used for the onboard LED.

    SYSCTL_RCGCGPIO_R = 0x20;

    // Do a dummy read to insert a few cycles after enabling the peripheral.

    ulLoop = SYSCTL_RCGCGPIO_R;

    //_ Enable the GPIO pin for the LED (PF3). Set the direction as output and enable the GPIO pin for digital //function. _/

    GPIO_PORTF_DIR_R = 0x08;
    GPIO_PORTF_DEN_R = 0x08;

    // Loop for ever.

    while (1)
    {
        // Toggle  the LED.

        GPIO_PORTF_DATA_R ^= 0x08; // ^ means XOR in c

        // Delay for a bit.

        for (ulLoop = 0; ulLoop < DELAY; ulLoop++)
        {

            for (ulLoop = 0; ulLoop < DELAY; ulLoop++)
            {
            }
        }
    }
} 
```

## Labwork

1. Modify the assembly program so that the Blue Led is flashing instead of the Green one. 
   1. Increase delay amount to 2000000 and observe the response.
   2. Decrease the delay amount to 2000 and observe the response.
2. Modify the c code so the three Leds light in this sequence R->B->G. Choose a reasonable delay so the three led’s can be observed.