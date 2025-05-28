import { defineConfig } from "vitepress";
import { groupIconMdPlugin, groupIconVitePlugin, localIconLoader } from 'vitepress-plugin-group-icons'


export default defineConfig({
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
      })
    ],
  },

  cleanUrls: true,
  lastUpdated: true,

  base: "/encs4110/",
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
      "link",
      {
        rel: "icon",
        href: 'data:image/svg+xml,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100"><text y="90" font-size="90">💻</text></svg>',
      },
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

  title: "Computer Design Laboratory",
  description:
    "A comprehensive guide and resource for ARM assembly language experiments.",
  themeConfig: {
    nav: [
      { text: "Home", link: "/" },
      { text: "Experiments", link: "/experiments/" },
    ],

    sidebar: [
      {
        text: "🧭 Experiment Navigator",
        link: "/experiments/",
      },
      {
        text: "🚀 Keil uVision5 IDE Setup",
        link: "/setup",
      },
      {
        text: "ARM Assembly Experiments",
        collapsed: false,
        items: [
          {
            text: "1. Introduction to ARM Assembly",
            link: "/experiments/1.md",
          },
          {
            text: "2. ARM Data Processing Instructions",
            link: "/experiments/2.md",
          },
          {
            text: "3. ARM Flow Control Instructions",
            link: "/experiments/3.md",
          },
          {
            text: "4. ARM Addressing Modes",
            link: "/experiments/4.md",
          },
          {
            text: "5. ARM Subroutines",
            link: "/experiments/5.md",
          },
        ],
      },
      {
        text: "TM4C123G LaunchPad Experiments",
        collapsed: false,
        items: [
          {
            text: "6. GPIO (General Purpose Input/Output)",
            link: "/experiments/6.md",
          },
          {
            text: "7. External Interrupts (Hardware Interrupts)",
            link: "/experiments/7.md",
          },
          {
            text: "8. Software Interrupts (Timer Interrupts)",
            link: "/experiments/8.md",
          },
          {
            text: "9. Liquid Crystal Display (LCD) Interfacing",
            link: "/experiments/9.md",
          },
          {
            text: "10. Analog-to-Digital Converter (ADC)",
            link: "/experiments/10.md",
          },
          {
            text: "11. Universal Asynchronous Receiver-Transmitter (UART)",
            link: "/experiments/11.md",
          },
        ],
      },
      {
        text: "Resources",
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
});
