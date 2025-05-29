#include "lcd.h"

#define SYSTEM_CLOCK_HZ 50000000 // 50 MHz system clock
#define CYCLES_PER_US   (SYSTEM_CLOCK_HZ / 1000000)



//====================[ SysTick Delay Functions ]====================
void SysTick_Init(void)
{
    SysTick->CTRL = 0;
    SysTick->LOAD = CYCLES_PER_US - 1;  // 1us delay at 50MHz
    SysTick->VAL = 0;
    SysTick->CTRL = 0x5;     // Enable with system clock
}

void delay_us(int us)
{
    SysTick->LOAD = (CYCLES_PER_US * us) - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = 0x5; // Enable with system clock
    while ((SysTick->CTRL & 0x10000) == 0);
    SysTick->CTRL = 0;
}

void delay_ms(int ms)
{
    while (ms--)
        delay_us(1000);
}

//====================[ LCD Helper Functions ]====================
void LCD_EnablePulse(void)
{
    delay_us(1);
    GPIOB->DATA |= EN;
    delay_us(1);
    GPIOB->DATA &= ~EN;
    delay_us(1);
}

void LCD_SendNibble(unsigned char nibble)
{
    // Send nibble to PB4–PB7
    GPIOB->DATA = (GPIOB->DATA & ~DATA_MASK) | ((nibble << 4) & DATA_MASK);
    LCD_EnablePulse();
}

//====================[ LCD Initialization ]====================
void LCD_Init(void)
{
    // Enable clock to PORTB
    SYSCTL->RCGCGPIO |= (1 << 1);
    while ((SYSCTL->PRGPIO & (1 << 1)) == 0)
        ;

    // Configure PB0 (RS), PB1 (EN), PB4–PB7 (data) as output
    GPIOB->DIR |= RS | EN | DATA_MASK;
    GPIOB->DEN |= RS | EN | DATA_MASK;
    GPIOB->DATA &= ~(RS | EN | DATA_MASK);  // Clear all

    SysTick_Init();

    delay_ms(50);  // Wait for LCD to power up

    // Initialization sequence (8-bit interface mode to start)
    LCD_SendNibble(0x03);
    delay_ms(5);

    LCD_SendNibble(0x03);
    delay_us(150);

    LCD_SendNibble(0x03);
    delay_us(150);

    LCD_SendNibble(0x02);  // Set 4-bit mode
    delay_us(150);

    // Now in 4-bit mode: use full commands
    LCD_Command(0x28); // Function set: 4-bit, 2 lines, 5x8 dots
    LCD_Command(0x0C); // Display ON, Cursor OFF
    LCD_Command(0x06); // Entry mode: increment cursor
    LCD_Command(0x01); // Clear display
    delay_ms(2);
}

//====================[ LCD Command/Data API ]====================
void LCD_Command(unsigned char command)
{
    GPIOB->DATA &= ~RS; // RS = 0 for command
    delay_us(1);
    LCD_SendNibble(command >> 4);     // Upper nibble
    LCD_SendNibble(command & 0x0F);   // Lower nibble
    delay_ms(2);
}

void LCD_Data(unsigned char data)
{
    GPIOB->DATA |= RS; // RS = 1 for data
    delay_us(1);
    LCD_SendNibble(data >> 4);
    LCD_SendNibble(data & 0x0F);
    delay_ms(1);
}

void LCD_Clear(void)
{
    LCD_Command(0x01);
    delay_ms(2);
}

void LCD_SetCursor(unsigned char row, unsigned char col)
{
    unsigned char address = (row == 0) ? 0x80 + col : 0xC0 + col;
    LCD_Command(address);
    delay_ms(1);
}

void LCD_Print(char *str)
{
    while (*str)
    {
        LCD_Data(*str++);
    }
}
