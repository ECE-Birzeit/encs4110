#include "TM4C123.h"
#include "lcd.h"

int main(void)
{
	LCD_Init();
	LCD_Clear();                    // Make sure the display is clear
	LCD_SetCursor(0, 0);            // Line 1, column 0
	LCD_Print("ENCS4110 Lab");

	while (1)
	{
	}
}
