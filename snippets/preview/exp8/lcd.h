// lcd.h: the I2C and LCD driver from the Experiment 8 Activity.
// The LCD's PCF8574 backpack is on I2C0: SCL = PB2, SDA = PB3, address 0x27.
// Before using it, call SysTick_Config(SystemCoreClock / 1000) to start the 1 ms clock,
// then I2C0_Init(), then LCD_Init(). Your SysTick_Handler must count: ms++.
// Never call delay_ms() or the LCD functions from an interrupt handler.
#ifndef LCD_H
#define LCD_H

#include "TM4C123.h"

extern volatile uint32_t ms;                    // Counted by SysTick_Handler, every 1 ms
uint32_t millis(void);                          // Milliseconds since SysTick started
void delay_ms(uint32_t n);                      // Wait n milliseconds
void I2C0_Init(void);                           // I2C0 master on PB2/PB3, 100 kHz
int  I2C0_Write(uint8_t addr, uint8_t data);    // One byte: 0 = acknowledged, -1 = no answer
void LCD_Init(void);                            // Power-up wait, 4-bit mode, clear
void LCD_Command(uint8_t cmd);                  // One command byte (RS = 0)
void LCD_Data(uint8_t ch);                      // One character (RS = 1) at the cursor
void LCD_SetCursor(uint8_t row, uint8_t col);   // Row 0 or 1, column 0 to 15
void LCD_Print(const char *str);                // A string at the cursor
void LCD_PrintNumber(uint32_t n);               // A number in decimal at the cursor

#endif
