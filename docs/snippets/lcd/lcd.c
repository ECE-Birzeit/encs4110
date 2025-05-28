#include "lcd.h"

void SysTick_Init(void)
{
    SysTick->CTRL = 0;                   // Disable SysTick during setup
    SysTick->LOAD = 50000000/1000 - 1;   // Load value for 1ms delay at 50MHz
    SysTick->VAL = 0;                    // Clear current value
    SysTick->CTRL = 0x5;                 // Enable SysTick, use system clock
}

void delay_us(int us)
{
    SysTick->LOAD = 50 * us - 1;      // 1 us = 50 clock cycles at 50 MHz
    SysTick->VAL = 0;
    SysTick->CTRL = 0x5;              // Enable SysTick with system clock
    while ((SysTick->CTRL & 0x10000) == 0); // Wait for COUNT flag
    SysTick->CTRL = 0;                // Disable SysTick after done
}

void delay_ms(int ms)
{
    while (ms--)
    {
        delay_us(1000);  // 1 ms = 1000 us
    }
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

    for (int i = 0; i < 3; i++) {
        LCD_Command(0x30);  // Send 0x30 (upper nibble only) 3 times
    }

    // Switch to 4-bit mode (only sending upper nibble 0x2)
    LCD_Command(0x20);
    
    // Now in 4-bit mode - all subsequent commands will send both nibbles
    LCD_Command(0x28);  // Function set: 4-bit, 2 lines, 5x7 dots
    LCD_Command(0x0C);  // Display ON, cursor OFF, blink OFF
    LCD_Command(0x06);  // Entry mode set: Increment cursor
    LCD_Command(0x01);  // Clear display
    LCD_Command(0x02);  // Return home
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
    delay_ms(1);  // Wait for data to be processed
}

void LCD_Clear(void)
{
    LCD_Command(0x01);  // Clear display command
    delay_ms(5);        // Clear command needs 1.52ms
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
    }
}