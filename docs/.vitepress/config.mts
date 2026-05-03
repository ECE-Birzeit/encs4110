import { defineConfig } from "vitepress";
import { groupIconMdPlugin, groupIconVitePlugin, localIconLoader } from 'vitepress-plugin-group-icons'
import { withPwa } from '@vite-pwa/vitepress'


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
      })
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
        text: "Experiment Navigator",
        link: "/experiments/",
      },
      { 
        text: "Keil uVision5 IDE Setup",
        link: "/setup",
      },
      {
        text: "ARM Cortex-M4 Assembly Language Basics",
        collapsed: false,
        items: [
          {
            text: "1. Assembly Basics and Program Structure",
            collapsed: true,
            items: [
              { text: "Theory", link: "/experiments/1_theory" },
              { text: "Procedure", link: "/experiments/1" },
            ],
          },
          {
            text: "2. Data Processing and Memory Operations",
            collapsed: true,
            items: [
              { text: "Theory", link: "/experiments/2_theory" },
              { text: "Procedure", link: "/experiments/2" },
            ],
          },
          {
            text: "3. Control Flow and Subroutines",
            collapsed: true,
            items: [
              { text: "Theory", link: "/experiments/3_theory" },
              { text: "Procedure", link: "/experiments/3" },
            ],
          },
        ],
      },
      {
        text: "EK-TM4C123GXL Microcontroller",
        collapsed: false, 
        items: [
          {
            text: "4. Microcontroller Architecture and GPIO Output",
            collapsed: true,
            items: [
              { text: "Theory", link: "/experiments/4_theory" },
              { text: "Procedure", link: "/experiments/4" },
            ],
          },
          {
            text: "5. GPIO Inputs and Interrupt Handling",
            collapsed: true,
            items: [
              { text: "Theory", link: "/experiments/5_theory" },
              { text: "Procedure", link: "/experiments/5" },
            ],
          },
          {
            text: "6. Hardware Timers and Timing Control",
            collapsed: true,
            items: [
              { text: "Theory", link: "/experiments/6_theory" },
              { text: "Procedure", link: "/experiments/6" },
            ],
          },
          {
            text: "7. Character LCD Display Interface",
            collapsed: true,
            items: [
              { text: "Theory", link: "/experiments/7_theory" },
              { text: "Procedure", link: "/experiments/7" },
            ],
          },
          {
            text: "8. Analog-to-Digital Converter (ADC)",
            collapsed: true,
            items: [
              { text: "Theory", link: "/experiments/8_theory" },
              { text: "Procedure", link: "/experiments/8" },
            ],
          },
          {
            text: "9. Universal Asynchronous Receiver-Transmitter (UART)",
            collapsed: true,
            items: [
              { text: "Theory", link: "/experiments/9_theory" },
              { text: "Procedure", link: "/experiments/9" },
            ],
          },
        ],
      },
      {
        text: "Extra TM4C123 Examples",
        collapsed: false,
        items: [
          {
            text: "PWM Signal Generation Using Timers",
            link: "/examples/pwm-timer",
          },
          {
            text: "Quadrature Encoder Interface (QEI)",
            link: "/examples/qei-encoder",
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
  pwa: {
    manifest: {
      name: "Computer Design Laboratory",
      short_name: "ENCS 4110",
      description: "A comprehensive guide and resource for ARM assembly language experiments.",
      theme_color: "#646cff",
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
      globPatterns: ["**/*.{css,js,html,svg,png,ico,txt,woff2}"],
    },
  },
}));
