---
outline: deep
title: Assembly Project
description: Create a Keil uVision5 project for a generic ARM Cortex-M4, write an assembly program, and step through it in the simulator. Used for experiments 1 to 4.
prev:
  text: "Installing Keil and Tiva Support"
  link: /setup
next:
  text: "Tiva LaunchPad Project"
  link: /tiva-project
---

# Assembly Project

Experiments **1–4** are pure ARM assembly. They run in Keil's built-in **simulator**, so you do not need the LaunchPad. This guide builds a project from an empty folder, runs a tiny program, and shows you the debugger windows you will use in every lab.

::: info Before you start
You need Keil uVision5 and the **ARM::Cortex_DFP** pack. If you haven't installed them, do [Steps 1 and 2 of the setup guide](./setup) first.
:::

::: details The whole thing in 30 seconds (once you've done it before)
1. **Project → New µVision Project…** → new folder → name it → **Save**.
2. Device: **ARM → ARM Cortex M4 → ARMCM4** → **OK**.
3. Run-Time Environment: **tick nothing** → **OK**.
4. Right-click **Source Group 1** → **Add New Item…** → **Asm File (.s)** → `main.s`.
5. <kbd>Alt</kbd>+<kbd>F7</kbd> → **Debug** → **Use Simulator**, untick *Run to main()*.
6. <kbd>F7</kbd> to build, <kbd>Ctrl</kbd>+<kbd>F5</kbd> to debug, <kbd>F11</kbd> to step.
:::

## Step 1: Make a folder for the project

Keil creates several files per project, so give each experiment its own folder. For example:

```text
C:\ENCS4110\Exp1
```

::: warning Keep the path simple
Avoid spaces, Arabic letters and OneDrive-synced folders (such as *Desktop* or *Documents* when OneDrive is on). Long or unusual paths are the most common cause of strange build errors. `C:\ENCS4110\...` is safe.
:::

## Step 2: Create the project

1. Open **Keil uVision5**.
2. If a previous project opens automatically, close it with **Project → Close Project**.
3. Click **Project → New µVision Project…**.
4. Browse to your folder (`C:\ENCS4110\Exp1`), type a name such as `exp1` and click **Save**.

<Screenshot src="assembly/01-new-project-menu.png" alt="Project menu with New µVision Project highlighted">
uVision5 with no project open and the Project menu dropped down, "New µVision Project..." highlighted.
</Screenshot>

<Screenshot src="assembly/02-save-project.png" alt="Create New Project dialog saving exp1 in C:\ENCS4110\Exp1">
The "Create New Project" save dialog, inside C:\ENCS4110\Exp1, with "exp1" in the File name box.
</Screenshot>

## Step 3: Choose the ARMCM4 device

The **Select Device for Target** window opens right after you save.

1. Make sure the drop-down at the top-left says **Software Packs**.
2. Expand **ARM → ARM Cortex M4** and select **ARMCM4**.
3. Click **OK**.

<Screenshot src="assembly/03-select-armcm4.png" alt="Select Device for Target with ARM, ARM Cortex M4, ARMCM4 selected">
"Select Device for Target 'Target 1'..." window with the tree expanded to ARM → ARM Cortex M4 → ARMCM4 and ARMCM4 selected. The description pane on the right should mention the Cortex-M4.
</Screenshot>

::: tip ARMCM4 vs. ARMCM4_FP
Either works for these experiments. `ARMCM4_FP` also has the floating-point unit, which experiments 1–4 do not use. Pick plain **ARMCM4**.
:::

::: details I don't see "ARM Cortex M4" in the list
The **ARM::Cortex_DFP** pack is missing. Click **Cancel**, open **Project → Manage → Pack Installer…**, search for `ARMCM4`, click **Install** next to *ARM::Cortex_DFP*, then go back to **Project → Options for Target → Device** to choose it.
:::

## Step 4: Skip the Run-Time Environment

Next, the **Manage Run-Time Environment** window opens. It offers ready-made software such as a startup file.

**Leave every box unticked and click OK.**

<Screenshot src="assembly/04-rte-empty.png" alt="Manage Run-Time Environment with nothing selected">
The "Manage Run-Time Environment" window with CMSIS and Device collapsed and nothing ticked. Highlight the OK button.
</Screenshot>

::: info Why nothing?
In experiments 1–4 **your own program** provides the vector table (the `RESET` area with `__Vectors` and `Reset_Handler`). If you also tick **Device → Startup**, Keil adds a second vector table and the build fails with `Symbol __Vectors multiply defined`. You can reopen this window at any time from **Project → Manage → Run-Time Environment…**.
:::

## Step 5: Add an assembly source file

The **Project** panel on the left now shows **Target 1 → Source Group 1**, empty.

1. Right-click **Source Group 1** → **Add New Item to Group 'Source Group 1'…**.
2. Choose **Asm File (.s)**.
3. Name it `main.s` and click **Add**.

<Screenshot src="assembly/05-add-new-item-menu.png" alt="Right-click menu on Source Group 1 with Add New Item highlighted">
Project panel with Target 1 expanded, right-click menu open on "Source Group 1", "Add New Item to Group 'Source Group 1'..." highlighted.
</Screenshot>

<Screenshot src="assembly/06-add-asm-file.png" alt="Add New Item dialog with Asm File selected and main.s as the name">
"Add New Item to Group" dialog with "Asm File (.s)" selected and "main.s" in the Name box.
</Screenshot>

`main.s` opens in the editor. Paste this test program and press <kbd>Ctrl</kbd>+<kbd>S</kbd>:

```asm [main.s]
        AREA    RESET, CODE, READONLY
        EXPORT  __Vectors
__Vectors
        DCD     0x20001000          ; Initial stack pointer
        DCD     Reset_Handler       ; Reset vector: execution starts here

        AREA    MYCODE, CODE, READONLY
        ENTRY
        EXPORT  Reset_Handler
Reset_Handler
        MOV     R0, #5              ; R0 = 5
        MOV     R1, #7              ; R1 = 7
        ADDS    R2, R0, R1          ; R2 = R0 + R1 = 12, and update the flags
        SUBS    R3, R0, R0          ; R3 = 0, so the Z flag becomes 1
STOP    B       STOP                ; Loop here forever
        END
```

Every program in experiments 1–4 has this same shape:

| Part | What it does |
|------|--------------|
| `AREA RESET` | The **vector table**. The first word is the initial stack pointer; the second is the address of the first instruction to run. |
| `AREA MYCODE` + `ENTRY` | Your code. `Reset_Handler` is where the processor starts after reset. |
| `STOP B STOP` | An infinite loop. There is no operating system to "return" to, so a program must never run off the end. |
| `END` | Tells the assembler the file is finished. Anything after it is ignored. |

::: warning Indentation matters
In Keil assembly, **labels** (`__Vectors`, `Reset_Handler`, `STOP`) start in the **first column**. **Instructions and directives** (`AREA`, `MOV`, `DCD`…) must be **indented** by at least one space or tab. An unindented `MOV` is read as a label and causes errors.
:::

::: tip Using the lab snippets
The examples on the experiment pages are shown as `.asm`. Copy their contents into a `.s` file in your project, as above.
:::

## Step 6: Set the project options

Open **Project → Options for Target 'Target 1'…** (<kbd>Alt</kbd>+<kbd>F7</kbd>, or the magic-wand icon in the toolbar). Keil's defaults are fine everywhere except the **Debug** tab.

### Debug tab

1. On the **left** half, select **Use Simulator**.
2. Untick **Run to main()**. There is no `main` in an assembly program.
3. Click **OK**.

<Screenshot src="assembly/09-options-debug-simulator.png" alt="Options for Target, Debug tab, Use Simulator selected">
Options for Target, Debug tab, with "Use Simulator" selected on the left and "Run to main()" unticked. Highlight both.
</Screenshot>

## Step 7: Build

Click **Project → Build Target** (<kbd>F7</kbd>, or the icon with a downward arrow into a box). Watch the **Build Output** panel at the bottom. A good build ends with:

```text
".\Objects\exp1.axf" - 0 Error(s), 0 Warning(s).
```

<Screenshot src="assembly/10-build-output.png" alt="Build Output panel showing 0 Error(s), 0 Warning(s)">
uVision with main.s open and the Build Output panel at the bottom showing a successful build ending in "0 Error(s), 0 Warning(s)". Highlight the Build toolbar button and the final line.
</Screenshot>

If there are errors, double-click the first error line in Build Output: Keil jumps to the line that caused it. Always fix the **first** error first; later ones are often caused by it. See [Troubleshooting](#troubleshooting) for the usual suspects.

## Step 8: Run it in the debugger

Start the debugger with **Debug → Start/Stop Debug Session** (<kbd>Ctrl</kbd>+<kbd>F5</kbd>, or the magnifier-over-a-chip icon). Click **OK** on the *EVALUATION MODE* notice if it appears.

The layout changes. The yellow arrow in the editor marks the **next instruction to run**: the first `MOV` in `Reset_Handler`.

<Screenshot src="assembly/11-debug-session-overview.png" alt="The debug session layout with the Registers, Disassembly and editor windows labelled">
The full uVision window in a debug session, labelled: Registers window (left), Disassembly (top), editor with the yellow arrow on the first MOV, Command window (bottom left), and the debug toolbar (Reset, Run, Stop, Step, Step Over, Step Out).
</Screenshot>

### Step through the program

Press **Step** (<kbd>F11</kbd>) once per instruction and watch the **Registers** window. The registers that just changed are highlighted in blue.

1. After the first step, `R0` = `0x00000005`.
2. After the second, `R1` = `0x00000007`.
3. After `ADDS`, `R2` = `0x0000000C` (12 in decimal).
4. After `SUBS`, `R3` = `0x00000000`. Expand **xPSR**: the **Z** flag is now `1`.

<Screenshot src="assembly/12-registers-after-adds.png" alt="Registers window after ADDS, with R2 = 0x0000000C highlighted and xPSR expanded">
Registers window after stepping past SUBS: R0=5, R1=7, R2=0x0C, R3=0, and xPSR expanded showing the N, Z, C, V bits with Z = 1. Highlight the changed registers.
</Screenshot>

### The controls you'll use every lab

| Action | Key | What it does |
|--------|-----|--------------|
| Step | <kbd>F11</kbd> | Run one instruction. Enters subroutines (`BL`). |
| Step Over | <kbd>F10</kbd> | Run one instruction, but treat a `BL` call as a single step. |
| Step Out | <kbd>Ctrl</kbd>+<kbd>F11</kbd> | Run until the current subroutine returns. |
| Run | <kbd>F5</kbd> | Run until a breakpoint (or forever). |
| Stop | toolbar ⏹ | Pause a running program. |
| Reset | toolbar **RST** | Go back to the start of the program. |
| Breakpoint | <kbd>F9</kbd> | Toggle a breakpoint on the current line. Clicking the grey margin left of a line does the same. |
| Run to cursor | <kbd>Ctrl</kbd>+<kbd>F10</kbd> | Run until the line the cursor is on. |
| End debugging | <kbd>Ctrl</kbd>+<kbd>F5</kbd> | Leave the debugger and go back to editing. |

::: warning Edits need a rebuild
You cannot edit and continue. To change the program: stop debugging (<kbd>Ctrl</kbd>+<kbd>F5</kbd>), edit, rebuild (<kbd>F7</kbd>), and start debugging again.
:::

### Look at memory

From experiment 2 on you will load and store data. To see memory:

1. **View → Memory Windows → Memory 1**.
2. Type an address in the **Address** box, such as `0x20000000` (start of RAM), and press <kbd>Enter</kbd>.
3. Right-click inside the window to change how values are shown (for example **Unsigned → Int** to see 32-bit words instead of bytes).

You can also type a label from your program, such as `Reset_Handler`, in the address box.

<Screenshot src="assembly/13-memory-window.png" alt="Memory 1 window showing RAM at 0x20000000 as 32-bit words">
The Memory 1 window docked at the bottom with 0x20000000 in the Address box, right-click menu open on Unsigned → Int.
</Screenshot>

### Other useful views

- **View → Disassembly Window**: the actual machine instructions, with their addresses and encodings. Useful when the assembler expands a pseudo-instruction such as `LDR R0, =value` or `MOV32`.
- **View → Watch Windows → Watch 1**: type a register or label name to keep an eye on it.
- In the **Registers** window, **Internal → States** counts clock cycles and **Sec** shows elapsed time. Handy for timing loops in experiment 3.

## Step 9: Reuse the project for more examples

Each experiment has several examples, and each one has its own `RESET` vector table, so **only one of them can be in the build at a time**. Two good ways to handle this:

- **One project per experiment, one file per example.** Add each example as its own `.s` file. Right-click every file you are *not* running → **Options for File '…'** → untick **Include in Target Build**. The excluded files show a small red mark in the Project panel.
- **One project per example.** Repeat this guide in a new folder. Slower, but impossible to mix up.

<Screenshot src="assembly/14-exclude-from-build.png" alt="Options for File dialog with Include in Target Build unticked">
The "Options for File 'example2.s'" dialog with "Include in Target Build" unticked, and the Project panel behind it showing two .s files, one marked as excluded.
</Screenshot>

### What's in the project folder

| File/folder | What it is |
|-------------|-----------|
| `exp1.uvprojx` | **The project.** Double-click this to reopen the project later. |
| `exp1.uvoptx` | Your window layout and debug settings. |
| `main.s` | Your source code. |
| `Objects\`, `Listings\` | Build output. Safe to delete; Keil recreates them on the next build. |

To hand in a project, zip the whole folder. Delete `Objects\` first to keep it small.

## Troubleshooting

| Message or symptom | Cause and fix |
|--------------------|---------------|
| `Symbol __Vectors multiply defined` | Either **Device → Startup** is ticked in the Run-Time Environment ([Step 4](#step-4-skip-the-run-time-environment)), or two example files are both in the build ([Step 9](#step-9-reuse-the-project-for-more-examples)). |
| `error: unknown directive` / `unexpected token` on `AREA` | The assembler is set to GNU syntax. Set **Asm → Assembler Option** to **armasm (Arm Syntax)** (**Project → Options for Target → Asm**). |
| `A1163E: Unknown opcode` on a line that looks fine | The instruction isn't indented, so it was read as a label. Indent it. |
| `A1167E: Invalid line start` | A label is indented, or there is a stray character in the first column. Labels go in column one. |
| `L6236E: No section matches selector - no section to be FIRST/LAST` | The vector table is missing. Check your file starts with `AREA RESET` and that the file is in the build. |
| `*** error 65: access violation at 0x40...` while running | The program touched a peripheral address (`0x400xxxxx`). The generic ARMCM4 simulator has no peripherals; programs that use GPIO belong in a [Tiva LaunchPad Project](./tiva-project). |
| The file has no syntax colouring and isn't built | It was saved without the `.s` extension. Remove it from the project, rename it to `main.s`, and add it back with **Add Existing Files to Group…**. |
| Nothing happens when you press Step | You're not in a debug session. Press <kbd>Ctrl</kbd>+<kbd>F5</kbd> first. |
