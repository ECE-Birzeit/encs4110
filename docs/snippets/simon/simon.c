#include "simon.h"

volatile uint32_t global_ms        = 0;
volatile uint8_t  pressed_button   = 0xFF;
volatile uint32_t last_press_ms[4] = {0, 0, 0, 0};

uint8_t sequence[MAX_SEQUENCE];
uint8_t sequence_len = 0;

void Simon_GPIO_Init(void)
{
    // TODO: enable the clocks for GPIOC and GPIOD
    // TODO: PC4-7 as inputs -- DIR cleared, DEN, PUR (pull-up)
    // TODO: PD0-3 as outputs -- DIR, DEN
}

void SysTick_Handler(void)
{
    // TODO: increment global_ms
}

void Simon_ShowSequence(void)
{
    // TODO -- Step 3: for i = 0 .. sequence_len-1, light LED (1 << sequence[i])
    //         on PD0-3 for a fixed duration with a gap between steps
}

void GPIOC_ButtonInterrupt_Init(void)
{
    // TODO -- Step 4: configure PC4-7 for a falling-edge interrupt + NVIC (GPIOC = IRQ 2)
}

void GPIOC_Handler(void)
{
    // TODO -- Step 4: for each of PC4-7 that triggered, clear its interrupt
    //         flag, and -- if at least DEBOUNCE_MS has passed since that
    //         button's own last accepted press (last_press_ms[i]) -- record
    //         it in pressed_button and update last_press_ms[i]
}

void Simon_Run(void)
{
    // TODO -- Step 5: drive the game -- show the sequence, consume
    //         pressed_button (set it back to 0xFF once read) in order,
    //         grow the sequence by one random step on a correct repeat,
    //         end the round on a wrong press -- see the assignment page
    //         for the exact requirements
}
