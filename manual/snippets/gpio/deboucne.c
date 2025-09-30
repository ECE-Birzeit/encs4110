#include "TM4C123.h"

#define SWITCH (1u<<4)   // SW1 on PF4
#define LED    (1u<<3)   // Green LED on PF3

volatile uint32_t g_ms = 0;
volatile uint32_t last_ms = 0;
const uint32_t DEBOUNCE_MS = 150;

void SysTick_Handler(void) {
    g_ms++;
}

int main(void) {
    // Enable clock to Port F
    SYSCTL->RCGCGPIO |= (1u<<5);
    (void)SYSCTL->RCGCGPIO; // dummy read to settle

    // Unlock PF0 just in case
    GPIOF->LOCK = 0x4C4F434B;
    GPIOF->CR   = (1u<<0);

    // PF3 output, PF4 input with pull-up
    GPIOF->DIR &= ~SWITCH;
    GPIOF->DIR |=  LED;
    GPIOF->DEN |=  LED | SWITCH;
    GPIOF->PUR |=  SWITCH;

    // Configure PF4 interrupt: falling edge
    GPIOF->IS  &= ~SWITCH;   // edge-sensitive
    GPIOF->IBE &= ~SWITCH;   // not both edges
    GPIOF->IEV &= ~SWITCH;   // falling edge
    GPIOF->ICR  =  SWITCH;   // clear prior flag
    GPIOF->IM  |=  SWITCH;   // unmask

    NVIC_EnableIRQ(GPIOF_IRQn);

    // 1 ms SysTick tick
    SysTick_Config(SystemCoreClock / 1000u);


    for(;;) {
        __WFI(); // sleep until next interrupt
    }
}

void GPIOF_Handler(void) {

    uint32_t now = g_ms;

    if (GPIOF->MIS & SWITCH) {
        if ((now - last_ms) >= DEBOUNCE_MS) {
            last_ms = now;
            GPIOF->DATA ^= LED;  // toggle LED
        }
				GPIOF->ICR = SWITCH;   // clear interrupt
    }
}
