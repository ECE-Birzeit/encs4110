#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include "TM4C123.h"

// ---- Onboard RGB LED: traffic phase indicator ----
#define RED_LED     0x02                 // PF1
#define BLUE_LED    0x04                 // PF2
#define GREEN_LED   0x08                 // PF3
#define LED_MASK    (RED_LED | GREEN_LED | BLUE_LED)
// Stop = RED_LED, Go = GREEN_LED, Caution = RED_LED|GREEN_LED (yellow), Walk = LED_MASK (white)

// ---- Pedestrian request button (onboard SW1) ----
#define BTN_REQUEST 0x10                 // PF4

#define RED_SECONDS     5U
#define GREEN_SECONDS   5U
#define YELLOW_SECONDS  2U
#define WALK_SECONDS    5U

typedef enum { PHASE_RED, PHASE_GREEN, PHASE_YELLOW, PHASE_WALK } phase_t;

extern volatile uint32_t global_ms;        // free-running millisecond counter
extern volatile uint8_t  ped_request;      // set by the button ISR, cleared once serviced

extern phase_t  phase;                     // current phase
extern uint32_t phase_started_ms;          // global_ms value when the current phase began

void TrafficLight_GPIO_Init(void);         // Step 1
void SysTick_Handler(void);                // Step 2 -- advance global_ms

void TrafficLight_Run(void);               // Step 3 & 4 -- call every main-loop iteration

void GPIOF_ButtonInterrupt_Init(void);     // Step 4 -- edge interrupt config for PF4 + NVIC (GPIOF = IRQ 30)
void GPIOF_Handler(void);                  // Step 4 -- on a press, set ped_request

#endif // TRAFFICLIGHT_H
