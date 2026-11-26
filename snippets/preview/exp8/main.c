// #region full
// #region defines
#include "TM4C123.h"

#define RCGC_GPIOB    (1U << 1)         // RCGCGPIO bit for Port B

// The LCD's pins on Port B
#define LCD_RS        (1U << 0)         // PB0: 0 = command, 1 = character
#define LCD_RW        (1U << 1)         // PB1: 0 = write (held low, always)
#define LCD_EN        (1U << 2)         // PB2: enable, the LCD latches on its falling edge
#define LCD_DATA      (0xFU << 4)       // PB4-PB7: D4-D7
#define LCD_PINS      (LCD_RS | LCD_RW | LCD_EN | LCD_DATA)

// HD44780 commands
#define CMD_CLEAR     0x01              // Clear the screen, cursor home (1.6 ms)
#define CMD_4BIT_2LN  0x28              // Function set: 4-bit, 2 lines, 5x8 dots
#define CMD_DISPLAY   0x0C              // Display on, cursor off, blink off
#define CMD_ENTRY     0x06              // Cursor moves right after each character
#define CMD_LINE1     0x80              // Set DDRAM address 0x00: line 1, column 0
#define CMD_LINE2     0xC0              // Set DDRAM address 0x40: line 2, column 0

#define CYCLES_PER_US (SystemCoreClock / 1000000U)
// #endregion defines


// #region low
// Busy-wait with SysTick: load, clear, wait for COUNTFLAG
void delay_us(uint32_t us)
{
	SysTick->LOAD = CYCLES_PER_US * us - 1;
	SysTick->VAL  = 0;
	SysTick->CTRL = 0x5;						// Enable, system clock, no interrupt
	while ((SysTick->CTRL & 0x10000) == 0);
	SysTick->CTRL = 0;
}

void delay_ms(uint32_t ms)
{
	while (ms--)
		delay_us(1000);
}

// Put a nibble on D7-D4 and pulse E: the LCD latches it on the falling edge
void LCD_SendNibble(uint8_t nibble)
{
	GPIOB->DATA = (GPIOB->DATA & ~LCD_DATA) | ((uint32_t)nibble << 4);
	delay_us(1);								// Data settles before E rises
	GPIOB->DATA |= LCD_EN;
	delay_us(1);								// E high for at least 450 ns
	GPIOB->DATA &= ~LCD_EN;
	delay_us(1);
}
// #endregion low


// #region lcd
void LCD_Command(uint8_t cmd)
{
	GPIOB->DATA &= ~LCD_RS;						// RS = 0: command
	LCD_SendNibble(cmd >> 4);					// Upper nibble first
	LCD_SendNibble(cmd & 0x0F);
	delay_ms(cmd == CMD_CLEAR ? 2 : 1);			// Clear takes 1.6 ms, the rest under 40 us
}

void LCD_Data(uint8_t ch)
{
	GPIOB->DATA |= LCD_RS;						// RS = 1: character
	LCD_SendNibble(ch >> 4);
	LCD_SendNibble(ch & 0x0F);
	delay_us(50);								// A character takes about 40 us
}

void LCD_Init(void)
{
	SYSCTL->RCGCGPIO |= RCGC_GPIOB;
	while ((SYSCTL->PRGPIO & RCGC_GPIOB) == 0);
	GPIOB->DIR  |= LCD_PINS;					// All seven pins are outputs
	GPIOB->DEN  |= LCD_PINS;
	GPIOB->DATA &= ~LCD_PINS;					// RW = 0 (write) from here on

	delay_ms(50);								// Let the LCD power up
	LCD_SendNibble(0x3);						// Three 0x3s: back to a known state
	delay_ms(5);
	LCD_SendNibble(0x3);
	delay_us(150);
	LCD_SendNibble(0x3);
	delay_us(150);
	LCD_SendNibble(0x2);						// 0x2: switch to 4-bit mode
	delay_us(150);

	LCD_Command(CMD_4BIT_2LN);					// From here, whole bytes as two nibbles
	LCD_Command(CMD_DISPLAY);
	LCD_Command(CMD_ENTRY);
	LCD_Command(CMD_CLEAR);
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
	LCD_Command((uint8_t)((row == 0 ? CMD_LINE1 : CMD_LINE2) + col));
}

void LCD_Print(const char *str)
{
	while (*str)
		LCD_Data((uint8_t)*str++);
}

void LCD_PrintNumber(uint32_t n)
{
	char digits[10];
	int  i = 0;

	do {										// Collect the digits, last one first
		digits[i++] = (char)('0' + n % 10);
		n /= 10;
	} while (n > 0);

	while (i > 0)								// Send them first one first
		LCD_Data((uint8_t)digits[--i]);
}
// #endregion lcd


// #region main
int main(void)
{
	uint32_t count = 0;

	LCD_Init();
	LCD_SetCursor(0, 0);
	LCD_Print("ENCS4110 Lab");
	LCD_SetCursor(1, 0);
	LCD_Print("Count: ");

	while (1)
	{
		LCD_SetCursor(1, 7);					// Just after "Count: "
		LCD_PrintNumber(count++);
		delay_ms(500);
	}
}
// #endregion main
// #endregion full
