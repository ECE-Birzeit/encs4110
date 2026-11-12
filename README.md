<a href="https://encs4110.birzeit.me"><img src="public/og.png" alt="ENCS4110 Computer Design Laboratory" width="100%"></a>

# ENCS4110 · Computer Design Laboratory

Step-by-step lab manual for ENCS4110 at Birzeit University: the ARM Cortex-M4 from the inside (registers, instruction set and assembly), then put to work on the TM4C123 Tiva C LaunchPad with GPIO, interrupts, timers, ADC and UART.

**[Open the manual](https://encs4110.birzeit.me)** · [PDF](https://encs4110.birzeit.me/manual/encs4110_manual.pdf) · [Setup guide](https://encs4110.birzeit.me/setup) · [All labs](https://birzeit.me)

## Run it locally

Requires [Node.js](https://nodejs.org) 24 and npm. The PDFs come from a submodule, so clone recursively:

```sh
git clone --recurse-submodules https://github.com/ECE-Birzeit/encs4110.git
cd encs4110
npm ci
npm run dev # http://localhost:5173, reloads as you edit
```

```sh
npm run build       # outputs to .vitepress/dist
npm run docs:preview     # serve the built site on port 8080
npm run smoke-test:pwa   # check the offline (PWA) build, run after build
```

Already cloned without the submodule? Run `git submodule update --init`.

## Project layout

| Path | Contents |
|:--|:--|
| `index.md` | Home page |
| `setup/` | Setup Guide (`index.md`) and one page per tool or project: Keil and Tiva support, the assembly project, the LaunchPad project |
| `experiments/` | One page per experiment (`exp1-assembly-basics.md` … `exp10-uart.md`) and the Experiment Navigator. The sidebar is built from these files (`.vitepress/lab.mts`) |
| `snippets/` | Code included into pages |
| `public/manual/` | Submodule: the `pdf` branch of [encs4110-manual](https://github.com/ECE-Birzeit/encs4110-manual) |
| `.vitepress/config.mts` | Site config: nav, sidebar, PWA, analytics, link previews |
| `scripts/smoke-test-pwa.mjs` | Checks the service worker and offline manifest |

### Updating the PDFs

The PDFs are built in [encs4110-manual](https://github.com/ECE-Birzeit/encs4110-manual) and published to its `pdf` branch. To pull the latest into this site:

```sh
git submodule update --remote public/manual
git add public/manual
git commit -m "Update manual PDFs"
```

## Contributing

Found a mistake? Click **Edit this page on GitHub** at the bottom of any page, or [open an issue](https://github.com/ECE-Birzeit/encs4110/issues/new/choose). Problems in the PDF itself belong in [encs4110-manual](https://github.com/ECE-Birzeit/encs4110-manual/issues/new/choose). The [contributing guide](https://github.com/ECE-Birzeit/.github/blob/main/CONTRIBUTING.md) covers writing style and pull requests.

## Deployment

`.github/workflows/deploy.yml` builds the site, runs the PWA smoke test, and deploys to GitHub Pages on every push to `main`, so merged changes are live within a couple of minutes at [encs4110.birzeit.me](https://encs4110.birzeit.me). The old domain, `encs4110.me`, redirects there through [encs4110-redirect](https://github.com/ECE-Birzeit/encs4110-redirect).

## License

The written content is licensed under [CC BY 4.0](LICENSE): you may share and adapt it, including for your own course, as long as you credit the source. Code, meaning `snippets/`, `scripts/` and the site code (`.vitepress/`, `.github/`), is licensed under [MIT](LICENSE-CODE). © 2026 [Mohammad Abu-Shelbaia](https://mabushelbaia.com).

The Birzeit University name and logo are the university's; they are not licensed here. Screenshots of third-party software, and product names and logos, belong to their respective owners. `public/datasheets/tm4c123gh6pm.pdf` is Texas Instruments' TM4C123GH6PM data sheet (SPMS376E), copied unchanged so it opens offline; it is TI's and not covered by either license.

---

<sub>Part of [ECE Labs](https://birzeit.me) · Maintained by [Mohammad Abu-Shelbaia](https://mabushelbaia.com)</sub>
