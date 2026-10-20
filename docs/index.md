---
# VitePress Home Page Configuration
layout: home

hero:
  name: "ENCS4110"
  text: "Computer Design Laboratory"
  tagline: "Department of Electrical & Computer Engineering"

  actions:
    - theme: brand
      text: "Explore Experiments"
      link: /experiments/
    - theme: alt
      text: "Download Full Manual"
      link: /manual/encs4110_manual.pdf
      target: _blank
    - theme: alt
      text: "Setup Guide"
      link: /setup

features:
  - icon:
      src: /icons/cortex.svg
      alt: Processor
      width: 28
      height: 28
      wrap: true
    title: "ARM Cortex-M4 Architecture"
    details: |
      Understand the ARM Cortex-M4 processor through
      structured labs covering its components, registers,
      instruction set, and operating modes.
  - icon:
      src: /icons/assembly.svg
      alt: Assembly code
      width: 28
      height: 28
      wrap: true
    title: "Assembly Programming"
    details: |
      Gain hands-on experience writing ARM assembly code,
      focusing on data processing, flow control, procedures,
      and stack management.
  - icon:
      src: /icons/launchpad.svg
      alt: LaunchPad board
      width: 28
      height: 28
      wrap: true
    title: "EK-TM4C123GXL Microcontroller"
    details: |
      Apply theory to practice by programming the Tiva C
      LaunchPad, interfacing with sensors, displays, and
      other peripherals.
---
