#include "lcd.h"

//====================[ Timebase: SysTick, 1 ms tick ]====================
static volatile uint32_t ms_ticks = 0;  // incremented every 1 ms by SysTick_Handler

void SysTick_Handler(void)
{
    ms_ticks++;
}

static void Timebase_Init(void)
{
    SysTick_Config(SystemCoreClock / 1000);  // reload every 1 ms, enable interrupt, start
}

uint32_t millis(void)
{
    return ms_ticks;
}

void delay_ms(uint32_t ms)
{
    // Count SysTick reloads (COUNTFLAG) instead of watching ms_ticks: inside another
    // interrupt handler SysTick_Handler can't run, so ms_ticks would never change.
    if ((SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) == 0)  // reading CTRL also clears COUNTFLAG
        Timebase_Init();

    ms++;  // the first reload can come at any point in the next 1 ms, so wait one extra
    while (ms)
    {
        if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)  // set once per reload, cleared by the read
            ms--;
    }
}

//====================[ LCD Helper Functions ]====================
static void LCD_PulseDelay(void)
{
    for (volatile int i = 0; i < 20; i++);  // ~2 us at 50 MHz; EN needs at least 450 ns
}

static void LCD_EnablePulse(void)
{
    GPIOB->DATA |= EN;
    LCD_PulseDelay();
    GPIOB->DATA &= ~EN;  // LCD latches D4-D7 on this falling edge
    LCD_PulseDelay();
}

static void LCD_SendNibble(unsigned char nibble)
{
    // Send nibble to PB4-PB7
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

    // Configure PB0 (RS), PB1 (RW), PB2 (EN), PB4-PB7 (data) as output
    GPIOB->DIR |= RS | RW | EN | DATA_MASK;
    GPIOB->DEN |= RS | RW | EN | DATA_MASK;
    GPIOB->DATA &= ~(RS | RW | EN | DATA_MASK);  // Clear all

    Timebase_Init();

    delay_ms(50);  // Wait for LCD to power up

    // Initialization sequence (8-bit interface mode to start)
    LCD_SendNibble(0x03);
    delay_ms(5);

    LCD_SendNibble(0x03);
    delay_ms(1);

    LCD_SendNibble(0x03);
    delay_ms(1);

    LCD_SendNibble(0x02);  // Set 4-bit mode
    delay_ms(1);

    // Now in 4-bit mode: use full commands
    LCD_Command(0x28); // Function set: 4-bit, 2 lines, 5x8 dots
    LCD_Command(0x0C); // Display ON, Cursor OFF
    LCD_Command(0x06); // Entry mode: increment cursor
    LCD_Clear();
}

//====================[ LCD Command/Data API ]====================
void LCD_Command(unsigned char command)
{
    GPIOB->DATA &= ~RS; // RS = 0 for command
    LCD_SendNibble(command >> 4);     // Upper nibble
    LCD_SendNibble(command & 0x0F);   // Lower nibble
    delay_ms(2);                      // Longest command (clear/home) needs 1.52 ms
}

void LCD_Data(unsigned char data)
{
    GPIOB->DATA |= RS; // RS = 1 for data
    LCD_SendNibble(data >> 4);
    LCD_SendNibble(data & 0x0F);
    delay_ms(1);                      // Writing a character needs 37 us
}

void LCD_Clear(void)
{
    LCD_Command(0x01);
}

void LCD_SetCursor(unsigned char row, unsigned char col)
{
    unsigned char address = (row == 0) ? 0x80 + col : 0xC0 + col;
    LCD_Command(address);
}

void LCD_Print(char *str)
{
    while (*str)
    {
        LCD_Data(*str++);
    }
}
