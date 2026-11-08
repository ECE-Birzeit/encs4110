---
outline: deep
title: Installing Keil and Tiva Support
description: Install Keil uVision5, the TM4C device pack, the Stellaris ICDI debugger add-on and the LaunchPad USB drivers on Windows.
short: Keil and Tiva Support
order: 1
---

# Installing Keil uVision5 and Tiva Support

This guide takes a fresh **Windows 10/11** PC to the point where Keil uVision5 can build programs for the **TM4C123GH6PM** and flash them onto the **Tiva C LaunchPad (EK-TM4C123GXL)**. Do it once, at home, before your first lab session. It takes about 30 minutes, most of it downloading.

::: info What you will install
| # | Item | Why you need it | Board needed? |
|---|------|-----------------|---------------|
| 1 | Keil MDK (uVision5) | The IDE, assembler, compiler and simulator | No |
| 2 | TM4C device pack | Teaches Keil about the TM4C123GH6PM chip | No |
| 3 | Stellaris ICDI add-on | Lets Keil talk to the LaunchPad's on-board debugger | No |
| 4 | Stellaris ICDI USB drivers | Lets Windows recognise the LaunchPad | **Yes** |

Experiments 1–4 run in the simulator and only need steps 1 and 2. Experiments 5–10 need all four.
:::

::: warning Windows only
Keil uVision5 runs on Windows. On a Mac, use a Windows virtual machine (Parallels, UTM or VMware Fusion) or a lab PC.
:::

## Step 1: Install Keil MDK

### Download the installer

1. Open the [Keil MDK download page](https://www.keil.com/demo/eval/arm.htm).
2. Fill in the form. Any valid email works; for *Company* write `Birzeit University`.
3. Click the **MDK5xx.EXE** link (the number changes with each release; take the newest).

### Run the installer

1. Right-click the downloaded `.EXE` and choose **Run as administrator**.
2. Accept the licence agreement and click **Next**.
3. Leave the **Destination Folders** at their defaults and **write the Core path down**. You will need it in Step 3. It is usually one of:
   ```text
   C:\Users\<YOUR_USERNAME>\AppData\Local\Keil_v5
   C:\Keil_v5
   ```
4. Enter your name and email on the *Customer Information* page and click **Next**.
5. When Windows asks whether to install the **ULINK** / **Arm** device drivers, click **Install**.
6. Click **Finish**.

### Let the Pack Installer finish

When the installer closes, uVision opens a window called **Pack Installer**. Its status bar (bottom right) says *Updating index…*. **Wait until it says *ONLINE* and the progress bar is gone** before you close it. This first update can take a few minutes.

<Screenshot src="setup/03-pack-installer-first-run.png" alt="Pack Installer on first launch, status bar reading ONLINE">
The Pack Installer window right after first install. Highlight the status bar at the bottom right showing ONLINE.
</Screenshot>

::: details About the licence
You do not need to buy anything. Without a licence key, Keil runs as **MDK-Lite**, which is limited to 32 KB of program code. Every program in this course is far smaller than that. You will see an *EVALUATION MODE* notice when you start debugging; just click **OK**.

If uVision asks you to activate a licence, you can activate the free **MDK-Community** edition from **File → License Management**, or simply close the dialog and carry on as MDK-Lite.
:::

## Step 2: Install the device packs

Keil uses *Device Family Packs* (DFPs) to describe each chip family. You need two:

- **Keil::TM4C_DFP** for the TM4C123GH6PM (experiments 5–10).
- **ARM::Cortex_DFP** for the generic Cortex-M4 simulator target (experiments 1–4). Most Keil installs include it already; check anyway.

### Install the TM4C pack

Make sure uVision is **closed**, then:

1. Download [Keil.TM4C_DFP.1.1.0.pack](https://keilpack.azureedge.net/pack/Keil.TM4C_DFP.1.1.0.pack).
2. **Double-click** the downloaded `.pack` file. The *Pack Unzip* window opens.
3. Click **Next** and wait for it to finish, then click **Finish**.

### Check both packs

1. Open uVision5.
2. Open **Project → Manage → Pack Installer…** (or click the Pack Installer icon in the toolbar, a green diamond with a box).
3. In the **Search** box on the left, type `TM4C123GH6PM` and select it. On the right, **Keil::TM4C_DFP** should say **Up to date** (or *Installed*).
4. Clear the search box, type `ARMCM4` and select it. **ARM::Cortex_DFP** should also say *Up to date*. If it says **Install**, click it.

<Screenshot src="setup/05-pack-installer-tm4c.png" alt="Pack Installer with TM4C123GH6PM selected and Keil::TM4C_DFP marked Up to date">
Pack Installer with "TM4C123GH6PM" typed in the left search box, the device selected, and Keil::TM4C_DFP on the right showing "Up to date". Highlight both.
</Screenshot>

### Generate the System Viewer file

The debugger's **System Viewer** (the live GPIO, UART, timer… register windows) reads a compiled `.SFR` file that sits next to the chip's `.svd` description. The TM4C pack ships without it and Keil often fails to build it, so in a debug session **Peripherals → System Viewer** shows only *Core Peripherals*. Build it yourself once with Keil's `SVDConv` tool:

1. Close uVision.
2. Open **Command Prompt** (press <kbd>Win</kbd>, type `cmd`, <kbd>Enter</kbd>).
3. Run these two commands:
   ```bat
   cd /d "%LOCALAPPDATA%\Arm\Packs\Keil\TM4C_DFP\1.1.0\SVD\TM4C123"
   "%LOCALAPPDATA%\Keil_v5\UV4\SVDConv.exe" TM4C123GH6PM.svd --generate=sfr
   ```
4. It should end with `Found 0 Error(s)` (warnings are fine). The folder now has a `TM4C123GH6PM.SFR` file next to `TM4C123GH6PM.svd`.

::: details Keil is in C:\Keil_v5, or "The system cannot find the path specified"
Use your own Core folder from Step 1 for `SVDConv.exe`, for example `"C:\Keil_v5\UV4\SVDConv.exe"`. On those installs the packs are often under the Core folder too, so the first line becomes `cd /d "C:\Keil_v5\ARM\PACK\Keil\TM4C_DFP\1.1.0\SVD\TM4C123"`. If the pack version isn't `1.1.0`, open the `TM4C_DFP` folder and use the version folder you find there.
:::

::: warning Redo this after reinstalling or updating the pack
The `.SFR` file lives inside the pack folder, so reinstalling Keil or installing a newer **TM4C_DFP** removes it. If System Viewer goes back to showing only *Core Peripherals*, run the two commands again.
:::

## Step 3: Install the Stellaris ICDI debugger add-on

The LaunchPad has a built-in debugger called the **Stellaris ICDI** (In-Circuit Debug Interface). Keil does not support it out of the box; Arm provides a small add-on for it.

### Download the add-on

1. Open the [Arm knowledge article for the Stellaris ICDI](https://developer.arm.com/documentation/ka002280/latest/).
2. Scroll down and download **`MDK_Stellaris_ICDI_AddOn.exe`**.

### Install it into your Keil folder

1. Close uVision.
2. Run `MDK_Stellaris_ICDI_AddOn.exe`.
3. When it asks for the destination folder, **change it to the Keil Core folder you wrote down in Step 1**, for example `C:\Users\<YOUR_USERNAME>\AppData\Local\Keil_v5`. The installer defaults to `C:\Keil_v5`; if that is not where Keil is, the add-on installs to the wrong place and *Stellaris ICDI* never appears in Keil.
4. Click **Next** until it finishes.

<Screenshot src="setup/06-icdi-addon-path.png" alt="Stellaris ICDI add-on installer with the destination folder set to the Keil folder">
The MDK_Stellaris_ICDI_AddOn installer's destination-folder page, with the path changed to the AppData\Local\Keil_v5 folder. Highlight the path box.
</Screenshot>

::: tip Can't remember the Keil folder?
Right-click the **Keil uVision5** shortcut on the desktop → **Open file location**. If that opens a Start Menu folder full of shortcuts, right-click the shortcut there → **Open file location** again. You land in `...\Keil_v5\UV4`; the folder you want is the parent, `...\Keil_v5`.
:::

You will select *Stellaris ICDI* inside each Tiva project; the [Tiva project guide](./tiva-project#debug-tab-select-the-stellaris-icdi) shows where.

## Step 4: Install the LaunchPad USB drivers

This step needs the board.

### Download the drivers

1. Open the [Stellaris ICDI drivers page on TI.com](https://www.ti.com/tool/STELLARIS_ICDI_DRIVERS).
2. Click **Download options** and download the ZIP file.
3. Extract it to a folder you will not delete, such as `C:\TivaDrivers`.

### Connect the board

1. Slide the **PWR SELECT** switch on the LaunchPad (top-left corner) to **DEBUG**.
2. Plug a micro-USB cable into the **DEBUG** USB port, which is the one at the **top** of the board next to the switch, not the one on the side.
3. The green **power LED** turns on. If the board still has TI's demo program, the RGB LED starts fading through colours.

::: warning Use a data cable
Some micro-USB cables (especially ones that came with power banks) only carry power. If the board lights up but Windows shows no new devices, try another cable.
:::

### Install the driver for each interface

1. Press <kbd>Win</kbd> + <kbd>X</kbd> and choose **Device Manager**.
2. Expand **Other devices**. You will see **In-Circuit Debug Interface** listed (usually three times) with a yellow warning triangle.
3. For **each** of them:
   1. Right-click → **Update driver**.
   2. Choose **Browse my computer for drivers**.
   3. Click **Browse…**, pick the folder you extracted (`C:\TivaDrivers`), and make sure **Include subfolders** is ticked.
   4. Click **Next**. If Windows warns about the publisher, choose **Install this driver software anyway**.

<Screenshot src="setup/08-device-manager-before.png" alt="Device Manager showing In-Circuit Debug Interface entries under Other devices">
Device Manager with "Other devices" expanded, showing the In-Circuit Debug Interface entries with yellow warning triangles.
</Screenshot>

<Screenshot src="setup/09-update-driver-browse.png" alt="Update Drivers dialog pointing at the extracted driver folder with Include subfolders ticked">
The "Browse for drivers on your computer" page of Update Drivers, with C:\TivaDrivers in the path box and "Include subfolders" ticked.
</Screenshot>

### Check that it worked

When all three are done, Device Manager should show no warnings for the board, and these entries:

- **Stellaris In-Circuit Debug Interface** → *Stellaris ICDI JTAG/SWD Interface* and *Stellaris ICDI DFU Device*
- **Ports (COM & LPT)** → *Stellaris Virtual Serial Port (COMx)*. Note the **COM number**; you will need it for the UART experiment.

<Screenshot src="setup/10-device-manager-after.png" alt="Device Manager after installing the drivers, with no warnings">
Device Manager after driver installation: the "Stellaris In-Circuit Debug Interface" category expanded with its two entries, and Ports (COM & LPT) showing "Stellaris Virtual Serial Port (COM?)".
</Screenshot>

## You're done. What next?

- For **experiments 1–4** (assembly in the simulator): [Assembly Project](./assembly-project).
- For **experiments 5–10** (the LaunchPad): [Tiva LaunchPad Project](./tiva-project). Its last step is a blinking-LED test that proves the whole chain works, so do it once before your first hardware lab.

## Troubleshooting

| Problem | Fix |
|---------|-----|
| Double-clicking the `.pack` file does nothing | Close uVision and try again. Or install it from inside Keil: Pack Installer → **File → Import…** → choose the `.pack` file. |
| `TM4C123GH6PM` does not show up in Pack Installer | The pack is not installed. Repeat [Step 2](#install-the-tm4c-pack). |
| *Stellaris ICDI* is not in the **Debug** tab's list | The add-on went into the wrong folder. Re-run it and set the path to your **Keil_v5** folder ([Step 3](#install-it-into-your-keil-folder)). |
| The board doesn't appear in Device Manager at all | Check the switch is on **DEBUG**, the cable is in the **top** port, and the cable carries data. Try another USB port on the PC. |
| Windows says *the best drivers are already installed* | You picked the wrong device, or Windows installed a generic driver. Right-click the device → **Uninstall device**, unplug and replug the board, then repeat [Step 4](#install-the-driver-for-each-interface). |

## Quick reference links

| Tool | Link |
|------|------|
| Keil MDK (uVision5) | [Download](https://www.keil.com/demo/eval/arm.htm) |
| TM4C Pack (Keil.TM4C_DFP.1.1.0.pack) | [Download](https://keilpack.azureedge.net/pack/Keil.TM4C_DFP.1.1.0.pack) |
| Stellaris ICDI Debugger Add-on | [Keil/Arm guide](https://developer.arm.com/documentation/ka002280/latest/) |
| Stellaris ICDI USB Drivers | [TI tool page](https://www.ti.com/tool/STELLARIS_ICDI_DRIVERS) |
