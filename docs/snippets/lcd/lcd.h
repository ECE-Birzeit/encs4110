#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include "TM4C123.h"

// LCD pin definitions (connected to PORTB)
#define RS         (1 << 0)  // PB0
#define RW         (1 << 1)  // PB1
#define EN         (1 << 2)  // PB2
#define DATA_MASK  0xF0      // PB4-PB7

// LCD functions
void LCD_Init(void);
void LCD_Command(unsigned char cmd);
void LCD_Data(unsigned char data);
void LCD_Clear(void);
void LCD_SetCursor(unsigned char row, unsigned char col);
void LCD_Print(char *str);

// Timebase: SysTick interrupts every 1 ms (started by LCD_Init).
// lcd.c owns SysTick_Handler, so don't define your own -- use millis() instead.
uint32_t millis(void);       // milliseconds since LCD_Init()
void delay_ms(uint32_t ms);  // blocking delay, safe to call inside interrupt handlers

#endif
