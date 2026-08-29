#include "TM4C123.h"
#include "uart.h"
#include "trafficlight.h"

int main(void)
{
    TrafficLight_GPIO_Init();
    UART0_Init();
    GPIOF_ButtonInterrupt_Init();
    SysTick_Config(SystemCoreClock / 1000);   // ~1 ms tick

    while (1) {
        TrafficLight_Run();
    }
}
