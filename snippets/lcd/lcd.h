#ifndef LCD_H
#define LCD_H

#include "TM4C123.h"

// LCD pins, all on Port B
#define RS         (1U << 0)            // PB0: 0 = command, 1 = character
#define RW         (1U << 1)            // PB1: always 0 (write)
#define EN         (1U << 2)            // PB2: enable, the LCD latches on its falling edge
#define DATA_MASK  0xF0U                // PB4-PB7: D4-D7

// LCD commands (see the command set)
#define LCD_FUNCTION_4BIT_2LINE  0x28   // 4-bit interface, 2 lines, 5x8 dots
#define LCD_DISPLAY_ON           0x0C   // Display on, cursor off
#define LCD_ENTRY_INCREMENT      0x06   // Cursor moves right after each character
#define LCD_CLEAR                0x01   // Clear, cursor home
#define LCD_LINE1                0x80   // Set DDRAM address: line 1, column 0
#define LCD_LINE2                0xC0   // Set DDRAM address: line 2, column 0

void LCD_Init(void);
void LCD_Command(unsigned char cmd);
void LCD_Data(unsigned char data);
void LCD_Clear(void);
void LCD_SetCursor(unsigned char row, unsigned char col);
void LCD_Print(char *str);
void delay_us(int us);
void delay_ms(int ms);

#endif
