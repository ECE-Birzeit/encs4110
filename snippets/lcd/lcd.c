#include "lcd.h"

#define RCGC_GPIOB      (1U << 1)       // RCGCGPIO bit for Port B
#define LCD_PINS        (RS | RW | EN | DATA_MASK)

// SysTick, used here as a busy-wait timer
#define STCTRL_ENABLE   (1U << 0)       // CTRL: counter on
#define STCTRL_CLK_SRC  (1U << 2)       // CTRL: count the system clock
#define STCTRL_COUNT    (1U << 16)      // CTRL: reached 0 since the last read
#define CYCLES_PER_US   (SystemCoreClock / 1000000U)

//====================[ SysTick Delay Functions ]====================
void delay_us(int us)
{
	SysTick->CTRL = 0;
	SysTick->LOAD = CYCLES_PER_US * us - 1;
	SysTick->VAL  = 0;
	SysTick->CTRL = STCTRL_CLK_SRC | STCTRL_ENABLE;         // On, no interrupt
	while ((SysTick->CTRL & STCTRL_COUNT) == 0);            // Wait until it reaches 0
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
	GPIOB->DATA &= ~EN;                             // Falling edge: the LCD latches the nibble
	delay_us(1);
}

void LCD_SendNibble(unsigned char nibble)
{
	GPIOB->DATA = (GPIOB->DATA & ~DATA_MASK) | ((nibble << 4) & DATA_MASK);    // On PB4-PB7
	LCD_EnablePulse();
}

//====================[ LCD Initialization ]====================
void LCD_Init(void)
{
	SYSCTL->RCGCGPIO |= RCGC_GPIOB;                 // Clock to Port B
	while ((SYSCTL->PRGPIO & RCGC_GPIOB) == 0);     // Wait until it's ready

	GPIOB->DIR  |= LCD_PINS;                        // RS, RW, EN and D4-D7 out
	GPIOB->DEN  |= LCD_PINS;
	GPIOB->DATA &= ~LCD_PINS;                       // All low

	delay_ms(50);                                   // Let the LCD power up

	// Start in 8-bit mode three times, then switch to 4-bit
	LCD_SendNibble(0x03);
	delay_ms(5);
	LCD_SendNibble(0x03);
	delay_us(150);
	LCD_SendNibble(0x03);
	delay_us(150);
	LCD_SendNibble(0x02);                           // 4-bit mode
	delay_us(150);

	// Now in 4-bit mode: full commands
	LCD_Command(LCD_FUNCTION_4BIT_2LINE);
	LCD_Command(LCD_DISPLAY_ON);
	LCD_Command(LCD_ENTRY_INCREMENT);
	LCD_Command(LCD_CLEAR);
	delay_ms(2);
}

//====================[ LCD Command/Data API ]====================
void LCD_Command(unsigned char command)
{
	GPIOB->DATA &= ~RS;                             // RS = 0: a command
	delay_us(1);
	LCD_SendNibble(command >> 4);                   // Upper nibble
	LCD_SendNibble(command & 0x0F);                 // Lower nibble
	delay_ms(2);
}

void LCD_Data(unsigned char data)
{
	GPIOB->DATA |= RS;                              // RS = 1: a character
	delay_us(1);
	LCD_SendNibble(data >> 4);
	LCD_SendNibble(data & 0x0F);
	delay_ms(1);
}

void LCD_Clear(void)
{
	LCD_Command(LCD_CLEAR);
	delay_ms(2);
}

void LCD_SetCursor(unsigned char row, unsigned char col)
{
	LCD_Command((row == 0 ? LCD_LINE1 : LCD_LINE2) + col);
	delay_ms(1);
}

void LCD_Print(char *str)
{
	while (*str)
		LCD_Data(*str++);
}
