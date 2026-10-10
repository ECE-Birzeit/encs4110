---
outline: deep
lastUpdated: true
---

# Stopwatch with LED Indicator — A Build Roadmap

A standalone assignment that pulls together **Experiment 6** (GPIO interrupts), **Experiment 7** (GPTM timing), and **Experiment 8** (character LCD) into one build: a stopwatch with start/stop/lap control, its elapsed time shown on an LCD instead of a serial terminal, with an LED that shows at a glance whether it's running.

**Target device:** TM4C123GH6PM.

## Introduction

### What Is This Build?

A hardware timer counts in fixed increments — here, hundredths of a second — for as long as the stopwatch is "running." Two buttons control it, each handled by a GPIO interrupt rather than being polled in the main loop: one toggles running/paused, the other records a lap while running or resets the clock while paused. An LED blinks while the stopwatch is running and goes dark while paused, so the state is readable without even looking at the display. The elapsed time and most recent lap are shown as text on a 16×2 LCD.

None of these pieces are new on their own — the timer pattern is the same one from Experiment 7, the LCD driver is the same one from Experiment 8. What's new is making all of them coexist correctly: the timer has to keep perfectly accurate time whether or not anyone touches a button, a button's interrupt handler has to mean something different depending on whether the clock is currently running or paused, and the display has to stay readable without needing every single tick.

### Hardware Setup

| Signal | Pin | Direction |
|---|---|---|
| Pause / Resume button (onboard SW1) | PF4 | Input, pull-up |
| Lap / Reset button (onboard SW2) | PF0 | Input, pull-up |
| Running indicator (onboard LED) | PF1 | Output |
| LCD (RS/E/D4–D7) | PB0, PB2, PB4–PB7 | Output |

No breadboard buttons needed — this reuses the LaunchPad's own SW1/SW2. PF0 is the one pin on this board that ships commit-protected: unlock it the same way Experiment 6 did, `GPIOF->LOCK = 0x4C4F434B;` then `GPIOF->CR = 0x01;`, before touching its `DIR`/`DEN`/`PUR`. Wire the LCD exactly as in Experiment 8 (4-bit mode, `RW` tied to ground for write-only).

### Behavior

| Input | Action |
|---|---|
| Pause/Resume button | Toggles running/paused |
| Lap button, while running | Records the current elapsed time as a lap, clock keeps going |
| Lap button, while paused | Resets both the elapsed time and the lap count to zero |

The onboard LED blinks at roughly 1 Hz while running, and stays off while paused — a runner glancing at the board should be able to tell which state it's in without reading the display at all.

## Build Steps

You get two files to fill in — `ledstopwatch.c`/`ledstopwatch.h` hold the timer, button, and display logic, `main.c` just calls into them (plus the LCD driver from Experiment 8, reused as-is). Every function is stubbed out with `// TODO` comments and builds cleanly as-is; no logic is implemented yet.

::: code-group
<<< @/snippets/ledstopwatch/main.c
<<< @/snippets/ledstopwatch/ledstopwatch.c
<<< @/snippets/ledstopwatch/ledstopwatch.h
<<< @/snippets/lcd/lcd.c
<<< @/snippets/lcd/lcd.h
:::

Fill it in through the four steps below, **in order** — don't start Step *N+1* until Step *N*'s check actually passes.

### Step 1: Configure the Ports

Fill in `Stopwatch_GPIO_Init`: clock for GPIOF, the PF0 unlock sequence, `DIR`/`DEN`/`PUR` for PF0 and PF4 (inputs), and `DIR`/`DEN` for PF1 (output).

```c
void Stopwatch_GPIO_Init(void)
{
    // TODO: enable the clock for GPIOF
    // TODO: unlock PF0 -- GPIOF->LOCK = 0x4C4F434B, then GPIOF->CR |= 0x01
    // TODO: PF0, PF4 as inputs -- DIR cleared, DEN, PUR (pull-up)
    // TODO: PF1 as output -- DIR, DEN
}
```

**Verify on the board/debugger:** breakpoint after `Stopwatch_GPIO_Init()` returns; watch `GPIOF->DIR`/`DEN`/`PUR` and confirm they match the comments. Hand-edit `GPIOF->DATA` bit 1 in the Watch window and confirm the LED lights; hold SW1 and SW2 in turn and confirm the matching `GPIOF->DATA` bit reads LOW while held.

### Step 2: Bring Up Both Timers

Fill in `SysTick_Handler` (advance `global_ms`), `Timer0_Init` (Timer0A, 32-bit periodic mode, `TICK_RELOAD` for a 100 Hz tick), and `TIMER0A_Handler` (clear the interrupt, advance `elapsed_cs` and blink the LED while `running`).

```c
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
```

`global_ms` and `elapsed_cs` look similar but serve different purposes: `global_ms` always advances, running or not, and Step 3's button interrupts lean on that. `elapsed_cs` is the actual stopwatch reading — it advances only while running, which is exactly why it isn't suitable for anything that also needs to work while paused.

**Verify in isolation, before Step 3 wires in the buttons:** confirm `global_ms` climbs steadily at roughly 1000/sec regardless of anything else. Then temporarily force `running = 1;` right after `Timer0_Init()` in `main.c`, watch `elapsed_cs` in the Watch window, and confirm it climbs by exactly 100 per second (time it against a clock) while the LED blinks at roughly 1 Hz. Delete the forced line once this checks out.

### Step 3: Handle the Buttons via Interrupt

Fill in `GPIOF_ButtonInterrupt_Init` (falling-edge interrupt on PF0 and PF4 + NVIC), `GPIOF_Handler` (debounce each button independently using `global_ms`, then set a request flag), and `Stopwatch_Update` (act on the request flags per the Behavior table above).

```c
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
```

**Something to think about:** `GPIOF_Handler` fires the instant a button is pressed, but `Stopwatch_Update` only runs on the next pass through the main loop. Why does that gap matter here, and why is it safer for the handler to just set a flag than to record the lap or toggle `running` directly from inside the interrupt?

**Verify:** press Pause/Resume and confirm `pause_requested` sets immediately, then `running` flips once `Stopwatch_Update` runs, with `elapsed_cs` visibly freezing and resuming right at the press (not a tick later) and the LED starting/stopping its blink to match. While running, press Lap a few times and confirm `laps[]`/`lap_count` fill in correctly without disturbing `elapsed_cs`. Pause, then press Lap again, and confirm `elapsed_cs` and `lap_count` both reset to 0. Confirm `GPIOF->MIS` clears immediately after each press and the program never hangs on one.

### Step 4: Display at a Sane Rate

Fill in `FormatTime` (integer math only, no `sprintf`/floats) and `Stopwatch_Display` (refresh the LCD roughly 10 times a second, not on every tick).

```c
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
```

> [!Warning] Don't clear and reprint the whole screen every update
> Calling `LCD_Clear()` on every refresh will visibly flash the display. Move the cursor straight to the numeric field with `LCD_SetCursor` and overwrite just that part of the line — same idea as recording a lap on line 2 without disturbing line 1.

**Verify:** confirm the time on the LCD updates smoothly, without flashing, and stays in sync with `elapsed_cs` in the Watch window. Record a lap and confirm it shows up immediately on the second line rather than waiting for the next scheduled refresh.

## Extras (Optional)

### Lap overflow

Decide — and justify — what happens on a 9th lap when `laps[]` only holds `MAX_LAPS` (8).

### Long-press reset

Add a second way to reset — holding Lap for 2+ seconds while paused — so the two behaviors (record vs. reset) don't rely purely on running/paused state.

### Blink rate reflects lap count

Make the LED blink faster each time a lap is recorded, as a rough visual count of how many laps have been taken this run.

## Verification Checklist

1. **Ports** — buttons and LED are correctly wired and configured (Step 1).
2. **Timers** — `global_ms` climbs steadily regardless of `running`; `elapsed_cs` advances at a steady 100/sec while forced on, and the LED blinks at roughly 1 Hz (Step 2).
3. **Buttons** — each button's interrupt reliably sets its request flag exactly once per press, with no hangs; Pause/Resume toggles `running` immediately, Lap records correctly while running and resets correctly while paused (Step 3).
4. **Display** — the LCD updates smoothly at a fixed cursor position with no flashing, and a recorded lap shows up immediately (Step 4).
5. **If you did the Extras** — confirm the behavior you chose works as intended.
