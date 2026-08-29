#ifndef KEYPAD_H
#define KEYPAD_H

#include "TM4C123.h"

// ---- Rows: PD0-PD3 (outputs) ----
#define ROW_MASK   0x0F                 // PD0-PD3

// ---- Columns: PC4-PC7 (inputs, pull-down) ----
#define COL1 0x10                       // PC4
#define COL2 0x20                       // PC5
#define COL3 0x40                       // PC6
#define COL4 0x80                       // PC7
#define COL_MASK (COL1 | COL2 | COL3 | COL4)

// ---- Onboard RGB LED: Port F ----
#define RED_LED   0x02                  // PF1
#define BLUE_LED  0x04                  // PF2
#define GREEN_LED 0x08                  // PF3
#define LED_MASK  (RED_LED | BLUE_LED | GREEN_LED)

// ---- Blink timer: Timer1A, 32-bit mode ----
#define TICKS_PER_SEC   50000000UL          // 50 MHz system clock
#define BLINK_DEFAULT   (2 * TICKS_PER_SEC) // 2 s
#define BLINK_STEP      (TICKS_PER_SEC / 4) // 0.25 s
#define BLINK_MIN       (TICKS_PER_SEC / 4) // 0.25 s floor -- fastest allowed
#define BLINK_MAX       (10 * TICKS_PER_SEC)// 10 s ceiling -- slowest allowed

extern const char keymap[4][4];         // TODO: fill in -- matches the wiring diagram

extern volatile uint32_t global_ms;     // free-running millisecond counter
extern volatile uint8_t  current_row;   // row currently driven HIGH (0-3)

void KeyPad_GPIO_Init(void);            // Step 1
void SysTick_Handler(void);             // Step 2
void KeyPad_Poll(void);                 // Step 2 -- call every main-loop iteration

void Blink_Timer_Init(void);            // Step 4
void TIMER1A_Handler(void);             // Step 4

void KeyPad_HandleKey(char key);        // Steps 3 & 5 -- action for `key`, see assignment table

#endif // KEYPAD_H
