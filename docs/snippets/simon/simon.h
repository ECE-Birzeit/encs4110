#ifndef SIMON_H
#define SIMON_H

#include "TM4C123.h"

// ---- Buttons: PC4-PC7 (inputs, pull-up, interrupt-driven) ----
#define BTN_MASK      0xF0               // PC4-PC7

// ---- LEDs: PD0-PD3 (outputs) -- LED i is paired with button i ----
#define LED_MASK      0x0F               // PD0-PD3

#define MAX_SEQUENCE  64                 // plenty of headroom for a growing sequence
#define DEBOUNCE_MS   25

extern volatile uint32_t global_ms;        // free-running millisecond counter
extern volatile uint8_t  pressed_button;   // set by GPIOC_Handler: 0-3, or 0xFF if none waiting
extern volatile uint32_t last_press_ms[4]; // per-button debounce timestamps

extern uint8_t sequence[MAX_SEQUENCE];   // the growing sequence, values 0-3
extern uint8_t sequence_len;             // how many steps are active right now

void Simon_GPIO_Init(void);              // Step 1
void SysTick_Handler(void);              // Step 2 -- advance global_ms

void Simon_ShowSequence(void);           // Step 3 -- play back sequence[0..sequence_len-1] on the LEDs

void GPIOC_ButtonInterrupt_Init(void);   // Step 4 -- edge interrupt config for PC4-7 + NVIC (GPIOC = IRQ 2)
void GPIOC_Handler(void);                // Step 4 -- debounce (using global_ms), record pressed_button

void Simon_Run(void);                    // Step 5 -- call every main-loop iteration, drives the whole game

#endif // SIMON_H
