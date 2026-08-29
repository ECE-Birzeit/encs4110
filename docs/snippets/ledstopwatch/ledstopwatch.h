#ifndef LEDSTOPWATCH_H
#define LEDSTOPWATCH_H

#include "TM4C123.h"

// ---- Buttons: onboard SW1/SW2 (interrupt-driven) ----
#define BTN_PAUSE    0x10                  // PF4 (SW1) -- pause/resume
#define BTN_LAP      0x01                  // PF0 (SW2) -- lap while running, reset while paused -- commit-protected, see below

// ---- Running indicator ----
#define RUN_LED      0x02                  // PF1

#define SYS_CLOCK    50000000UL            // 50 MHz system clock
#define TICK_HZ      100UL                  // Timer0A fires 100x/sec
#define TICK_RELOAD  (SYS_CLOCK / TICK_HZ)  // -> 10 ms per tick = 1 centisecond
#define BLINK_TICKS  50UL                   // toggle RUN_LED every 50 ticks (0.5 s) while running

#define MAX_LAPS     8
#define DEBOUNCE_MS  25

extern volatile uint32_t global_ms;       // free-running millisecond counter (SysTick) -- independent of `running`
extern volatile uint32_t elapsed_cs;      // centiseconds elapsed, advances only while running (Timer0A)
extern volatile uint8_t  running;         // 0 = paused, 1 = running

extern volatile uint8_t  pause_requested; // set by GPIOF_Handler, cleared once serviced
extern volatile uint8_t  lap_requested;   // set by GPIOF_Handler, cleared once serviced

extern uint32_t laps[MAX_LAPS];           // recorded lap times, in centiseconds
extern uint8_t  lap_count;                // how many laps recorded so far

void Stopwatch_GPIO_Init(void);           // Step 1 -- clocks + DIR/DEN/PUR for PF0/PF1/PF4 (unlock PF0 first)
void SysTick_Handler(void);               // Step 2 -- advance global_ms

void Timer0_Init(void);                   // Step 2 -- TIMER0A, 32-bit periodic mode, TICK_RELOAD, NVIC IRQ 19
void TIMER0A_Handler(void);               // Step 2 -- clear the interrupt, advance elapsed_cs, blink RUN_LED while running

void GPIOF_ButtonInterrupt_Init(void);    // Step 3 -- edge interrupt config for PF0/PF4 + NVIC (GPIOF = IRQ 30)
void GPIOF_Handler(void);                 // Step 3 -- debounce (using global_ms), set pause_requested / lap_requested

void Stopwatch_Update(void);              // Step 3 -- call every main-loop iteration, acts on the request flags

void FormatTime(uint32_t cs, char *out);  // Step 4 -- cs -> "MM:SS.cc" -- integer math only, no sprintf/float
void Stopwatch_Display(void);             // Step 4 -- call every main-loop iteration, updates the LCD at a sane rate

#endif // LEDSTOPWATCH_H
