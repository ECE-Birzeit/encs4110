import { defineConfig } from "vitepress";

export default defineConfig({
  markdown: {
    math: true,
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
  ],

  title: "Computer Design Laboratory",
  description:
    "A comprehensive guide and resource for ARM assembly language experiments.",
  themeConfig: {
    // https://vitepress.dev/reference/default-theme-config
    nav: [
      { text: "Home", link: "/" },
      { text: "Experiments", link: "/Experiment-1" },
    ],

    sidebar: [
      {
        text: "ARM Assembly Language Experiments",
        items: [
          {
            text: "Introduction to ARM Assembly",
            link: "/Experiment-1.md",
          },
          {
            text: "ARM Data Processing Instructions",
            link: "/Experiment-2.md",
          },
          {
            text: "ARM Flow Control Instructions",
            link: "/Experiment-3.md",
          },
          {
            text: "ARM Addressing Modes",
            link: "/Experiment-4.md",
          },
          {
            text: "ARM Subroutines",
            link: "/Experiment-5.md",
          },
        ],
      },
      {
        text: "TM4C123G LaunchPad Experiments",
        items: [
          {
            text: "GPIO (General Purpose Input/Output)",
            link: "/Experiment-6.md",
          },
          {
            text: "External Interrupts",
            link: "/Experiment-7.md",
          },
        ],
      },
      {
        text: "Resources",
        items: [
          {
            text: "Setting up Keil uVision5 IDE for TM4C123G LaunchPad",
            link: "/setup.md",
          },
          {
            text: "Keil uVision5 IDE",
            link: "https://www.keil.com/demo/eval/arm.htm",
          },
          {
            text: "ARM Documentation",
            link: "https://developer.arm.com/documentation/dui0041/latest/",
          },
          {
            text: "ARM Insturction Set Summary",
            link: "https://developer.arm.com/documentation/100165/0201/Programmers-Model/Instruction-set-summary/Processor-instructions",
          },
        ],
      },
    ],

    socialLinks: [{ icon: "github", link: "https://github.com/mabushelbaia/" }],
    search: {
      provider: "local",
    },
  },
});
