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
    // Shiki has no "h" grammar, so C headers fell back to plain text.
    languageAlias: { h: 'c' },
  },
  vite: {
    plugins: [
      groupIconVitePlugin({
        customIcon: {
          // Keyed on the extension, not the bare letter: matching is
          // label.includes(key), so 'c' also hit lcd.h, trafficlight.h and
          // ledstopwatch.h purely because those names contain a "c".
          '.c': cProgramIcon,
        },
      }),
    ],
  },

  cleanUrls: true,
  lastUpdated: true,

  base: "/",
  head: [
    ["link", { rel: "icon", type: "image/svg+xml", href: "/favicon.svg" }],
    ["meta", { name: "theme-color", content: "#dc2626" }],
    ["meta", { property: "og:type", content: "website" }],
    ["meta", { property: "og:site_name", content: "ECE Labs · Birzeit University" }],
    ["meta", { property: "og:image", content: "https://encs4110.birzeit.me/og.png" }],
    ["meta", { property: "og:image:width", content: "1200" }],
    ["meta", { property: "og:image:height", content: "630" }],
    ["meta", { name: "twitter:card", content: "summary_large_image" }],
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
      gtag('config', 'G-YVX3LGJEHB');
      gtag('config', 'G-K834Q72PP9');`,
    ],
  ],

  title: "ENCS4110 Birzeit University",
  description:
    "A hands-on ARM Cortex-M4 embedded systems laboratory covering assembly programming, GPIO, interrupts, timers, ADC, UART, and peripheral interfacing using the TM4C123 LaunchPad.",
  sitemap: { hostname: "https://encs4110.birzeit.me/" },

  // Link previews: per-page title, the page's (or site's) description, and canonical URL.
  transformHead({ pageData, siteData }) {
    const page = pageData.frontmatter.layout === "home" ? "" : pageData.title
    const path = pageData.relativePath.replace(/(^|\/)index\.md$/, "$1").replace(/\.md$/, "")
    return [
      ["meta", { property: "og:title", content: page ? `${page} · ENCS4110` : "ENCS4110 Computer Design Laboratory" }],
      ["meta", { property: "og:description", content: pageData.frontmatter.description || pageData.description || siteData.description }],
      ["meta", { property: "og:url", content: `https://encs4110.birzeit.me/${path}` }],
    ]
  },

  themeConfig: {
    logo: "/favicon.svg",

    footer: {
      message: 'ENCS4110 · Computer Design Laboratory',
      copyright: 'Developed by <a href="https://mabushelbaia.com" target="_blank">Mohammad Abu-Shelbaia</a>'
    }, 
    siteTitle: "ENCS4110-BZU",

    nav: [
      { text: "Home", link: "/" },
      { text: "Experiments", link: "/experiments/" },
      { text: "All labs", link: "https://birzeit.me/" },
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

    socialLinks: [{ icon: "github", link: "https://github.com/ECE-Birzeit/encs4110" }],

    editLink: {
      pattern: "https://github.com/ECE-Birzeit/encs4110/edit/main/docs/:path",
      text: "Edit this page on GitHub",
    },
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
      theme_color: "#dc2626",
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
