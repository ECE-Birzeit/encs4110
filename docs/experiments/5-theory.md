---
outline: deep
lastUpdated: true
prev:
  text: "4. Microcontroller Architecture and GPIO Output - Procedure"
  link: /experiments/4-procedure
next:
  text: "5. GPIO Inputs and Interrupt Handling - Procedure"
  link: /experiments/5-procedure
---
# Experiment 5: GPIO Inputs and Interrupt Handling
::: info Complete Lab Manual
For the complete experiment including learning objectives, theoretical background, and detailed explanations, download the PDF manual:
 <a href="../manual/encs4110_5.pdf" target="_blank">Download Experiment 5 PDF</a>
:::

## Learning Objectives

After completing this experiment, you will be able to:

- Configure GPIO inputs with digital enable and internal pull resistors.
- Read switches via polling with software debouncing.
- Configure edge-triggered GPIO interrupts and write minimal ISRs.
- Enable and manage interrupts in the NVIC (IRQ mapping, ISER).
- Unlock and configure protected pins (PF0/NMI) for general-purpose use.

## Experiment Overview

This experiment extends GPIO functionality to inputs and introduces interrupt-driven programming. You will configure GPIO pins to read mechanical switches using both polling and interrupt approaches, implement software debouncing techniques, write interrupt service routines, and enable GPIO interrupts in the NVIC. By the end of this lab, you will understand how to implement both polling and interrupt-driven input handling, and write responsive embedded applications that react to external events in real time.



## Theoretical Background

### GPIO Input Configuration

In the previous experiment, GPIO pins were configured as outputs to control LEDs. In this experiment, we configure GPIO pins as **inputs** to read the state of external devices such as switches, buttons, and sensors.

When a GPIO pin is configured as an input, the microcontroller reads the voltage level on the pin and interprets it as a logic '0' (low, typically 0V) or logic '1' (high, typically 3.3V). The pin's digital input buffer must be enabled, and the pin must be connected to a defined voltage level to avoid floating states.

#### Input Pin Requirements

For reliable digital input operation, three conditions must be met:

1. **Direction**: The pin must be configured as an input (bit cleared in `GPIODIR`).
2. **Digital Enable**: The digital input buffer must be enabled (bit set in `GPIODEN`).
3. **Defined Logic Level**: The pin must be connected to a valid logic level (not floating).

If a pin is left floating (not connected to a defined voltage), it can pick up electrical noise and produce random or unstable readings. To prevent this, pins are typically connected to either power (VCC) or ground (GND) through a resistor, or the microcontroller's internal pull-up or pull-down resistors can be enabled.

### Pull-Up and Pull-Down Resistors

Digital inputs must not be floating: an undefined voltage can produce random logic reads. A pull resistor defines the idle (no-press) level of a switch input.

**Pull-Up (active-low)**: A **pull-up** connects the pin weakly to VCC, so the idle level is logic 1. Pressing the switch drives the pin to GND → logic 0.

**Pull-Down (active-high)**: A **pull-down** connects the pin weakly to GND, so the idle level is logic 0. Pressing the switch drives the pin to VCC → logic 1.

#### Internal Resistors (TM4C123)

TM4C123 GPIO pins include configurable **internal** pull resistors:

- `GPIOPUR` — Pull-Up Select (enable bit = pull-up on that pin)
- `GPIOPDR` — Pull-Down Select (enable bit = pull-down on that pin)

Do not enable both on the same pin. On the LaunchPad, SW1 (PF4) and SW2 (PF0) short to GND when pressed, so enable **pull-ups** (`GPIOPUR`) to make them active-low (read 0 when pressed, 1 when released).

### Switch Bouncing and Debouncing

When a mechanical switch or button is pressed or released, the metal contacts inside do not make or break contact cleanly. Instead, they **bounce** — rapidly making and breaking contact multiple times before settling into a stable state. This bouncing typically lasts 5-30 milliseconds.

For applications that poll the input at a slow rate or only care about the final state, bouncing may not be a problem. However, for interrupt-driven systems or applications that count button presses, bouncing can cause multiple false triggers.

#### Debouncing Techniques

Two approaches can eliminate or mitigate switch bouncing:

**Hardware Debouncing**: Add an RC (resistor-capacitor) filter circuit or a dedicated debouncing IC to the switch. The capacitor smooths out the voltage transitions, preventing bounces from reaching the microcontroller input.

**Software Debouncing**: After detecting a button press (or release), wait for a short period (typically 10-50 ms) to allow the bouncing to settle, then read the input again to confirm the state. This can be implemented with:

- **Blocking delay debouncing**: Insert a fixed delay (busy-wait loop) after detecting an edge, then verify the input state. This is simple but prevents the CPU from doing other work during the delay period.
- **Timer-based debouncing (non-blocking)**: Use a hardware timer to sample the input periodically without blocking the main program. Only register a press after multiple consistent readings across timer intervals.

### Reading GPIO Inputs: Polling vs. Interrupts

There are two fundamental approaches to reading digital inputs:

#### Polling (Continuous Checking)

In polling, the CPU continuously reads the input pin in a loop and checks whether the state has changed. This is simple to implement but has drawbacks:

- **CPU Usage**: The CPU is busy checking the input even when nothing is happening.
- **Latency**: The response time depends on how frequently the loop runs.
- **Power Consumption**: The CPU cannot enter low-power sleep modes while polling.

Polling is suitable for simple applications where the CPU has nothing else to do or when inputs change slowly.

#### Interrupts (Event-Driven Response)

In interrupt-driven input, the GPIO peripheral notifies the CPU when an input change occurs by generating an **interrupt request** (IRQ). The CPU immediately stops its current task, executes an **Interrupt Service Routine** (ISR), and then resumes normal operation.

- **Efficiency**: The CPU can do other work or sleep until an input event occurs.
- **Responsiveness**: The CPU reacts immediately to input changes.
- **Power Efficiency**: The CPU can remain in low-power modes and wake only when needed.

Interrupts are essential for responsive embedded systems and are the preferred method for handling asynchronous events.

### How Interrupts Work

Understanding the interrupt mechanism is essential for writing interrupt-driven programs.

#### Interrupt Lifecycle

When a GPIO pin configured for interrupts detects the specified event (e.g., a button press), the following sequence occurs:

1. **Event Detection**: The GPIO peripheral continuously monitors the pin according to its configuration (edge/level detection). When the configured condition is met (e.g., falling edge), the peripheral sets an internal flag.

2. **Interrupt Request (IRQ)**: If the interrupt is unmasked (enabled in `GPIOIM`), the GPIO module sends an interrupt request to the NVIC. The NVIC receives interrupt requests from all peripherals and manages their execution.

3. **CPU Response**: The NVIC checks if the interrupt is enabled (`NVIC_ISER`) and compares its priority with currently executing code. If the interrupt should be serviced:
   - The CPU finishes executing the current instruction.
   - The CPU automatically saves the current execution context (program counter, registers, status flags) onto the stack.
   - The CPU loads the address of the Interrupt Service Routine (ISR) from the vector table.
   - Execution jumps to the ISR.

4. **ISR Execution**: The ISR (e.g., `GPIOF_Handler()`) executes and must:
   - Identify which pin(s) caused the interrupt (read `GPIOMIS`).
   - Perform the required response (toggle LED, set flag, etc.).
   - Clear the interrupt flag (write to `GPIOICR`) — **critical step**.

5. **Return from Interrupt**: When the ISR completes (returns), the CPU:
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
        
        GPIOF->ICR |= (1 << 4);  // MUST clear flag!
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

Before a GPIO pin can be used as an input, it must first be configured for digital functionality. The following three steps are required for **all GPIO pins**:

1. **Enable the GPIO clock** via `SYSCTL_RCGCGPIO`.
2. **Select the direction** of the pin using `GPIO_DIR` (0 for input, 1 for output).
3. **Enable digital functionality** using `GPIO_DEN`.

Once a pin is set as an input, the logic level on the pin may be undefined if it is left floating (not driven by an external source). To ensure a stable logic level, internal resistors can be activated.

These resistors are controlled by two dedicated registers:

- `GPIO_PUR` — Enable internal pull-up resistor on the selected pin(s).
- `GPIO_PDR` — Enable internal pull-down resistor on the selected pin(s).

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

The `GPIOCR` register controls which pins can be reconfigured. After unlocking with `GPIOLOCK`, set the corresponding bit in `GPIOCR` to allow changes to that pin's configuration registers.

**Bit Values:**

- **0**: Pin configuration is locked (cannot be modified)
- **1**: Pin configuration is unlocked (can be modified)

#### Unlocking Procedure

To unlock a protected pin (such as PF0):

1. Write the unlock key `0x4C4F434B` to the `GPIOLOCK` register.
2. Set the corresponding bit in the `GPIOCR` (Commit Register) to allow changes.
3. Configure the pin normally (`GPIODIR`, `GPIODEN`, etc.).

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

**Bit Values:**

- **0**: Edge-sensitive (detects transitions) — typical for buttons
- **1**: Level-sensitive (detects steady state)

#### GPIOIBE — Interrupt Both Edges Register

**Register:** `GPIOIBE` — GPIO Interrupt Both Edges (Port F: `0x40025408`)

When edge-sensitive mode is selected (bit cleared in `GPIOIS`), this register determines whether interrupts occur on a single edge or both edges.

**Bit Values:**

- **0**: Single edge (rising or falling, defined by `GPIOIEV`)
- **1**: Both rising and falling edges trigger interrupts

#### GPIOIEV — Interrupt Event Register

**Register:** `GPIOIEV` — GPIO Interrupt Event (Port F: `0x4002540C`)

For single-edge interrupts (when the corresponding bit in `GPIOIBE` is 0), this register selects which edge triggers the interrupt.

**Bit Values:**

- **0**: Falling edge (high-to-low transition)
- **1**: Rising edge (low-to-high transition)

For switches with pull-ups (LaunchPad SW1/SW2), falling edge interrupts detect button presses.

#### GPIOIM — Interrupt Mask Register

**Register:** `GPIOIM` — GPIO Interrupt Mask (Port F: `0x40025410`)

The `GPIOIM` register enables or disables (masks/unmasks) interrupt generation for each pin. Setting a bit to '1' allows that pin to generate interrupts.

**Bit Values:**

- **0**: Interrupt disabled (masked) — default
- **1**: Interrupt enabled (unmasked)

#### GPIOMIS — Masked Interrupt Status Register

**Register:** `GPIOMIS` — GPIO Masked Interrupt Status (Port F: `0x40025418`)

This read-only register shows which pins have pending interrupts after masking. The ISR reads this register to determine which pin(s) caused the interrupt.

**Bit Values:**

- **0**: No interrupt pending for this pin
- **1**: Interrupt pending for this pin

#### GPIOICR — Interrupt Clear Register

**Register:** `GPIOICR` — GPIO Interrupt Clear (Port F: `0x4002541C`)

Writing '1' to a bit in this register clears the corresponding interrupt flag. This is **critical**: the ISR must clear the flag, or the interrupt will continuously re-trigger.

**Bit Values:**

- Write **1** to clear the interrupt flag for that pin
- Write **0** has no effect

### Configuration Workflow

GPIO interrupt configuration is divided into two parts, configuring the pin as an input and configuring the interrupt settings. The following steps summarize the process:

#### Step 1: Pin Configuration

Configure the GPIO pin as an input with proper electrical characteristics:

1. Enable the GPIO port clock (`RCGCGPIO`).
2. Unlock protected pins if needed (`GPIOLOCK`, `GPIOCR`).
3. Configure the pin as input (`GPIODIR` = 0).
4. Enable the digital function (`GPIODEN` = 1).
5. Enable pull-up or pull-down resistor (`GPIOPUR` or `GPIOPDR`).

#### Step 2: Interrupt Configuration

1. Configure interrupt sense (`GPIOIS` = 0 for edge-sensitive).
2. Select edge(s) (`GPIOIBE`/`GPIOIEV`).
3. **Clear any prior flags** (`GPIOICR` = 1 for the pin).
4. Unmask the pin in the GPIO (`GPIOIM` = 1).
5. Enable the IRQ in the NVIC.

### NVIC — Nested Vectored Interrupt Controller

The NVIC manages all peripheral interrupts on the Cortex-M4, deciding which ISR to execute based on enable bits, masking, and priority.

#### NVIC_ISER — Interrupt Set-Enable Registers

**Registers:** `NVIC_ISER[0..3]` (`0xE000E100-0xE000E10C`)

Each 32-bit `ISER` register enables up to 32 interrupt sources. Writing a '1' to a bit enables that IRQ; writing '0' has no effect.

| Register | IRQ Range | Address |
|----------|-----------|---------|
| `ISER[0]` | 0-31 | 0xE000E100 |
| `ISER[1]` | 32-63 | 0xE000E104 |
| `ISER[2]` | 64-95 | 0xE000E108 |
| `ISER[3]` | 96-127 | 0xE000E10C |

For the TM4C123, all GPIO interrupts (IRQ0-IRQ4) for Ports A-E, and Port F (IRQ30) are in `ISER[0]`.

**Example — Enabling Port F Manually:**

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

### Interrupt Service Routine (ISR)

When a GPIO interrupt occurs, the NVIC calls the corresponding ISR. For GPIO Port F, the ISR is named `GPIOF_Handler()`. The ISR must:

1. Determine which pin caused the interrupt (read `GPIOMIS`).
2. Perform the required action (e.g., toggle an LED).
3. Clear the interrupt flag (write to `GPIOICR`).

```c
void GPIOF_Handler(void) {
    if (GPIOF->MIS & (1 << 4)) {     // Check if PF4 caused interrupt
        // Respond to SW1 press
        GPIOF->ICR |= (1 << 4);      // Clear PF4 interrupt flag
    }
    if (GPIOF->MIS & (1 << 0)) {     // Check if PF0 caused interrupt
        // Respond to SW2 press
        GPIOF->ICR |= (1 << 0);      // Clear PF0 interrupt flag
    }
}
```