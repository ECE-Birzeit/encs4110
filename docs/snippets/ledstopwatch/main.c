#include "TM4C123.h"
#include "lcd.h"
#include "ledstopwatch.h"

int main(void)
{
    LCD_Init();
    Stopwatch_GPIO_Init();
    Timer0_Init();
    GPIOF_ButtonInterrupt_Init();

    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("Time:");

    while (1) {
        Stopwatch_Update();
        Stopwatch_Display();
    }
}
