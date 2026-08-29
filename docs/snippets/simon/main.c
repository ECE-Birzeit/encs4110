#include "TM4C123.h"
#include "simon.h"

int main(void)
{
    Simon_GPIO_Init();
    GPIOC_ButtonInterrupt_Init();
    SysTick_Config(SystemCoreClock / 1000);   // ~1 ms tick

    while (1) {
        Simon_Run();
    }
}
