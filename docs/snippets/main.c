
#include "TM4C123.h"
#include "lcd.h"




int main(void)
{

    LCD_Init();
    LCD_Clear();        // Ensure display is clear
    LCD_SetCursor(0,0); // Set cursor to beginning
	LCD_Print("ENCS4110 Lab");
    
    while(1)
    {	
    }
}
