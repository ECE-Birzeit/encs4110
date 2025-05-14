---
outline: deep
lastUpdated: true
next: false
---

# Liquid Crystal Display (LCD) <Badge type="tip" text="Experiment 9" />


![alt text](/image-3.png)
## Introduction

The 16x2 LCD module is one of the most commonly used display devices in embedded systems. It can display 16 characters per line on 2 lines and is widely used for text-based user interfaces. The LCD uses the HD44780 controller, which can operate in either 8-bit or 4-bit mode. In this experiment, we focus on interfacing a 16x2 LCD with the TM4C123 microcontroller using the efficient 4-bit mode.

### Why 4-bit Mode?

Using 4-bit mode reduces the number of GPIO pins needed, as it sends data in two 4-bit nibbles rather than a single 8-bit byte. This mode is particularly useful when GPIO pins are limited.

### Understanding LCD Operation

The LCD has two main registers:

* **Command Register**: Used to send commands to the LCD (e.g., clear display, move cursor).
* **Data Register**: Used to send data (characters) to be displayed.

The LCD also has control pins that determine the mode of operation:

* **RS (Register Select)**: Selects between command and data register.
* **RW (Read/Write)**: Selects between read and write operations.
* **E (Enable)**: Latches the data into the LCD.
* **D0-D7**: Data pins used to send commands and data.


### Sending Commands and Data

To send a **command**:

1. Set **RS = 0** (Command mode).
2. Set **RW = 0** (Write mode).
3. Send the higher nibble (4 bits) using data pins (D4-D7).
4. Toggle the **E (Enable)** pin to latch the data.
5. Send the lower nibble similarly.
6. Wait for the command to complete.

To send **data**:

1. Set **RS = 1** (Data mode).
2. Set **RW = 0** (Write mode).
3. Send the higher nibble of the character.
4. Toggle **E** to latch the data.
5. Send the lower nibble.
6. Wait for the data to appear on the screen.

### How 4-bit Mode Works

In 4-bit mode, only the upper 4 data pins (D4-D7) are used to send data to the LCD. The data is sent in two parts:

1. High nibble (upper 4 bits)
2. Low nibble (lower 4 bits)

This requires two data writes to send a single 8-bit command or character. The Enable (E) pin is toggled after each nibble to latch the data.

### Initialization Process in 4-bit Mode

1. Set RS, RW, and E pins to low.
2. Wait for more than 40 ms after power-on.
3. **Send command 0x30 (upper nibble only '0011') and wait >4.1ms**
4. **Send command 0x30 (upper nibble only '0011') again and wait >100μs**
5. **Send command 0x30 (upper nibble only '0011') a third time and wait >100μs**
6. **Send command 0x20 (upper nibble only '0010') to switch to 4-bit mode and wait >100μs**
7. Set the function to 4-bit mode, 2 lines, and 5x7 dots by sending the command `0x28`.
8. Set the display on, cursor off, and blink off by sending the command `0x0C`.
9. Set the entry mode to increment cursor and no display shift by sending the command `0x06`.
10. Clear the display by sending the command `0x01`.
11. Return home by sending the command `0x02`.

**Note:** During steps 3-6 (initialization), only the upper nibble is sent because the LCD doesn't yet know it's in 4-bit mode. After step 6, both nibbles (upper and lower) must be sent for each command.

### **LCD Command Reference**

| Command | Hex Code | Binary | Description | Effects and Details |
|---------|----------|--------|-------------|---------------------|
| **Clear Display** | 0x01 | 0000 0001 | Clears the entire display | Returns cursor to home position (0,0). Takes about 1.52ms to execute. |
| **Return Home** | 0x02 | 0000 0010 | Returns cursor to home position | Moves cursor to the top-left position without clearing display. Takes about 1.52ms. |
| **Entry Mode Set** | 0x04/0x06 | 0000 01xx | Sets cursor movement direction and display shift | Bit 1 (I/D): 1=Increment cursor, 0=Decrement cursor.<br>Bit 0 (S): 1=Shift display, 0=No shift. |
| **Display Control** | 0x08-0x0F | 0000 1DCB | Controls display, cursor, and blinking | Bit 2 (D): 1=Display on, 0=Display off.<br>Bit 1 (C): 1=Cursor on, 0=Cursor off.<br>Bit 0 (B): 1=Blinking on, 0=Blinking off. |
| **Cursor/Display Shift** | 0x10-0x1F | 0001 SC00 | Moves cursor or shifts display | Bit 3 (S/C): 1=Display shift, 0=Cursor move.<br>Bit 2 (R/L): 1=Right shift, 0=Left shift. |
| **Function Set** | 0x20-0x3F | 001D NF00 | Sets interface data length, lines, and font | Bit 4 (DL): 1=8-bit interface, 0=4-bit interface.<br>Bit 3 (N): 1=2 lines, 0=1 line.<br>Bit 2 (F): 1=5×10 dots, 0=5×8 dots. |
| **Set CGRAM Address** | 0x40-0x7F | 01xx xxxx | Sets CGRAM address | Sets the address for custom character generation (lower 6 bits provide address). |
| **Set DDRAM Address** | 0x80-0xFF | 1xxx xxxx |  Sets DDRAM address | Sets cursor position. For most LCDs:<br>- Line 1: 0x80-0x8F<br>- Line 2: 0xC0-0xCF |

**Common Command Combinations**

| Function | Command Sequence | Description |
|----------|------------------|-------------|
| **Display On, No Cursor** | 0x0C | Turns display on with no cursor |
| **Display On with Cursor** | 0x0E | Turns display on with non-blinking cursor |
| **Display On with Blinking Cursor** | 0x0F | Turns display on with blinking cursor |
| **Move to Start of Line 1** | 0x80 | Positions cursor at the beginning of first line |
| **Move to Start of Line 2** | 0xC0 | Positions cursor at the beginning of second line |
| **Create Custom Character** | 0x40+addr, followed by 8 bytes of pattern data | Stores custom character pattern in CGRAM |

**Entry Mode Set Options (Command 0x04 + bits)**

| Hex | Binary | Description |
|-----|--------|-------------|
| 0x04 | 0000 0100 | Decrement cursor position (move left), no display shift |
| 0x05 | 0000 0101 | Decrement cursor position (move left), with display shift |
| 0x06 | 0000 0110 | Increment cursor position (move right), no display shift (most common) |
| 0x07 | 0000 0111 | Increment cursor position (move right), with display shift |

**Display Control Options (Command 0x08 + bits)**

| Hex | Binary | Description |
|-----|--------|-------------|
| 0x08 | 0000 1000 | Display off, cursor off, blinking off |
| 0x0C | 0000 1100 | Display on, cursor off, blinking off (most common) |
| 0x0E | 0000 1110 | Display on, cursor on, blinking off |
| 0x0F | 0000 1111 | Display on, cursor on, blinking on |


## Example Code
```c
#include "TM4C123.h"

// Define control pins (PB0: RS, PB2: EN)
#define RS 0x01        // PB0
#define EN 0x04        // PB2
#define DATA_MASK 0xF0 // PB4-PB7 for D4-D7

// Function prototypes
void LCD_Init(void);
void LCD_Command(unsigned char command);
void LCD_Data(unsigned char data);
void LCD_Clear(void);
void LCD_SetCursor(unsigned char row, unsigned char col);
void LCD_Print(char *str);
void delay_ms(int ms);
void delay_us(int us);
void LCD_EnablePulse(void);
void LCD_SendNibble(unsigned char nibble);
void SysTick_Init(void);

int main(void)
{

    LCD_Init();
    delay_ms(100);      // Additional delay after init
    LCD_Clear();        // Ensure display is clear
    delay_ms(5);        // Wait after clear
    LCD_SetCursor(0,0); // Set cursor to beginning
    delay_ms(1);        // Wait after setting cursor
    LCD_Print("ENCS4110 Lab");
    
    while(1)
    {
        // Main loop
    }
}

void SysTick_Init(void)
{
    SysTick->CTRL = 0;                   // Disable SysTick during setup
    SysTick->LOAD = 16000000/1000 - 1;   // Load value for 1ms delay at 16MHz
    SysTick->VAL = 0;                    // Clear current value
    SysTick->CTRL = 0x5;                 // Enable SysTick, use system clock
}

void delay_us(int us)
{
    SysTick->LOAD = (16 * us) - 1;     // 16 cycles = 1us at 16MHz
    SysTick->VAL = 0;                  // Clear current value
    SysTick->CTRL |= 0x1;              // Enable SysTick
    while((SysTick->CTRL & 0x10000) == 0); // Wait for COUNT flag
}

void delay_ms(int ms)
{
    SysTick->LOAD = (16000 * ms) - 1;  // 16000 cycles = 1ms at 16MHz
    SysTick->VAL = 0;                  // Clear current value
    SysTick->CTRL |= 0x1;              // Enable SysTick
    while((SysTick->CTRL & 0x10000) == 0); // Wait for COUNT flag
}

void LCD_EnablePulse(void)
{
    delay_us(1);          // Pre-pulse delay
    GPIOB->DATA |= EN;    // EN = 1
    delay_us(1);          // Wait > 450ns
    GPIOB->DATA &= ~EN;   // EN = 0
    delay_us(1);          // Post-pulse delay
}

// Helper function to send a single nibble (4 bits) to the LCD
void LCD_SendNibble(unsigned char nibble)
{
    // Send only the upper 4 bits (connected to PB4-PB7)
    GPIOB->DATA = (GPIOB->DATA & ~DATA_MASK) | (nibble & DATA_MASK);
    LCD_EnablePulse();
}

void LCD_Init(void)
{
    // Enable and wait for clock to PORTB
    SYSCTL->RCGCGPIO |= (1 << 1);
    while((SYSCTL->PRGPIO & (1 << 1)) == 0);
    
    // Configure port B pins
    GPIOB->DIR |= (RS | EN | DATA_MASK);  // Set as outputs
    GPIOB->DEN |= (RS | EN | DATA_MASK);  // Enable digital
    
    // Initialize SysTick for delays
    SysTick_Init();
    
    // Initial state
    GPIOB->DATA &= ~(RS | EN | DATA_MASK);  // All pins low
    delay_ms(50);   // Wait after power up (at least 40ms)
    
    // The crucial three-time 0x30 initiation sequence
    // During initialization, we're only sending the upper nibble (0x3)
    // because the LCD doesn't know it's in 4-bit mode yet
    
    // First send of 0x30 (only upper nibble)
    LCD_Command(0x30);
    delay_ms(5);    // Wait > 4.1ms
    
    // Second send of 0x30 (only upper nibble)
    LCD_Command(0x30);
    delay_us(200);  // Wait > 100us
    
    // Third send of 0x30 (only upper nibble)
    LCD_Command(0x30);
    delay_us(200);  // Wait > 100us
    
    // Switch to 4-bit mode (only sending upper nibble 0x2)
    LCD_Command(0x20);
    delay_us(200);  // Wait > 100us
    
    // Now in 4-bit mode - all subsequent commands will send both nibbles
    LCD_Command(0x28);  // Function set: 4-bit, 2 lines, 5x7 dots
    delay_ms(1);
    LCD_Command(0x0C);  // Display ON, cursor OFF, blink OFF
    delay_ms(1);
    LCD_Command(0x06);  // Entry mode set: Increment cursor
    delay_ms(1);
    LCD_Command(0x01);  // Clear display
    delay_ms(2);
    LCD_Command(0x02);  // Return home
    delay_ms(2);
}

void LCD_Command(unsigned char command)
{
    GPIOB->DATA &= ~RS;  // RS = 0 for command
    delay_us(1);
    
    // Are we in initialization mode (before 4-bit mode is set)?
    if (command == 0x30 || command == 0x20) {
        // For initialization, we only send the upper nibble
        LCD_SendNibble(command);
    } else {
        // Normal 4-bit operation - send both nibbles
        // Send upper nibble
        LCD_SendNibble(command);
        
        // Send lower nibble
        LCD_SendNibble(command << 4);
    }
    
    delay_ms(2);  // Wait for command to complete
}

void LCD_Data(unsigned char data)
{
    GPIOB->DATA |= RS;  // RS = 1 for data
    delay_us(1);
    
    // Send upper nibble
    LCD_SendNibble(data);
    
    // Send lower nibble
    LCD_SendNibble(data << 4);
    
    delay_ms(1);  // Wait for data to complete
}

void LCD_Clear(void)
{
    LCD_Command(0x01);  // Clear display command
    delay_ms(2);        // Clear command needs 1.52ms
}

void LCD_SetCursor(unsigned char row, unsigned char col)
{
    unsigned char address;
    address = (row == 0) ? 0x80 + col : 0xC0 + col;  // Row 0 or 1
    LCD_Command(address);
    delay_ms(1);
}

void LCD_Print(char *str)
{
    while (*str)
    {
        LCD_Data(*str++); // Print the current character
        delay_ms(1);      // Small delay between characters
    }
}
```
