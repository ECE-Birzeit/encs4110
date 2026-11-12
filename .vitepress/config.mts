import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";

import { defineConfig } from "vitepress";
import { groupIconMdPlugin, groupIconVitePlugin } from 'vitepress-plugin-group-icons'

import { withPwa } from '@vite-pwa/vitepress'
import { linkPreviews } from "./og.mts"
import { all, redirects, experimentEdges, sidebar } from "./lab.mts"
import { registerDiagrams, datasheetRefs } from "./registers.mts"

const srcDir = fileURLToPath(new URL("..", import.meta.url))

// Per-experiment link previews, drawn at build time (see og.mts)
const og = linkPreviews({ code: "ENCS4110", name: "Computer Design Laboratory", host: "encs4110.birzeit.me", accent: "#f87171", platform: "TM4C123 · ARM Cortex-M4" })


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

// Register -> page in the local datasheet copy (scripts/datasheet-pages.py)
const DATASHEET = "/datasheets/tm4c123gh6pm.pdf"
const datasheetPages: Record<string, number> = JSON.parse(
  readFileSync(fileURLToPath(new URL("./datasheet-pages.json", import.meta.url)), "utf8"),
);

// Addresses from before the pages moved (2026-11-08), so bookmarks and shared links still work.
// /setup itself needs no entry: GitHub Pages sends /setup to /setup/ on its own.
const moved: Record<string, string> = {
  "/experiments/1": "/experiments/exp1-assembly-basics",
  "/experiments/2": "/experiments/exp2-data-processing",
  "/experiments/3": "/experiments/exp3-branching-loops",
  "/experiments/4": "/experiments/exp4-procedures-stack",
  "/experiments/5": "/experiments/exp5-gpio-output",
  "/experiments/6": "/experiments/exp6-gpio-interrupts",
  "/experiments/7": "/experiments/exp7-timers",
  "/experiments/8": "/experiments/exp8-lcd",
  "/experiments/9": "/experiments/exp9-adc",
  "/experiments/10": "/experiments/exp10-uart",
  "/assembly-project": "/setup/assembly-project",
  "/tiva-project": "/setup/tiva-project",
}

export default withPwa(defineConfig({
  markdown: {
    config(md) {
      md.use(groupIconMdPlugin)
      md.use(registerDiagrams)
      md.use(datasheetRefs, {
        pdf: DATASHEET,
        name: "TM4C123GH6PM datasheet",
        pages: datasheetPages,
        // Names the pages use that aren't the datasheet's own mnemonic
        alias: { NVIC_ISER: "EN0", GPIO_CR: "GPIOCR", STLOAD: "STRELOAD" },
      })
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
          // label.includes(key), so 'c' also hit lcd.h and uart.h purely
          // because those names contain a "c".
          '.c': cProgramIcon,
        },
      }),
    ],
  },

  srcExclude: ["README.md", "snippets/**"],
  cleanUrls: true,
  lastUpdated: true,

  base: "/",
  head: [
    ["link", { rel: "icon", type: "image/svg+xml", href: "/favicon.svg" }],
    ["link", { rel: "apple-touch-icon", href: "/apple-touch-icon.png" }],
    ["meta", { name: "theme-color", content: "#dc2626" }],
    ["meta", { property: "og:type", content: "website" }],
    ["meta", { property: "og:site_name", content: "ECE Labs · Birzeit University" }],
    ...og.head,
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

  buildEnd: all(og.buildEnd, redirects(moved, "encs4110.birzeit.me")),

  // No Previous page on the first experiment, no Next page on the last (see lab.mts)
  transformPageData: experimentEdges(srcDir),

  // Link previews: per-page title, the page's (or site's) description, and canonical URL.
  transformHead({ pageData, siteData }) {
    const page = pageData.frontmatter.layout === "home" ? "" : pageData.title
    const path = pageData.relativePath.replace(/(^|\/)index\.md$/, "$1").replace(/\.md$/, "")
    return [
      ...og.transformHead({ pageData } as any),
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
      { text: "Experiments", link: "/experiments/", activeMatch: "/experiments/" },
      { text: "All labs", link: "https://birzeit.me/" },
    ],

    // Built from the pages (see lab.mts): adding an experiment or a setup page is the only step
    sidebar: sidebar(srcDir, {
      downloads: [
          { text: "Keil MDK (uVision5)", link: "https://www.keil.com/demo/eval/arm.htm" },
          { text: "Keil TM4C Device Pack", link: "https://keilpack.azureedge.net/pack/Keil.TM4C_DFP.1.1.0.pack" },
          { text: "Stellaris ICDI Add-on for Keil", link: "https://developer.arm.com/documentation/ka002280/latest/" },
          { text: "Stellaris ICDI Drivers", link: "https://www.ti.com/tool/STELLARIS_ICDI_DRIVERS" },
          { text: "PuTTY (UART lab)", link: "https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html" },
          // target '_blank' makes VitePress treat the link as external, so the
          // PDF opens in a new tab instead of going through the router.
          { text: "Full Lab Manual (PDF)", link: "/manual/encs4110_manual.pdf", target: "_blank" },
      ],
      docs: [
          // A local copy so it's cached with the site and opens offline. target '_blank' keeps the
          // router out of it, as for the manual PDF.
          { text: "TM4C123GH6PM Datasheet", link: DATASHEET, target: "_blank" },
          { text: "EK-TM4C123GXL LaunchPad User's Guide", link: "https://www.ti.com/lit/ug/spmu296/spmu296.pdf" },
          { text: "Arm Cortex-M4 Generic User Guide", link: "https://developer.arm.com/documentation/dui0553/latest/" },
          {
            text: "ARM Instruction Set Summary",
            link: "https://developer.arm.com/documentation/100165/0201/Programmers-Model/Instruction-set-summary/Processor-instructions",
          },
          { text: "armasm User Guide (directives)", link: "https://developer.arm.com/documentation/dui0473/latest/" },
          { text: "HD44780 LCD Datasheet", link: "https://www.sparkfun.com/datasheets/LCD/HD44780.pdf" },
      ],
    }),

    outline: { level: [2, 3] },

    socialLinks: [{ icon: "github", link: "https://github.com/ECE-Birzeit/encs4110" }],

    editLink: {
      pattern: "https://github.com/ECE-Birzeit/encs4110/edit/main/:path",
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
      description: "A comprehensive guide and resource for ARM assembly language experiments and TM4C123 LaunchPad microcontroller interfacing.",
      theme_color: "#dc2626",
      background_color: "#ffffff",
      display: "standalone",
      scope: "/",
      start_url: "/",
      // Rendered from favicon.svg, the sidebar logo, so the installed app matches the site.
      icons: [
        { src: "/icons/pwa-192x192.png", sizes: "192x192", type: "image/png" },
        { src: "/icons/pwa-512x512.png", sizes: "512x512", type: "image/png" },
        { src: "/icons/maskable-512x512.png", sizes: "512x512", type: "image/png", purpose: "maskable" },
        { src: "/favicon.svg", sizes: "any", type: "image/svg+xml" },
      ],
    },
    workbox: {
      globPatterns: ["**/*.{css,js,html,svg,png,jpg,jpeg,ico,txt,woff2,pdf}"],
      // The site links only the full manual; the per-experiment PDFs from the submodule are unused.
      // Link-preview images are for chat apps and search engines, not for reading offline.
      globIgnores: ["manual/encs4110_[0-9]*.pdf", "og/**"],
      // The datasheet is 8.7 MB; precaching it is what makes it open offline.
      maximumFileSizeToCacheInBytes: 10000000,
    },
    experimental: {
      includeAllowlist: true,
    },
  },
}));
