import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";

import { defineConfig } from "vitepress";
import { groupIconMdPlugin, groupIconVitePlugin } from 'vitepress-plugin-group-icons'

import { withPwa } from '@vite-pwa/vitepress'


// Vendored rather than referenced by URL. vitepress-plugin-group-icons passes
// an http(s) customIcon through fetch() at build time and, if that fails, only
// warns and drops the icon -- so a flaky host would silently ship a missing
// icon. Reading the file keeps the build self-contained. The leading <?xml?>
// prolog is stripped because the plugin inlines a string only when it starts
// with "<svg"; anything else is treated as an "collection:name" icon id.
const cProgramSvg = readFileSync(
  fileURLToPath(new URL("./icons/c-program.svg", import.meta.url)),
  "utf8",
);
const cProgramIcon = cProgramSvg.slice(cProgramSvg.indexOf("<svg"));

export default withPwa(defineConfig({
  markdown: {
    config(md) {
      md.use(groupIconMdPlugin)
    },
    math: true,
  },
  vite: {
    plugins: [
      groupIconVitePlugin({
        customIcon: {
          'c': cProgramIcon,
        },
      }),
    ],
  },

  cleanUrls: true,
  lastUpdated: true,

  base: "/",
  head: [
    [
      "script",
      {
        async: "",
        src: "https://www.googletagmanager.com/gtag/js?id=G-YVX3LGJEHB",
      },
    ],
    [
      "script",
      {},
      `window.dataLayer = window.dataLayer || [];
      function gtag(){dataLayer.push(arguments);}
      gtag('js', new Date());
      gtag('config', 'G-YVX3LGJEHB');`,
    ],
  ],

  title: "ENCS4110 Birzeit University",
  description:
    "A hands-on ARM Cortex-M4 embedded systems laboratory covering assembly programming, GPIO, interrupts, timers, ADC, UART, and peripheral interfacing using the TM4C123 LaunchPad.",
  themeConfig: {

    footer: {
      message: 'ENCS4110 · Computer Design Laboratory',
      copyright: 'Developed by <a href="https://mabushelbaia.com" target="_blank">Mohammad Abu-Shelbaia</a>'
    }, 
    siteTitle: "ENCS4110-BZU",

    nav: [
      { text: "Home", link: "/" },
      { text: "Experiments", link: "/experiments/" },
    ],

    sidebar: [
      {
        text: "Experiment Navigator",
        link: "/experiments/",
      },
      { 
        text: "Keil uVision5 IDE Setup",
        link: "/setup",
      },
      {
        text: "Part 1: ARM Assembly",
        collapsed: false,
        items: [
          { text: "1. Assembly Basics and Program Structure", link: "/experiments/1" },
          { text: "2. Data Processing and Memory Operations", link: "/experiments/2" },
          { text: "3. Branching and Loops", link: "/experiments/3" },
          { text: "4. Procedures and the Stack", link: "/experiments/4" },
        ],
      },
      {
        text: "Part 2: TM4C123 LaunchPad",
        collapsed: false,
        items: [
          { text: "5. Microcontroller Architecture and GPIO Output", link: "/experiments/5" },
          { text: "6. GPIO Inputs and Interrupt Handling", link: "/experiments/6" },
          { text: "7. Hardware Timers and Timing Control", link: "/experiments/7" },
          { text: "8. Character LCD Display Interface", link: "/experiments/8" },
          { text: "9. Analog-to-Digital Converter (ADC)", link: "/experiments/9" },
          { text: "10. Universal Asynchronous Receiver-Transmitter (UART)", link: "/experiments/10" },
        ],
      },
      {
        text: "Training Assignments",
        collapsed: false,
        items: [
          { text: "Matrix Keypad Interfacing", link: "/keypad-assignment" },
          { text: "Simon-Says Sequence Repeater", link: "/simon-assignment" },
          { text: "Stopwatch with LED Indicator", link: "/ledstopwatch-assignment" },
          { text: "Traffic Light with Pedestrian Crossing", link: "/trafficlight-assignment" },
        ],
      },
      {
        text: "Additional Resources",
        items: [
          {
            text: "Keil uVision5 IDE",
            link: "https://www.keil.com/demo/eval/arm.htm",
          },
          {
            text: "ARM Documentation",
            link: "https://developer.arm.com/documentation/dui0041/latest/",
          },
          {
            text: "ARM Instruction Set Summary",
            link: "https://developer.arm.com/documentation/100165/0201/Programmers-Model/Instruction-set-summary/Processor-instructions",
          },
          {
            text: "TM4C123G Datasheet",
            link: "https://www.ti.com/lit/ds/spms376e/spms376e.pdf",
          },

        ],
      },
    ],

    outline: { level: [2, 3] },

    socialLinks: [{ icon: "github", link: "https://github.com/mabushelbaia/encs4110" }],
    search: {
      provider: "local",
    },
  },
  pwa: {
    registerType: "autoUpdate",
    manifest: {
      name: "Computer Design Laboratory",
      short_name: "ENCS4110",
      description: "A comprehensive guide and resource for ARM assembly language experiments. and TM4C123 LaunchPad microcontroller interfacing.",
      theme_color: "#181818",
      background_color: "#ffffff",
      display: "standalone",
      scope: "/",
      start_url: "/",
      icons: [
        {
          src: "/icons/pwa-icon.svg",
          sizes: "192x192",
          type: "image/svg+xml",
        },
        {
          src: "/icons/pwa-icon.svg",
          sizes: "512x512",
          type: "image/svg+xml",
        },
      ],
    },
    workbox: {
      globPatterns: ["**/*.{css,js,html,svg,png,jpg,jpeg,ico,txt,woff2,pdf}"],
      maximumFileSizeToCacheInBytes: 5000000,
    },
    experimental: {
      includeAllowlist: true,
    },
  },
}));
