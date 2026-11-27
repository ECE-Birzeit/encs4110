---
title: "GPIO Inputs and Interrupt Handling"
description: "This experiment extends GPIO functionality to inputs and introduces interrupt-driven programming."
outline: deep
lastUpdated: true
manual: "/manual/encs4110_manual.pdf#nameddest=chapter.6"
datasheet: "/datasheets/tm4c123gh6pm.pdf#page=649"
current: "/experiments/exp6-gpio-interrupts"
short: "GPIO Interrupts"
search: false
prev: false
next: false
head:
  - - meta
    - name: robots
      content: noindex
---

# GPIO Inputs and Interrupt Handling

This experiment extends GPIO functionality to inputs and introduces interrupt-driven programming. You will configure GPIO pins to read mechanical switches using both polling and interrupt approaches, implement software debouncing techniques, write interrupt service routines, and enable GPIO interrupts in the NVIC. By the end of this lab, you will understand how to implement both polling and interrupt-driven input handling, and write responsive embedded applications that react to external events in real time.

::: warning Reworked version: in testing
This is the reworked version of [Experiment 6](/experiments/exp6-gpio-interrupts), shared for TAs to test. It has the same background, then one Activity built step by step and one Task. **Students: use the [current version](/experiments/exp6-gpio-interrupts)** until this one replaces it.
:::


## Objectives

- Configure the LaunchPad buttons as GPIO inputs with digital enable and internal pull-ups.
- Read a button by polling, see switch bounce for yourself, and debounce a polled button with a delay.
- Configure falling-edge GPIO interrupts (`IS`, `IBE`, `IEV`, `ICR`, `IM`) and write a short `GPIOF_Handler`.
- Enable Port F's interrupt (IRQ 30) in the NVIC, with `NVIC_EnableIRQ` and with `ISER` directly.
- Unlock PF0 (SW2) for general-purpose use.
- Handle two buttons in one handler: tell them apart with `MIS`, each with its own state and its own debounce.

## Background

### GPIO Input Configuration

In the previous experiment, GPIO pins were configured as outputs to control LEDs. In this experiment, we configure GPIO pins as **inputs** to read the state of external devices such as switches, buttons, and sensors.

When a GPIO pin is configured as an input, the microcontroller reads the voltage level on the pin and interprets it as a logic ’0’ (low, typically 0V) or logic ’1’ (high, typically 3.3V). The pin’s digital input buffer must be enabled, and the pin must be connected to a defined voltage level to avoid floating states.

#### Input Pin Requirements

For reliable digital input operation, three conditions must be met:

- **Direction**: The pin must be configured as an input (bit cleared in `GPIODIR`).
- **Digital Enable**: The digital input buffer must be enabled (bit set in `GPIODEN`).
- **Defined Logic Level**: The pin must be connected to a valid logic level (not floating).

If a pin is left floating (not connected to a defined voltage), it can pick up electrical noise and produce random or unstable readings. To prevent this, pins are typically connected to either power (`VCC`) or ground (`GND`) through a resistor, or the microcontroller’s internal pull-up or pull-down resistors can be enabled.

#### Pull-Up and Pull-Down Resistors

Digital inputs must not be left floating: an undefined voltage can produce random logic reads. A pull resistor defines the idle (no-press) level of a switch input.

##### Pull-Up (active-low)

A **pull-up** connects the pin weakly to $V_{CC}$, so the idle level is logic 1. Pressing the switch drives the pin to GND → logic 0.

##### Pull-Down (active-high)

A **pull-down** connects the pin weakly to GND, so the idle level is logic 0. Pressing the switch drives the pin to $V_{CC}$ → logic 1.

![Pull-up: a resistor from VCC to the input and a switch to ground, so the input reads 1 when released and 0 when pressed. Pull-down: a switch from VCC and a resistor to ground, so the input reads 0 when released and 1 when pressed](/images/exp6/pull-up-down.svg)

##### Internal Resistors (TM4C123)

TM4C123 GPIO pins include configurable **internal** pull resistors:

- `GPIOPUR` — Pull-Up Select (enable bit = pull-up on that pin)
- `GPIOPDR` — Pull-Down Select (enable bit = pull-down on that pin)

Do not enable both on the same pin. On the LaunchPad, SW1 (PF4) and SW2 (PF0) short to GND when pressed, so enable **pull-ups** (`GPIOPUR`) to make them active-low (read 0 when pressed, 1 when released).

### Switch Bouncing and Debouncing

When a mechanical switch or button is pressed or released, the metal contacts inside do not make or break contact cleanly. Instead, they **bounce** — rapidly making and breaking contact multiple times before settling into a stable state. This bouncing typically lasts 5-30 milliseconds.

![Switch bounce: the ideal signal changes once on press and once on release; the real one bounces for 5 to 20 ms each time](/images/manual/switch-bounce.svg)

*Mechanical switch bounce: press and release both bounce*

For applications that poll the input at a slow rate or only care about the final state, bouncing may not be a problem. However, for interrupt-driven systems or applications that count button presses, bouncing can cause multiple false triggers.

#### Debouncing Techniques

Two approaches can eliminate or mitigate switch bouncing:

##### Hardware Debouncing

Add an RC (resistor-capacitor) filter circuit or a dedicated debouncing IC to the switch. The capacitor smooths out the voltage transitions, preventing bounces from reaching the microcontroller input.

##### Software Debouncing

After detecting a button press (or release), wait for a short period (typically 10-50 ms) to allow the bouncing to settle, then read the input again to confirm the state. This can be implemented with:

- **Blocking delay debouncing**: Insert a fixed delay (busy-wait loop) after detecting an edge, then verify the input state. This is simple but prevents the CPU from doing other work during the delay period.
- **Timer-based debouncing (non-blocking)**: Use a hardware timer to sample the input periodically without blocking the main program. Only register a press after multiple consistent readings across timer intervals.

In the Activity, SW2 is read by polling and SW1 by interrupt, with no debouncing, so you can see the bounce for yourself. The Task's provided code debounces with delays, and [Experiment 7](/preview/exp7-timers) debounces with a timer.

### Reading GPIO Inputs: Polling vs. Interrupts

There are two fundamental approaches to reading digital inputs:

#### Polling (Continuous Checking)

In polling, the CPU continuously reads the input pin in a loop and checks whether the state has changed. This is simple to implement but has drawbacks:

- **CPU Usage**: The CPU is busy checking the input even when nothing is happening.
- **Latency**: The response time depends on how frequently the loop runs.
- **Power Consumption**: The CPU cannot enter low-power sleep modes while polling.

Polling is suitable for simple applications where the CPU has nothing else to do or when inputs change slowly.

```c
while (1) {
    if (button is pressed) {
        // Respond to button press
    }
}
```

#### Interrupts (Event-Driven Response)

In interrupt-driven input, the GPIO peripheral notifies the CPU when an input change occurs by generating an **interrupt request** (IRQ). The CPU immediately stops its current task, executes an **Interrupt Service Routine** (ISR), and then resumes normal operation.

- **Efficiency**: The CPU can do other work or sleep until an input event occurs.
- **Responsiveness**: The CPU reacts immediately to input changes.
- **Power Efficiency**: The CPU can remain in low-power modes and wake only when needed.

Interrupts are essential for responsive embedded systems and are the preferred method for handling asynchronous events.

```c
// Main loop can perform other tasks
while (1) {
    // Do other work or sleep
}

// ISR is called automatically when button is pressed
void ButtonISR(void) {
    // Respond to button press
    // Clear interrupt flag
}
```

### How Interrupts Work

Understanding the interrupt mechanism is essential for writing interrupt-driven programs. This section explains the complete lifecycle of an interrupt from trigger to completion.

#### Interrupt Lifecycle

When a GPIO pin configured for interrupts detects the specified event (e.g., a button press), the following sequence occurs:

##### 1. Event Detection

The GPIO peripheral continuously monitors the pin according to its configuration (edge/level detection). When the configured condition is met (e.g., falling edge), the peripheral sets an internal flag.

##### 2. Interrupt Request (IRQ)

If the interrupt is unmasked (enabled in `GPIOIM`), the GPIO module sends an interrupt request to the NVIC. The NVIC receives interrupt requests from all peripherals and manages their execution.

##### 3. CPU Response

The NVIC checks if the interrupt is enabled (`NVIC_ISER`) and compares its priority with currently executing code. If the interrupt should be serviced:

- The CPU finishes executing the current instruction.
- The CPU automatically saves the current execution context (program counter, registers, status flags) onto the stack.
- The CPU loads the address of the Interrupt Service Routine (ISR) from the vector table.
- Execution jumps to the ISR.

##### 4. ISR Execution

The ISR (e.g., `GPIOF_Handler()`) executes and must:

- Identify which pin(s) caused the interrupt (read `GPIOMIS`).
- Perform the required response (toggle LED, set flag, etc.).
- Clear the interrupt flag (write to `GPIOICR`) — **critical step**.

##### 5. Return from Interrupt

When the ISR completes (returns), the CPU:

- Automatically restores the saved context from the stack.
- Resumes execution from where it was interrupted.

This entire process happens in microseconds, making interrupts extremely responsive.

#### Why Clear the Interrupt Flag?

The interrupt flag remains set until explicitly cleared by software. If the ISR does not clear the flag by writing to `GPIOICR`, the NVIC will immediately re-trigger the interrupt as soon as the ISR returns, creating an infinite loop of interrupts that hangs the system.

```c
void GPIOF_Handler(void) {
    // Read which pin caused interrupt
    if (GPIOF->MIS & (1 << 4)) {
        // Handle PF4 interrupt
        // ...
        
        GPIOF->ICR = (1 << 4);   // MUST clear flag!
    }
}
```

#### Interrupt Advantages and Considerations

**Advantages:**
- **Efficient**: CPU can perform other tasks or sleep while waiting for events.
- **Responsive**: Immediate response to external events (microsecond latency).
- **Power-efficient**: CPU can remain in low-power modes between events.
- **Real-time**: Predictable timing for critical events.

**Considerations:**
- **Complexity**: More complex to implement and debug than polling.
- **Shared data**: ISRs and main code must carefully manage shared variables (use `volatile`).
- **ISR constraints**: ISRs should be short and fast — avoid delays and lengthy operations.
- **Debouncing**: Mechanical switch bouncing can trigger multiple interrupts; software or hardware debouncing is often necessary.

### GPIO Input Configuration Registers

Before a GPIO pin can be used as an input, it must first be configured for digital functionality. As discussed in the previous experiment, the following three steps are required for **all GPIO pins**:

1.  **Enable the GPIO clock** via `SYSCTL_RCGCGPIO`.
2.  **Select the direction** of the pin using `GPIO_DIR` (`0` for input, `1` for output).
3.  **Enable digital functionality** using `GPIO_DEN`.

Once a pin is set as an input, the logic level on the pin may be undefined if it is left floating (not driven by an external source). To ensure a stable logic level, internal resistors can be activated:

These resistors are controlled by two dedicated registers:

- `GPIO_PUR` — Enable internal pull-up resistor on the selected pin(s).
- `GPIO_PDR` — Enable internal pull-down resistor on the selected pin(s).

#### GPIOPUR — Pull-Up Resistor Enable

**Register:** `GPIOPUR` — GPIO Pull-Up Select (Port F: `0x40025510`)

The `GPIOPUR` register enables the internal pull-up resistor for each GPIO pin. When enabled, the pin is weakly pulled to logic ’1’ (3.3V) through a resistor (typically 13-40 k$\Omega$). This prevents floating inputs and is essential for switches connected to ground.

**GPIOPUR Register — Pull-Up Enable**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | PF7 |
| 6 | PF6 |
| 5 | PF5 |
| 4 | PF4 |
| 3 | PF3 |
| 2 | PF2 |
| 1 | PF1 |
| 0 | PF0 |

**Bit Values:**

- **0**: Pull-up disabled (default)
- **1**: Pull-up enabled

For the LaunchPad switches (SW1 on PF4, SW2 on PF0), pull-ups must be enabled.

#### GPIOPDR — Pull-Down Resistor Enable

**Register:** `GPIOPDR` — GPIO Pull-Down Select (Port F: `0x40025514`)

The `GPIOPDR` register enables the internal pull-down resistor for each GPIO pin. When enabled, the pin is weakly pulled to logic ’0’ (0V) through a resistor. This is used for switches connected to power (`VCC`).

**GPIOPDR Register — Pull-Down Enable**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | PF7 |
| 6 | PF6 |
| 5 | PF5 |
| 4 | PF4 |
| 3 | PF3 |
| 2 | PF2 |
| 1 | PF1 |
| 0 | PF0 |

**Bit Values:**

- **0**: Pull-down disabled (default)
- **1**: Pull-down enabled

**Note:** Pull-up and pull-down resistors should not be enabled simultaneously on the same pin.

### Unlocking Protected GPIO Pins

Some GPIO pins on the TM4C123 are **locked** by default to prevent accidental reconfiguration of critical functions such as JTAG/SWD (debug interface) and NMI (Non-Maskable Interrupt).

On Port F, pin **PF0** is locked because it is the NMI input by default. To use PF0 as a general-purpose GPIO input (for SW2 on the LaunchPad), it must be unlocked.

#### GPIOLOCK — Lock Register

**Register:** `GPIOLOCK` — GPIO Lock (Port F: `0x40025520`)

The `GPIOLOCK` register controls write access to the `GPIOCR` register. Writing the magic value `0x4C4F434B` ("LOCK" in ASCII) unlocks the port and allows modifications to the commit register.

**Values:**

- Write `0x4C4F434B`: Unlock the port (allow changes to `GPIOCR`)
- Write any other value: Lock the port (prevent changes to `GPIOCR`)
- Read: Returns `0x00000001` if locked, `0x00000000` if unlocked

#### GPIOCR — Commit Register

**Register:** `GPIOCR` — GPIO Commit (Port F: `0x40025524`)

The `GPIOCR` register controls which pins can be reconfigured. After unlocking with `GPIOLOCK`, set the corresponding bit in `GPIOCR` to allow changes to that pin’s configuration registers.

**GPIOCR Register — Commit Control**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | PF7 |
| 6 | PF6 |
| 5 | PF5 |
| 4 | PF4 |
| 3 | PF3 |
| 2 | PF2 |
| 1 | PF1 |
| 0 | PF0 |

**Bit Values:**

- **0**: Pin configuration is locked (cannot be modified)
- **1**: Pin configuration is unlocked (can be modified)

#### Unlocking Procedure

To unlock a protected pin (such as PF0):

1.  Write the unlock key `0x4C4F434B` to the `GPIOLOCK` register.
2.  Set the corresponding bit in the `GPIOCR` (Commit Register) to allow changes.
3.  Configure the pin normally (`GPIODIR`, `GPIODEN`, etc.).

```c
GPIOF->LOCK = 0x4C4F434B;    // Unlock Port F
GPIOF->CR   |= (1 << 0);     // Allow changes to PF0
```

After unlocking, PF0 can be configured like any other GPIO pin.

### GPIO Interrupt Configuration Registers

To configure a GPIO pin to generate interrupts, several registers must be configured. The process involves selecting the trigger condition (edge or level, rising or falling) and enabling the interrupt at both the GPIO module and the NVIC.

#### GPIOIS — Interrupt Sense Register

**Register:** `GPIOIS` — GPIO Interrupt Sense (Port F: `0x40025404`)

The `GPIOIS` register determines whether interrupts are triggered by signal **edges** (transitions) or **levels** (steady states).

**GPIOIS Register — Interrupt Sense**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | PF7 |
| 6 | PF6 |
| 5 | PF5 |
| 4 | PF4 |
| 3 | PF3 |
| 2 | PF2 |
| 1 | PF1 |
| 0 | PF0 |

**Bit Values:**

- **0**: Edge-sensitive (detects transitions) — typical for buttons
- **1**: Level-sensitive (detects steady state)

#### GPIOIBE — Interrupt Both Edges Register

**Register:** `GPIOIBE` — GPIO Interrupt Both Edges (Port F: `0x40025408`)

When edge-sensitive mode is selected (bit cleared in `GPIOIS`), this register determines whether interrupts occur on a single edge or both edges.

**GPIOIBE Register — Interrupt Both Edges**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | PF7 |
| 6 | PF6 |
| 5 | PF5 |
| 4 | PF4 |
| 3 | PF3 |
| 2 | PF2 |
| 1 | PF1 |
| 0 | PF0 |

**Bit Values:**

- **0**: Single edge (rising or falling, defined by `GPIOIEV`)
- **1**: Both rising and falling edges trigger interrupts

#### GPIOIEV — Interrupt Event Register

**Register:** `GPIOIEV` — GPIO Interrupt Event (Port F: `0x4002540C`)

For single-edge interrupts (when the corresponding bit in `GPIOIBE` is 0), this register selects which edge triggers the interrupt.

**GPIOIEV Register — Interrupt Event**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | PF7 |
| 6 | PF6 |
| 5 | PF5 |
| 4 | PF4 |
| 3 | PF3 |
| 2 | PF2 |
| 1 | PF1 |
| 0 | PF0 |

**Bit Values:**

- **0**: Falling edge (high-to-low transition)
- **1**: Rising edge (low-to-high transition)

For switches with pull-ups (LaunchPad SW1/SW2), falling edge interrupts detect button presses.

#### GPIOIM — Interrupt Mask Register

**Register:** `GPIOIM` — GPIO Interrupt Mask (Port F: `0x40025410`)

The `GPIOIM` register enables or disables (masks/unmasks) interrupt generation for each pin. Setting a bit to ’1’ allows that pin to generate interrupts.

**GPIOIM Register — Interrupt Mask**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | PF7 |
| 6 | PF6 |
| 5 | PF5 |
| 4 | PF4 |
| 3 | PF3 |
| 2 | PF2 |
| 1 | PF1 |
| 0 | PF0 |

**Bit Values:**

- **0**: Interrupt disabled (masked) — default
- **1**: Interrupt enabled (unmasked)

#### GPIOMIS — Masked Interrupt Status Register

**Register:** `GPIOMIS` — GPIO Masked Interrupt Status (Port F: `0x40025418`)

This read-only register shows which pins have pending interrupts after masking. The ISR reads this register to determine which pin(s) caused the interrupt.

**GPIOMIS Register — Masked Interrupt Status (Read-Only)**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | PF7 |
| 6 | PF6 |
| 5 | PF5 |
| 4 | PF4 |
| 3 | PF3 |
| 2 | PF2 |
| 1 | PF1 |
| 0 | PF0 |

**Bit Values:**

- **0**: No interrupt pending for this pin
- **1**: Interrupt pending for this pin

#### GPIOICR — Interrupt Clear Register

**Register:** `GPIOICR` — GPIO Interrupt Clear (Port F: `0x4002541C`)

Writing ’1’ to a bit in this register clears the corresponding interrupt flag. This is **critical**: the ISR must clear the flag, or the interrupt will continuously re-trigger.

**GPIOICR Register — Interrupt Clear**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | PF7 |
| 6 | PF6 |
| 5 | PF5 |
| 4 | PF4 |
| 3 | PF3 |
| 2 | PF2 |
| 1 | PF1 |
| 0 | PF0 |

**Bit Values:**

- Write **1** to clear the interrupt flag for that pin
- Write **0** has no effect

### Configuration Workflow

GPIO interrupt configuration is divided into two parts, configuring the pin as an input and configuring the interrupt settings. The following steps summarize the process:

##### 1. Pin Configuration

Configure the GPIO pin as an input with proper electrical characteristics:

1.  Enable the GPIO port clock (`RCGCGPIO`).
2.  Unlock protected pins if needed (`GPIOLOCK`, `GPIOCR`).
3.  Configure the pin as input (`GPIODIR` = 0).
4.  Enable the digital function (`GPIODEN` = 1).
5.  Enable pull-up or pull-down resistor (`GPIOPUR` or `GPIOPDR`).

##### 2. Interrupt Configuration

1.  Configure interrupt sense (`GPIOIS` = 0 for edge-sensitive).
2.  Select edge(s) (`GPIOIBE`/`GPIOIEV`).
3.  **Clear any prior flags** (`GPIOICR` = 1 for the pin).
4.  Unmask the pin in the GPIO (`GPIOIM` = 1).
5.  Enable the IRQ in the NVIC.

### NVIC — Nested Vectored Interrupt Controller

The NVIC manages all peripheral interrupts on the Cortex-M4, deciding which ISR to execute based on enable bits, masking, and priority.

#### NVIC_ISER — Interrupt Set-Enable Registers

**Registers:** `NVIC_ISER[0..3]` (`0xE000E100-0xE000E10C`)

Each 32-bit `ISER` register enables up to 32 interrupt sources. Writing a ’1’ to a bit enables that IRQ; writing ’0’ has no effect.

`NVIC_ISER[n]` is the CMSIS name for exactly the same registers Experiment 5 called `NVIC_ENx` (the TI datasheet naming) — `NVIC_ISER[0]` *is* `NVIC_EN0`, at the same address, with the same set-enable behavior. Likewise `NVIC_ICER[n]` (clear-enable) is `NVIC_DISx`. The two names aren’t different register sets, just different sources naming the same hardware.

| **Register** | **IRQ Range** | **Address** |
|:-------------|:--------------|:------------|
| `ISER[0]`    | 0-31          | 0xE000E100  |
| `ISER[1]`    | 32-63         | 0xE000E104  |
| `ISER[2]`    | 64-95         | 0xE000E108  |
| `ISER[3]`    | 96-127        | 0xE000E10C  |

*Interrupt ranges for NVIC_ISER registers*

For the TM4C123, all GPIO interrupts (IRQ0-IRQ4) for Ports A-E, and Port F (IRQ30) are in `ISER[0]`.

**Example — Enabling Port F Manually**

```c
NVIC->ISER[0] |= (1 << 30);   // IRQ30 - NVIC_EN0 bit 30
```

**CMSIS Alternative (Recommended):**

```c
NVIC_EnableIRQ(GPIOF_IRQn);
```

This function automatically computes the correct register and bit:

```c
void NVIC_EnableIRQ(IRQn_Type IRQn) {
    if (IRQn >= 0)
        NVIC->ISER[IRQn >> 5] |= (1u << (IRQn & 0x1F)); // /32 and %32
}
```

**Explanation:**
$$
\text{Register index} = \left\lfloor \frac{\text{IRQn}}{32} \right\rfloor, \qquad
\text{Bit position} = \text{IRQn} \bmod 32.
$$

Each `ISER` register enables 32 interrupts. Dividing the IRQ number by 32 selects the correct register index, and the remainder (`IRQn % 32`) identifies the bit position within that 32-bit register.

### Interrupt Service Routine (ISR)

When a GPIO interrupt occurs, the NVIC calls the corresponding ISR. For GPIO Port F, the ISR is named `GPIOF_Handler()`. The ISR must:

1.  Determine which pin caused the interrupt (read `GPIOMIS`).
2.  Perform the required action (e.g., toggle an LED).
3.  Clear the interrupt flag (write to `GPIOICR`).

```c
void GPIOF_Handler(void) {
    if (GPIOF->MIS & (1 << 4)) {     // Check if PF4 caused interrupt
        // Respond to SW1 press
        GPIOF->ICR = (1 << 4);       // Clear PF4 interrupt flag
    }
    if (GPIOF->MIS & (1 << 0)) {     // Check if PF0 caused interrupt
        // Respond to SW2 press
        GPIOF->ICR = (1 << 0);       // Clear PF0 interrupt flag
    }
}
```

## Activity

Use the two LaunchPad buttons on the RGB LED, each read the way that suits it:

- **SW1** steps through the seven colours of [Experiment 5](/preview/exp5-gpio-output). A press is an **event**: it happens at one moment, and must never be missed. SW1 uses an **interrupt**.
- **SW2** makes the LED go dark **while it's held**. Being held is a **state**: what matters is the level right now. SW2 is **polled** in the main loop.

Along the way, a counter shows you what switch bounce looks like.

::: info Setting up the project
This experiment runs on the TM4C123 LaunchPad. If you haven't made a project yet, follow the [Tiva LaunchPad Project](/setup/tiva-project) guide first, choosing **C**, and make sure its blinking-LED test works. All the code goes in that project's `main.c`.
:::

Nothing needs wiring: the buttons and the LED are on the board.

### Step 1: Add the definitions

Replace the contents of `main.c` with the include, the definitions, and the variables:

<<< @/snippets/preview/exp6/main.c#defines{c}

- Every Port F pin gets a name, with `LEDS` and `SWITCHES` for the groups, so the configuration below reads as "LEDs out, switches in".
- **`GPIO_UNLOCK`** is the key from [Unlocking Protected GPIO Pins](#unlocking-protected-gpio-pins): the ASCII codes of "LOCK".
- **`colors`** is the colour table from Experiment 5 as an array, in the order SW1 steps through it. It's `const`, so it stays in flash.
- **`color_index`** and **`presses`** are `volatile` because the interrupt handler changes them while `main` (and the debugger) read them.

::: info Choosing the types
- **Bit masks are unsigned:** `1U << 4`, not `1 << 4`. A plain `1` is a signed `int`, so `~SW1` would be a negative number, and `1 << 31` on a signed `int` is undefined behaviour in C.
- **`colors` is `uint8_t`:** each entry is a pattern for Port F's 8 pins, so a byte holds it. Writing it into the 32-bit `DATA` register simply fills the upper bits with 0, so the table doesn't need the register's width.
- **Counters and indices are `uint32_t`:** 32 bits is the Cortex-M4's own word size. A smaller type saves nothing for a single variable, and can cost an extra instruction each time the value has to be cut back to its size.
:::

### Step 2: Add the interrupt handler

Add `GPIOF_Handler` after the definitions:

<<< @/snippets/preview/exp6/main.c#handler{c}

- The startup file's vector table already has an entry called `GPIOF_Handler`. Writing a function with exactly that name puts your code in it (see [Interrupt Service Routine](#interrupt-service-routine-isr)).
- All eight Port F pins share this one handler, so it checks **`MIS`** to see which pin caused the interrupt. Only SW1 can here, but the check costs nothing and is needed as soon as a second pin interrupts (as in the [Task](#task)).
- The flag is cleared by writing a 1 to SW1's bit in **`ICR`**. A flag left set would call the handler again as soon as it returns, forever ([Why Clear the Interrupt Flag?](#why-clear-the-interrupt-flag)).
- The handler **only changes the state**, `color_index`. It doesn't touch the LED: the main loop owns the output. Keeping handlers this short is a habit worth having, because while one runs, the rest of the program waits.

### Step 3: Write `main`

Add `main` at the end of the file:

<<< @/snippets/preview/exp6/main.c#main{c}

It follows the [Configuration Workflow](#configuration-workflow):

1. **Clock, then unlock PF0.** SW2 is on PF0, which is locked at reset. Writing the key to `LOCK` lets `CR` be changed, and setting PF0's bit in `CR` lets its other registers be changed. PF4 (SW1) isn't locked.
2. **Pins.** LEDs are outputs and switches inputs. The switches connect to ground when pressed, so they need the pull-ups (`PUR`): released reads 1, pressed reads 0.
3. **SW1's interrupt.** Edge-sensitive (`IS`), one edge (`IBE`), the falling one (`IEV`): the moment of the press. `ICR` clears any edge seen while the pins were being set up, `IM` lets SW1 interrupt, and `NVIC_EnableIRQ` lets Port F's interrupt reach the CPU. SW2's bit stays clear in `IM`: it never interrupts.
4. **The loop polls SW2** and drives the LED: dark while SW2 reads 0 (held), the selected colour otherwise.

Build, flash, and run.

<Checklist id="activity" title="Activity checklist">

- Does the LED start red, and does each SW1 press move it one colour on: red, yellow, green, cyan, blue, magenta, white, then red again?
- Does the LED go dark while you hold SW2, and come back when you let go?
- Press SW1 while holding SW2, then let go: has the colour moved on?
- In the Watch window, does `presses` sometimes jump by 2 or 3 for one slow press?
- Can you say why SW1 needs an interrupt but SW2 can be polled?

</Checklist>

::: details More to try (optional)
| Try this | What you'll see |
| --- | --- |
| Read SW1 by polling too: comment out the `IM` line and add `if ((GPIOF->DATA & SW1) == 0) color_index = (color_index + 1) % 7;` to the loop | One press races through many colours: the loop sees the button held thousands of times. The interrupt's edge detection catches the *change* for you. |
| Debounce the polled SW1 with delay loops (code below) | Each press steps exactly once, but the loop does nothing else while it waits for the release. |
| Make the polled SW1 a toggle: `on ^= 1;` instead of the step, and show the colour only while `on` is 1 | Each press turns the LED off or back on. |
| Replace `NVIC_EnableIRQ(GPIOF_IRQn);` with `NVIC->ISER[0] = 1U << 30;` | It works the same: Port F is interrupt 30 (why below). With `1U << 29` instead, SW1 does nothing. |
| Comment out the `ICR` line in the handler | The first press freezes the board: the handler runs again and again. Pause the debugger to see where it's stuck. |

The delay-loop debounce. One pass of the loop is about 7 clock cycles, so 140000 passes at 50 MHz are about 20 ms:

```c
if ((GPIOF->DATA & SW1) == 0)                           // SW1 pressed
{
	color_index = (color_index + 1) % 7;
	for (volatile uint32_t i = 0; i < 140000; i++);     // About 20 ms: the press bounce
	while ((GPIOF->DATA & SW1) == 0);                   // Wait for the release
	for (volatile uint32_t i = 0; i < 140000; i++);     // About 20 ms: the release bounce
}
```

Why `1U << 30`: `GPIOF_IRQn` is 30 in `TM4C123.h`, and `ISER[0]` holds the enable bits of interrupts 0 to 31, so Port F is bit 30 of `ISER[0]` ([NVIC_ISER](#nvic-iser-—-interrupt-set-enable-registers)). Writing 0 to an `ISER` bit does nothing, so a plain `=` leaves the other interrupts enabled.
:::

::: details The complete program
<<< @/snippets/preview/exp6/main.c#full{c}
:::

::: details Common problems
| Symptom | Cause and fix |
| --- | --- |
| SW2 does nothing | PF0 isn't unlocked: `LOCK` must get the key **before** `CR` is written. |
| SW1 does nothing | `NVIC_EnableIRQ(GPIOF_IRQn)` or the `IM` line is missing, or the handler isn't named exactly `GPIOF_Handler`. |
| The first press freezes the board: the colour never changes again | The flag isn't cleared, so the handler runs again and again: write SW1's bit to `ICR` in the handler. |
| The LED is always dark, or presses fire by themselves | The pull-ups are missing: set `PUR` for both switches. |
| Colours skip sometimes | That's [switch bounce](#switch-bouncing-and-debouncing), expected in the Activity. The Task fixes it with a delay loop, and [Experiment 7](/preview/exp7-timers) with a timer. |
| LEDs stay off | `DIR` or `DEN` doesn't include the LED bits. |
:::

## Task

Give each button its own colours, both by interrupt: **SW1** steps through **red, blue, green**, and **SW2** through **yellow, magenta, cyan**. The LED shows the colour of the last press, and each button keeps **its own place** in its own sequence. One `GPIOF_Handler` serves both: it checks `MIS` to tell them apart ([Interrupt Service Routine](#interrupt-service-routine-isr)).

### The starter program

Start from this program. SW1 and the debouncing already work: SW2 is yours to add.

<<< @/snippets/preview/exp6/task.c{c}

**Test now, before writing anything:** the LED starts off, and SW1 steps red, blue, green, never skipping a colour. SW2 does nothing yet.

::: details How the provided part works
Each button is debounced on its own, with the delay loop from "More to try", so a press of one button is never lost while the other is held:

1. Each button has a **`ready`** flag, 1 at the start. The first falling edge of a press interrupts while the button is ready: `GPIOF_Handler` steps the colour and clears the flag. The bounces still interrupt, but the flag is 0, so they're ignored.
2. The main loop shows the colour, sees the cleared flag, and calls `debounce()` for that button.
3. `debounce()` waits about 20 ms, waits until the button is released (still showing the colour), and waits another 20 ms. Then it sets the `ready` flag again.

Don't change the provided part.
:::

:::: details What to write: the four TODOs
Each TODO is highlighted in the starter, exactly where its code goes. Write them in order: **1, 2, 3, 4**. TODO 2 must come before TODO 4: an interrupt nothing clears freezes the board.

::: details TODO 1: SW2's colours (`colors2`)
- Replace the three zeros with yellow, magenta, and cyan, each two LEDs joined with `|`: yellow is `RED_LED | GREEN_LED`. The others are in the Activity's `colors` table.

:::

::: details TODO 2: SW2 in the handler (`GPIOF_Handler`, after SW1's block)
- Copy SW1's whole `if (GPIOF->MIS & SW1)` block after it (not inside it), and change SW1's parts into SW2's: `SW2` in `MIS` and `ICR`, `ready[1]`, `colors2`, `index2`.

:::

::: details TODO 3: unlock PF0 (`main`, before the pins are set up)
- The three lines of the Activity's `main`: `GPIO_UNLOCK` to `LOCK`, SW2's bit in `CR`, then 0 to `LOCK`. Before `DIR`, `PUR` and `DEN`: until then, their writes to SW2's bit are ignored.

:::

::: details TODO 4: SW2's interrupt (`main`, after SW1's interrupt lines)
- The same five lines as SW1's, with `SW2`: `IS`, `IBE`, `IEV`, `ICR`, `IM`. `NVIC_EnableIRQ(GPIOF_IRQn)` needs no change: both buttons are on Port F, interrupt 30.

:::
::::

Build and flash, and add `index1`, `index2`, and `led` to the Watch window.

<Checklist id="task" title="Task checklist">

- Did you write SW2's handler block and its interrupt setup yourself?
- Does SW2 step yellow, magenta, cyan, then yellow again?
- SW1 (red), SW2 (yellow), SW1 again: is it blue, not red?
- Ten slow presses of either button: exactly one step each, and its index changes once per press?
- Hold SW1, press and release SW2: does the LED change to SW2's colour while SW1 is still held?

</Checklist>
