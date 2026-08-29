#include "ledstopwatch.h"
#include "lcd.h"

volatile uint32_t global_ms  = 0;
volatile uint32_t elapsed_cs = 0;
volatile uint8_t  running    = 0;

volatile uint8_t  pause_requested = 0;
volatile uint8_t  lap_requested   = 0;

uint32_t laps[MAX_LAPS];
uint8_t  lap_count = 0;

void Stopwatch_GPIO_Init(void)
{
    // TODO: enable the clock for GPIOF
    // TODO: unlock PF0 -- GPIOF->LOCK = 0x4C4F434B, then GPIOF->CR |= 0x01
    // TODO: PF0, PF4 as inputs -- DIR cleared, DEN, PUR (pull-up)
    // TODO: PF1 as output -- DIR, DEN
}

void SysTick_Handler(void)
{
    // TODO: increment global_ms
}

void Timer0_Init(void)
{
    // TODO: enable the Timer0 clock
    // TODO: CFG = 0x0 (32-bit mode), TAMR = periodic mode
    // TODO: TAILR = TICK_RELOAD - 1
    // TODO: clear + enable the timeout interrupt, enable NVIC IRQ for TIMER0A
}

void TIMER0A_Handler(void)
{
    // TODO: clear the timeout interrupt
    // TODO: if running, advance elapsed_cs by one
    // TODO: if running, toggle RUN_LED every BLINK_TICKS ticks; hold it off while paused
}

void GPIOF_ButtonInterrupt_Init(void)
{
    // TODO -- Step 3: configure PF0 and PF4 for a falling-edge interrupt + NVIC (GPIOF = IRQ 30)
}

void GPIOF_Handler(void)
{
    // TODO -- Step 3: for whichever of PF0/PF4 triggered, clear its interrupt
    //         flag, and -- if at least DEBOUNCE_MS has passed since that
    //         button's own last accepted press -- set pause_requested (PF4)
    //         or lap_requested (PF0)
}

void Stopwatch_Update(void)
{
    // TODO -- Step 3: if pause_requested, clear it and toggle `running`
    // TODO -- Step 3: if lap_requested, clear it, and -- while running,
    //         record elapsed_cs into laps[] (if there's room) and bump
    //         lap_count; while paused, reset elapsed_cs and lap_count to
    //         0 instead
}

void FormatTime(uint32_t cs, char *out)
{
    // TODO -- Step 4: write "MM:SS.cc" into out, null-terminated --
    //         integer math only, no sprintf, no floats
}

void Stopwatch_Display(void)
{
    // TODO -- Step 4: roughly 10 times a second, format elapsed_cs and
    //         write it to a fixed spot on the LCD's first line -- not on
    //         every call, and not by clearing the whole screen
}
