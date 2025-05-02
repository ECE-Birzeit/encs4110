# 🚀 Setting Up Keil uVision5 for TM4C123GH6PM (Tiva C LaunchPad)

This guide walks you through installing everything needed to program the **TM4C123GH6PM** microcontroller using **Keil uVision5** on **Windows 10/11**.


##  Step 1: Install the TM4C Device Pack

Keil uses "Device Family Packs" (DFPs) to support different microcontrollers. You'll need the **TM4C DFP** to build and flash projects for the TM4C123GH6PM.

### 🔗 Download the TM4C DFP

- Click here to download the official pack:[Keil.TM4C_DFP.1.1.0.pack](https://keilpack.azureedge.net/pack/Keil.TM4C_DFP.1.1.0.pack)

### 🛠️ Install the Pack

1. After downloading, **double-click the `.pack` file**.
2. Keil's Pack Installer will open and install the pack automatically.
3. You can verify it by opening **Pack Installer** inside Keil (`Project → Manage → Pack Installer`) and searching for `TM4C123GH6PM`. It should appear as **installed**.



##  Step 2: Install Stellaris ICDI Debugger Support

Keil does **not** include support for the Stellaris ICDI debugger by default — you have to install it manually.

### 🔗 Download the Debugger Add-On

- 👉 [Official Keil Stellaris ICDI Debugger Guide](https://developer.arm.com/documentation/ka002280/latest/)

Scroll down the page and download the **`MDK_Stellaris_ICDI_AddOn.exe`** file.

### 🛠️ Install the Add-On

1. Run `MDK_Stellaris_ICDI_AddOn.exe`.
2. When asked for an install path, **change it to your Keil installation folder**, usually:
```bash
C:\Users\{YOUR_USERNAME}\AppData\Local\Keil_v5
```

Replace `{YOUR_USERNAME}` with your actual Windows username.

3. Finish installation.

### ✅ Enable Stellaris ICDI in Keil

1. Open Keil and go to `Project → Options for Target` (or click the gear icon).
2. Under the **Debug** tab, set the debugger to:  
**"Stellaris ICDI"**



##  Step 3: Install Stellaris ICDI USB Drivers

Windows needs the proper USB drivers to talk to the LaunchPad.

### 🔗 Download the Drivers

- 👉 [Stellaris ICDI USB Drivers (TI.com)](https://www.ti.com/tool/STELLARIS_ICDI_DRIVERS)

Click **Download Options** on the TI page and download the ZIP file. Extract it somewhere safe (e.g. `C:\TivaDrivers`).

### 🛠️ Install the Drivers

1. Plug your Tiva C board into your PC via the **DEBUG USB port**.
2. Open **Device Manager** and look for two unrecognized devices.
3. For each:
- Right-click → `Update driver`
- Select **"Browse my computer for drivers"**
- Point it to the folder where you extracted the drivers
- Make sure "Include subfolders" is checked → Click **Next**

Repeat this for all Stellaris ICDI interfaces.

## 🧩 Quick Reference Links

| Tool | Link |
|------|------|
| TM4C Pack (Keil.TM4C_DFP.1.1.0.pack) | [Download](https://keilpack.azureedge.net/pack/Keil.TM4C_DFP.1.1.0.pack) |
| Stellaris ICDI USB Drivers | [TI Tool Page](https://www.ti.com/tool/STELLARIS_ICDI_DRIVERS) |
| Stellaris ICDI Debugger Add-on | [Keil/ARM Guide](https://developer.arm.com/documentation/ka002280/latest/) |

---

