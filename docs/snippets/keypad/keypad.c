#include "keypad.h"

const char keymap[4][4] = {0};   // TODO: fill in row-major, matching your wiring diagram

volatile uint32_t global_ms   = 0;
volatile uint8_t  current_row = 0;

void KeyPad_GPIO_Init(void)
{
    // TODO -- Step 1: enable the clocks for GPIOC, GPIOD, GPIOF (RCGCGPIO)
    // TODO -- Step 1: PD0-3 as outputs -- DIR, DEN -- idle LOW
    // TODO -- Step 1: PC4-7 as inputs -- DEN, PDR (pull-down)
    // TODO -- Step 1: PF1-3 as outputs -- DIR, DEN
}

void SysTick_Handler(void)
{
    // TODO -- Step 2: increment global_ms
    // TODO -- Step 2: drive current_row HIGH and every other row LOW, then advance
    //                 current_row to the next value (0-3, wrapping back to 0)
}

void KeyPad_Poll(void)
{
    // TODO -- Step 2: read COL_MASK while current_row is whatever SysTick left active
    // TODO -- Step 2: if exactly one column reads HIGH, decode (current_row, column)
    //                 through keymap and call KeyPad_HandleKey
}

void Blink_Timer_Init(void)
{
    // TODO -- Step 4: enable the Timer1 clock (RCGCTIMER)
    // TODO -- Step 4: CFG = 0x0 (32-bit mode), TAMR = periodic mode
    // TODO -- Step 4: TAILR = BLINK_DEFAULT - 1
    // TODO -- Step 4: clear + enable the timeout interrupt, enable NVIC IRQ for TIMER1A
    // TODO -- Step 4: leave the timer itself disabled (CTL) -- 'A' starts it, in Step 5
}

void TIMER1A_Handler(void)
{
    // TODO -- Step 4: clear the timeout interrupt (ICR)
    // TODO -- Step 4: toggle the active color on/off on the RGB LED
}

void KeyPad_HandleKey(char key)
{
    switch (key) {
        // TODO -- Step 3: cases '1'-'7' set the LED color combination, '0' clears it
        //                 (see the Key Actions table)

        // TODO -- Step 5: case 'A' toggles blink on/off (enable/disable Timer1A via CTL)
        // TODO -- Step 5: case 'B' decreases the blink interval by BLINK_STEP,
        //                 floored at BLINK_MIN -- check before you subtract
        // TODO -- Step 5: case 'C' increases the blink interval by BLINK_STEP,
        //                 capped at BLINK_MAX -- check before you add
        // TODO -- Step 5: case 'D' resets the interval to BLINK_DEFAULT

        default:
            break;   // 8, 9, *, # -- see Extras
    }
}
