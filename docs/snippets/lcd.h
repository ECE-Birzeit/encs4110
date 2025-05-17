#ifndef LCD_H
#define LCD_H

#include "TM4C123.h"  // Or your MCU's main header

// Control pin definitions
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

#endif // LCD_H
