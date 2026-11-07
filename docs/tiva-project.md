---
outline: deep
title: Tiva LaunchPad Project
description: Create a Keil uVision5 project for the TM4C123GH6PM, flash it onto the Tiva C LaunchPad over the Stellaris ICDI, and debug it on the board. Used for experiments 5 to 10.
prev:
  text: "Assembly Project"
  link: /assembly-project
next:
  text: "Experiment Navigator"
  link: /experiments/
---

# Tiva LaunchPad Project

Experiments **5–10** run on the real chip: the **TM4C123GH6PM** on the **Tiva C LaunchPad**. This guide builds a project from an empty folder, flashes a blinking-LED program onto the board, and shows you how to debug it live. Do it once before your first hardware lab; if the LED blinks, your whole setup is working.

::: info Before you start
You need **all four** steps of the [setup guide](./setup) done: Keil, the TM4C pack, the Stellaris ICDI add-on, and the USB drivers. Have the LaunchPad plugged into the **DEBUG** port with the switch on **DEBUG**.
:::

::: details The whole thing in 30 seconds (once you've done it before)
1. **Project → New µVision Project…** → new folder → name it → **Save**.
2. Device: **Texas Instruments → Tiva C Series → TM4C123x Series → TM4C123GH6PM** → **OK**.
3. Run-Time Environment: tick **CMSIS → CORE** and **Device → Startup** → **OK**.
4. Right-click **Source Group 1** → **Add New Item…** → `main.c` (or `main.s`).
5. <kbd>Alt</kbd>+<kbd>F7</kbd> → **Debug** → **Use:** *Stellaris ICDI*.
6. <kbd>F7</kbd> build, <kbd>F8</kbd> download, press **RESET** on the board.
:::

## How this differs from an assembly project

| | Assembly project | Tiva LaunchPad project |
|---|---|---|
| Device | ARMCM4 (generic) | **TM4C123GH6PM** |
| Vector table | You write it (`AREA RESET`) | **Provided** by `startup_TM4C123.s` |
| Your program starts at | `Reset_Handler` | `main` (C) or `__main` (assembly) |
| Peripherals (GPIO, timers…) | None | All of them, real |
| Runs on | Your PC | The board |

## Step 1: Make a folder for the project

Use a short path with no spaces, for example:

```text
C:\ENCS4110\Exp5
```

Avoid OneDrive-synced folders (*Desktop*, *Documents*) and Arabic folder names.

## Step 2: Create the project

1. Open **Keil uVision5**. If a project opens automatically, close it with **Project → Close Project**.
2. **Project → New µVision Project…**
3. Browse to your folder, type a name such as `exp5`, and click **Save**.

<Screenshot src="tiva/01-save-project.png" alt="Create New Project dialog saving exp5 in C:\ENCS4110\Exp5">
The "Create New Project" save dialog inside C:\ENCS4110\Exp5 with "exp5" as the file name.
</Screenshot>

## Step 3: Choose the TM4C123GH6PM

In the **Select Device for Target** window:

1. Make sure the top-left drop-down says **Software Packs**.
2. Type `TM4C123GH6PM` in the **Search** box, or expand **Texas Instruments → Tiva C Series → TM4C123x Series**.
3. Select **TM4C123GH6PM** and click **OK**.

<Screenshot src="tiva/02-select-tm4c123gh6pm.png" alt="Select Device for Target with TM4C123GH6PM selected">
"Select Device for Target" with "TM4C123GH6PM" in the search box and the device selected under Texas Instruments → Tiva C Series → TM4C123x Series. The description on the right should mention 256 KB Flash, 32 KB SRAM.
</Screenshot>

::: warning Check the exact name
It's **TM4C123G<u>H6PM</u>**: the chip printed on the LaunchPad. Similar-looking parts (TM4C123GH6PGE, TM4C1233H6PM…) build fine but have different pins and memory, and things go wrong later in confusing ways.
:::

If the Texas Instruments entry is missing, the TM4C pack isn't installed: see [setup Step 2](./setup#step-2-install-the-device-packs).

## Step 4: Add the startup files

The **Manage Run-Time Environment** window opens. This time you **do** want Keil's ready-made files:

1. Expand **CMSIS** and tick **CORE**.
2. Expand **Device** and tick **Startup**.
3. The **Sel.** column turns green. If any row is yellow or orange, click **Resolve** at the bottom-left.
4. Click **OK**.

<Screenshot src="tiva/03-rte-core-startup.png" alt="Manage Run-Time Environment with CMSIS CORE and Device Startup ticked">
"Manage Run-Time Environment" with CMSIS → CORE and Device → Startup ticked and both rows green. Highlight the two checkboxes and the OK button.
</Screenshot>

The **Project** panel now has a **Device** group containing two files Keil manages for you:

| File | What it does |
|------|--------------|
| `startup_TM4C123.s` | The **vector table** (stack pointer, reset handler, and an entry for every interrupt) plus the reset code. After reset it calls `SystemInit`, then jumps to your program. In experiment 6 you will name your interrupt handlers to match the entries in this file, e.g. `GPIOF_Handler`. |
| `system_TM4C123.c` | `SystemInit()`: sets up the clock before your code runs. |

<Screenshot src="tiva/04-project-tree-device.png" alt="Project panel showing the Device group with startup_TM4C123.s and system_TM4C123.c">
Project panel with Target 1 → Source Group 1 and Target 1 → Device expanded, showing startup_TM4C123.s and system_TM4C123.c with the green diamond icon.
</Screenshot>

::: tip Don't edit these directly
The small key icon means the file is managed by the pack. To look up an interrupt handler's name, open `startup_TM4C123.s` and read the vector table, but write your handlers in your own file.
:::

## Step 5: Add your program

Right-click **Source Group 1** → **Add New Item to Group 'Source Group 1'…**, then:

- For a **C** program (experiments 6–10): choose **C File (.c)**, name it `main.c`.
- For an **assembly** program (experiment 5): choose **Asm File (.s)**, name it `main.s`.

<Screenshot src="tiva/05-add-main-c.png" alt="Add New Item dialog with C File selected and main.c as the name">
"Add New Item to Group 'Source Group 1'" dialog with "C File (.c)" selected and "main.c" as the name.
</Screenshot>

Paste one of these test programs and save with <kbd>Ctrl</kbd>+<kbd>S</kbd>. Both drive the red LED on **PF1**:

::: code-group

```c [main.c]
#include "TM4C123.h"                 // Register names for the TM4C123 (from the pack)

#define RED_LED 0x02                 // PF1

int main(void) {
    volatile unsigned int i;         // volatile: keep the delay loop

    SYSCTL->RCGCGPIO |= (1 << 5);    // Turn on the clock to Port F
    GPIOF->DIR |= RED_LED;           // PF1 is an output
    GPIOF->DEN |= RED_LED;           // PF1 is a digital pin

    while (1) {
        GPIOF->DATA ^= RED_LED;      // Toggle the red LED
        for (i = 0; i < 1000000; i++) {}  // Wait a bit
    }
}
```

```asm [main.s]
        AREA    MYCODE, CODE, READONLY
        THUMB
        EXPORT  __main              ; The startup file jumps here

__main  PROC
        LDR     R1, =0x400FE608     ; RCGCGPIO
        LDR     R0, [R1]
        ORR     R0, R0, #0x20       ; Turn on the clock to Port F
        STR     R0, [R1]
        NOP                         ; Give the clock time to start
        NOP

        LDR     R1, =0x40025400     ; GPIOF DIR
        LDR     R0, [R1]
        ORR     R0, R0, #0x02       ; PF1 is an output
        STR     R0, [R1]

        LDR     R1, =0x4002551C     ; GPIOF DEN
        LDR     R0, [R1]
        ORR     R0, R0, #0x02       ; PF1 is a digital pin
        STR     R0, [R1]

        LDR     R1, =0x40025008     ; GPIOF DATA, masked to PF1 only
        MOV     R0, #0x02
        STR     R0, [R1]            ; Red LED on
STOP    B       STOP
        ENDP
        END
```

:::

::: info `main` or `__main`?
The startup file always jumps to a symbol called `__main` (two underscores).

- In **C**, `__main` is supplied by the C library: it sets up your global variables and then calls **your `main()`**. So write `int main(void)`.
- In **assembly** there's no C library, so **you** provide `__main` and `EXPORT` it, as above.

A project has exactly one of these. Two files both defining `main` (or `__main`) will not link.
:::

## Step 6: Set the project options

Open **Project → Options for Target 'Target 1'…** (<kbd>Alt</kbd>+<kbd>F7</kbd>, or the magic-wand icon). Keil's defaults are fine everywhere except the **Debug** tab, where you tell it to use the board instead of the simulator.

### Debug tab: select the Stellaris ICDI

On the **Debug** tab:

1. On the **right** half, select **Use:** and pick **Stellaris ICDI** from the drop-down.
2. Make sure **Load Application at Startup** and **Run to main()** are ticked. (For an assembly program, untick *Run to main()*.)
3. Click **OK**.

<Screenshot src="tiva/08-options-debug-icdi.png" alt="Options for Target, Debug tab with Stellaris ICDI selected on the right">
Options for Target, Debug tab, with the right-hand "Use:" radio selected and "Stellaris ICDI" in the drop-down; "Load Application at Startup" and "Run to main()" ticked. Highlight the drop-down.
</Screenshot>

::: details "Stellaris ICDI" isn't in the list
The ICDI add-on is missing or went into the wrong folder. Re-do [setup Step 3](./setup#step-3-install-the-stellaris-icdi-debugger-add-on), restart Keil, and check again.
:::

::: tip Set it once, copy it after
These options are saved in the project. For the next experiment, you can copy the whole project folder, rename it, and replace `main.c`, instead of starting over.
:::

## Step 7: Build

**Project → Build Target** (<kbd>F7</kbd>). The Build Output should end with:

```text
".\Objects\exp5.axf" - 0 Error(s), 0 Warning(s).
```

The first build of a project also compiles `system_TM4C123.c` and assembles the startup file, so it takes a little longer.

<Screenshot src="tiva/10-build-output.png" alt="Build Output showing the startup files compiled and 0 Error(s)">
Build Output after the first build of the Tiva project: lines for compiling main.c, assembling startup_TM4C123.s, compiling system_TM4C123.c, linking, and "0 Error(s), 0 Warning(s)".
</Screenshot>

## Step 8: Download to the board

**Flash → Download** (<kbd>F8</kbd>, or the *LOAD* icon with a downward arrow). Keil erases the chip, writes your program, and checks it. The Build Output shows:

```text
Erase Done.
Programming Done.
Verify OK.
```

<Screenshot src="tiva/11-download-output.png" alt="Build Output after Flash Download showing Erase Done, Programming Done, Verify OK">
Build Output after pressing Download (F8): "Erase Done. Programming Done. Verify OK." Highlight the LOAD toolbar button.
</Screenshot>

Now press the **RESET** button on the board (next to the DEBUG USB port). **The red LED should start blinking** (or, for the assembly version, stay on).

The program stays in flash: unplug and replug the board and it runs again.

## Step 9: Debug on the board

**Debug → Start/Stop Debug Session** (<kbd>Ctrl</kbd>+<kbd>F5</kbd>). Because of the options you set, Keil flashes the latest build and stops at the first line of `main`. Everything from the [Assembly Project's debug controls](./assembly-project#the-controls-you-ll-use-every-lab) works the same, except now the code runs on the real chip:

- Press <kbd>F5</kbd> (**Run**) and the LED blinks. Press **Stop** and it freezes mid-blink.
- Put a breakpoint (<kbd>F9</kbd>) on the `GPIOF->DATA ^= RED_LED;` line, then press <kbd>F5</kbd> repeatedly: the LED changes once per press.

### Watch the hardware registers

**Peripherals → System Viewer → GPIOF** opens a live view of every Port F register. Step over the `DIR` and `DEN` lines and watch bit 1 flip. You can also click a bit to change it and see the LED react.

::: warning System Viewer only shows "Core Peripherals"?
The pack's System Viewer file hasn't been generated. Stop debugging and follow [Generate the System Viewer file](./setup#generate-the-system-viewer-file) in the setup guide; it takes two commands.
:::

<Screenshot src="tiva/13-system-viewer-gpiof.png" alt="System Viewer window showing GPIOF registers with DATA bit 1 set">
Debug session on the board with Peripherals → System Viewer → GPIOF open, showing DIR, DEN and DATA with bit 1 set. The editor shows the yellow arrow inside the while loop.
</Screenshot>

### Watch your variables

**View → Watch Windows → Watch 1**, then double-click **&lt;Enter expression&gt;** and type a variable name (such as `i`). In the experiments you will use this to read ADC results (experiment 9) and counters. Right-click a value → untick **Hexadecimal Display** to see it in decimal.

<Screenshot src="tiva/14-watch-window.png" alt="Watch 1 window showing a variable's value">
Watch 1 window docked at the bottom showing the variable "i" with its current value, right-click menu open with "Hexadecimal Display".
</Screenshot>

::: warning Stop debugging before re-flashing
End the debug session (<kbd>Ctrl</kbd>+<kbd>F5</kbd>) before editing and pressing <kbd>F8</kbd>. While the debugger is connected, Keil can't download separately.
:::

## Troubleshooting

| Message or symptom | Cause and fix |
|--------------------|---------------|
| `cannot open source input file "TM4C123.h"` | The device isn't **TM4C123GH6PM**, or **CMSIS → CORE** / **Device → Startup** aren't ticked. Check **Options → Device** and **Project → Manage → Run-Time Environment**. |
| `Undefined symbol main (referred from __rtentry2.o)` | Your C file has no `main()`, it's misspelt, or the file isn't in the build. |
| `Undefined symbol __main (referred from startup_TM4C123.o)` | Assembly project: your file doesn't `EXPORT __main`, or the label isn't spelt `__main` (two underscores). |
| `Symbol main multiply defined` | Two files in Source Group 1 each have a `main`. Remove one, or right-click it → **Options for File** → untick **Include in Target Build**. |
| `No ICDI device was found` / `Flash Download failed - Target DLL has been cancelled` | Keil can't see the board. Check the **DEBUG** port and switch, try another cable and USB port, and check Device Manager shows the [Stellaris entries](./setup#check-that-it-worked). Close any other program using the board (another Keil window, a terminal on the COM port). |
| `Flash Download failed - "Cortex-M4"` | The flash driver is wrong. In **Options for Target → Utilities**, tick **Use Debug Driver** (or pick **Stellaris ICDI**). |
| Download says *Verify OK* but nothing happens | Press **RESET** on the board. If still nothing, check you are toggling the right pin (red LED is PF1, `0x02`). |
| The LED never blinks, but stepping works | The delay loop was optimised away. Declare the loop counter `volatile`, or set **Options for Target → C/C++ (AC6) → Optimization** to **-O0**. |
| `L6050U: The code size of this image exceeds the maximum allowed` | You hit MDK-Lite's 32 KB limit, usually by using `printf` or floating-point maths heavily. Remove what you don't need. |
| **Peripherals → System Viewer** lists only *Core Peripherals* (no GPIOF, UART…) | The `.SFR` file for the chip is missing. Generate it with `SVDConv`: see [setup](./setup#generate-the-system-viewer-file). |
| Watch window shows `<not in scope>` | The program is stopped outside the function that owns the variable, or optimisation is above `-O0`. Make the variable global or step into its function. |
