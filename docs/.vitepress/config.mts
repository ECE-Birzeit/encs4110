import { defineConfig } from "vitepress";
import { groupIconMdPlugin, groupIconVitePlugin, localIconLoader } from 'vitepress-plugin-group-icons'

import { withPwa } from '@vite-pwa/vitepress'
import Icons from 'unplugin-icons/vite'


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
          'c': 'https://uxwing.com/wp-content/themes/uxwing/download/brands-and-social-media/c-program-icon.svg',
        },
      }),
      Icons({
        compiler: 'vue3',
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

    [
      "meta",
      {
        property: "og:image",
        content: "/path/to/image.png",
      },
    ],
    [
      "meta",
      {
        name: "twitter:image",
        content: "/path/to/image.png",
      },
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
    // logo: "icons/monitor.svg",
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
          { text: "3. Control Flow and Subroutines", link: "/experiments/3" },
        ],
      },
      {
        text: "Part 2: TM4C123 LaunchPad",
        collapsed: false,
        items: [
          { text: "4. Microcontroller Architecture and GPIO Output", link: "/experiments/4" },
          { text: "5. GPIO Inputs and Interrupt Handling", link: "/experiments/5" },
          { text: "6. Hardware Timers and Timing Control", link: "/experiments/6" },
          { text: "7. Character LCD Display Interface", link: "/experiments/7" },
          { text: "8. Analog-to-Digital Converter (ADC)", link: "/experiments/8" },
          { text: "9. Universal Asynchronous Receiver-Transmitter (UART)", link: "/experiments/9" },
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

    socialLinks: [{ icon: "github", link: "https://github.com/mabushelbaia/encs4110" }],
    search: {
      provider: "local",
    },
  },
  pwa: {
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
          src: "data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><text y='90' font-size='90'>💻</text></svg>",
          sizes: "192x192",
          type: "image/svg+xml",
        },
        {
          src: "data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><text y='90' font-size='90'>💻</text></svg>",
          sizes: "512x512",
          type: "image/svg+xml",
        },
      ],
    },
    workbox: {
      globPatterns: ["**/*.{css,js,html,svg,png,ico,txt,woff2,pdf}"],
      maximumFileSizeToCacheInBytes: 5000000,
    },
    experimental: {
      includeAllowlist: true,
    },
  },
}));
