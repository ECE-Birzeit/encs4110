#include "TM4C123.h"
#include "keypad.h"

int main(void)
{
    KeyPad_GPIO_Init();
    SysTick_Config(SystemCoreClock / 1000);   // ~1 ms tick, drives the row scan
    Blink_Timer_Init();

    while (1) {
        KeyPad_Poll();
    }
}
