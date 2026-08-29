---
outline: deep
lastUpdated: true
---

# Traffic Light with Pedestrian Crossing — A Build Roadmap

A standalone assignment that pulls together **Experiment 4** (GPIO output), **Experiment 5** (GPIO interrupts), **Experiment 6** (SysTick timing), and **Experiment 9** (UART) into one build: a timer-driven traffic light cycle that a pedestrian can interrupt — safely, not immediately.

**Target device:** TM4C123GH6PM.

## Introduction

### What Is This Build?

A traffic light cycles through a fixed sequence — vehicles stop, vehicles go, vehicles caution — each phase lasting its own fixed duration. That part is a plain timer-driven state machine, nothing new. What makes this a real controller instead of just a light show is the pedestrian request button: a press can happen at *any* moment, including in the middle of the vehicle "go" phase, but it would be unsafe to switch straight to a walk signal right then. The request has to be remembered and only acted on once the cycle reaches a safe point.

That's the actual assignment: separating *when a request arrives* from *when it's safe to act on it*, which is a pattern that shows up constantly in real embedded systems (anywhere an interrupt can't be handled the instant it fires).

### Hardware Setup

| Signal | Pin | Direction |
|---|---|---|
| Traffic phase LED (onboard RGB) | PF1 / PF2 / PF3 | Output (Red / Blue / Green) |
| Pedestrian request button (onboard SW1) | PF4 | Input, pull-up |
| UART0 TX / RX | PA1 / PA0 | Same wiring as Experiment 9 |

Everything here is onboard — no breadboard needed. UART0 rides the same USB virtual COM port as Experiment 9; open a terminal at 115200 baud before resetting the board.

### Behavior

| Phase | LED | Duration |
|---|---|---|
| Stop (red) | Red | `RED_SECONDS` (5 s) |
| Go (green) | Green | `GREEN_SECONDS` (5 s) |
| Caution (yellow) | Red + Green | `YELLOW_SECONDS` (2 s) |
| Walk (pedestrian) | Red + Green + Blue (white) | `WALK_SECONDS` (5 s) |

Normal order: Stop → Go → Caution → Stop → … A pedestrian request (SW1) only ever inserts a Walk phase in place of the next Go phase — it never cuts a Go or Caution phase short. Once per second, print the current phase name and remaining seconds over UART; print immediately again on every transition.

## Build Steps

You get two files to fill in — `trafficlight.c`/`trafficlight.h` hold the phase and interrupt logic, `main.c` just calls into them (plus the UART driver from Experiment 9, reused as-is). Every function is stubbed out with `// TODO` comments and builds cleanly as-is; no logic is implemented yet.

::: code-group
<<< @/snippets/trafficlight/main.c
<<< @/snippets/trafficlight/trafficlight.c
<<< @/snippets/trafficlight/trafficlight.h
:::

Fill it in through the four steps below, **in order** — don't start Step *N+1* until Step *N*'s check actually passes.

### Step 1: Configure the Ports

Fill in `TrafficLight_GPIO_Init`: clock for GPIOF, `DIR`/`DEN` for PF1–PF3 (outputs), `DIR`/`DEN`/`PUR` for PF4 (input).

```c
void TrafficLight_GPIO_Init(void)
{
    // TODO: enable the clock for GPIOF
    // TODO: PF1-3 as outputs -- DIR, DEN
    // TODO: PF4 as an input -- DIR cleared, DEN, PUR (pull-up)
}
```

**Verify on the board/debugger:** breakpoint after `TrafficLight_GPIO_Init()` returns; watch `GPIOF->DIR`/`DEN`/`PUR` and confirm they match the comments. Hand-edit `GPIOF->DATA` in the Watch window and confirm each of Red/Green/Blue lights individually and in combination; hold SW1 and confirm `GPIOF->DATA` bit 4 reads LOW while held.

### Step 2: Add the Millisecond Clock

Fill in `SysTick_Handler`.

```c
void SysTick_Handler(void)
{
    // TODO: increment global_ms
}
```

**Verify:** add `global_ms` to the Watch window and confirm it climbs steadily at roughly 1000 counts per second.

### Step 3: Drive the Normal Cycle

Fill in `TrafficLight_Run`'s first part: cycle Stop → Go → Caution → Stop using `global_ms` and `phase_started_ms`, updating the RGB LED on every transition and printing the phase + remaining seconds over UART about once a second.

```c
void TrafficLight_Run(void)
{
    // TODO -- Step 3: drive the normal RED -> GREEN -> YELLOW -> RED cycle,
    //         each phase lasting its *_SECONDS duration, using global_ms
    //         and phase_started_ms -- update the RGB LED to match `phase`
    //         on every transition, and print the new phase + remaining
    //         seconds over UART about once a second
}
```

**Verify:** watch a terminal and confirm phases advance in the exact order Stop → Go → Caution → Stop, each lasting close to its stated duration (time it against a clock), and that the countdown reaches 0 right at each transition rather than skipping past it or printing twice for the same second.

### Step 4: Handle the Pedestrian Request

Fill in `GPIOF_ButtonInterrupt_Init` (falling-edge interrupt on PF4 + NVIC), `GPIOF_Handler` (set `ped_request` on a press), and extend `TrafficLight_Run` to insert a Walk phase in place of the next Go phase whenever `ped_request` is set.

```c
void GPIOF_ButtonInterrupt_Init(void)
{
    // TODO -- Step 4: configure PF4 for a falling-edge interrupt + NVIC (GPIOF = IRQ 30)
}

void GPIOF_Handler(void)
{
    // TODO -- Step 4: clear the interrupt, set ped_request = 1
}
```

**Something to think about:** a request can arrive while the light is already showing Go, already showing Caution, or even already showing Walk. In each of those cases, should the request be honored immediately, remembered for later, or ignored? And once a Walk phase has been granted, what stops the *same* request from triggering a second Walk phase right after?

**Verify:** press SW1 while the light shows Go, and confirm the current Go phase is *not* interrupted — it finishes normally. Confirm Walk plays at the very next Stop phase, then the cycle resumes at Go afterward. Finally, mash SW1 several times in a row (including during the Walk phase itself) and confirm it never queues up more than one Walk phase before returning to the normal cycle.

## Extras (Optional)

### Adjustable durations over UART

Let a typed command (e.g. `SET GREEN 8`) change one phase's duration at runtime, reusing the parsing skill from any UART-based work you've done.

### Flashing-caution mode

Add a maintenance mode — triggered however you choose — where the light just flashes yellow continuously instead of running the normal cycle.

### Countdown display for pedestrians

Print (or otherwise show) a dedicated "seconds left to cross" countdown that's visually distinct from the vehicle-phase countdown.

## Verification Checklist

1. **Ports** — the RGB LED and request button are correctly wired and configured (Step 1).
2. **Timing** — `global_ms` increments steadily (Step 2).
3. **Normal cycle** — Stop/Go/Caution advance in order, each lasting close to its stated duration, reported correctly over UART (Step 3).
4. **Pedestrian request** — a request never cuts short the current vehicle phase, Walk plays at the next safe point, and repeated presses never queue more than one Walk phase (Step 4).
5. **If you did the Extras** — confirm the behavior you chose works as intended.
