// Link previews (og:image) per experiment page, drawn at build time in the ECE Labs terminal style.
// The same file lives in all four lab sites; only the options passed to linkPreviews() differ.
//
//   const og = linkPreviews({ code: 'ENCS4101', ... })
//   head: [...og.head]              // og:image tags for the site
//   transformHead: (ctx) => [...og.transformHead(ctx), ...]
//   buildEnd: og.buildEnd
//
// Experiment pages (experiments/<name>.md, not the index) get /og/<name>.png; every other page
// keeps the site's og.png. Titles come from the pages themselves, so a new experiment gets its
// preview on the next build with nothing else to do.
import { mkdir, readFile, writeFile } from 'node:fs/promises'
import { createRequire } from 'node:module'
import { dirname, join } from 'node:path'
import type { HeadConfig, SiteConfig, TransformContext } from 'vitepress'

const require = createRequire(import.meta.url)

export interface PreviewOptions {
  code: string // ENCS4101
  name: string // Embedded Systems Laboratory
  host: string // encs4101.birzeit.me
  accent: string // the lab's colour, as on birzeit.me's dark theme
  platform: string // shown bottom left, e.g. "STM32 · CubeIDE · HAL"
}

interface Page { title: string; number: string; path: string }

const EXPERIMENT = /(^|\/)experiments\/(?!index\.md$)([^/]+)\.md$/

export function linkPreviews(o: PreviewOptions) {
  const pages = new Map<string, Page>()
  const image = (file: string) => `https://${o.host}/${file}`

  function forPage(relativePath: string, title: string): Page | null {
    const m = relativePath.match(EXPERIMENT)
    if (!m || !title) return null
    const num = m[2].match(/\d+/)
    return { title, number: num ? `Exp ${Number(num[0])}` : '', path: `og/${m[2]}.png` }
  }

  return {
    head: [
      ['meta', { property: 'og:image:width', content: '1200' }],
      ['meta', { property: 'og:image:height', content: '630' }],
      ['meta', { name: 'twitter:card', content: 'summary_large_image' }],
    ] as HeadConfig[],

    transformHead({ pageData }: TransformContext): HeadConfig[] {
      const page = forPage(pageData.relativePath, pageData.title)
      if (page) pages.set(page.path, page)
      const alt = page ? `${o.code} ${page.number}: ${page.title}` : `${o.code} ${o.name}`
      return [
        ['meta', { property: 'og:image', content: image(page ? page.path : 'og.png') }],
        ['meta', { property: 'og:image:alt', content: alt }],
      ]
    },

    async buildEnd(site: SiteConfig) {
      if (!pages.size) return
      const [{ default: satori }, { Resvg }] = await Promise.all([import('satori'), import('@resvg/resvg-js')])
      const fonts = await Promise.all(([400, 500, 700, 800] as const).map(async (weight) => ({
        name: 'JetBrains Mono',
        weight,
        style: 'normal' as const,
        data: await readFile(require.resolve(`@fontsource/jetbrains-mono/files/jetbrains-mono-latin-${weight}-normal.woff`)),
      })))
      for (const page of pages.values()) {
        const svg = await satori(card(o, page) as any, { width: 1200, height: 630, fonts })
        const png = new Resvg(svg, { fitTo: { mode: 'width', value: 1200 } }).render().asPng()
        const out = join(site.outDir, page.path)
        await mkdir(dirname(out), { recursive: true })
        await writeFile(out, png)
      }
      console.log(`  link previews: ${pages.size} written to ${join(site.outDir, 'og')}`)
    },
  }
}

// --- the picture: a terminal window showing the page's address, the experiment number big in the lab colour

type Node = { type: string; props: { style?: Record<string, unknown>; children?: unknown } }
const el = (style: Record<string, unknown>, children?: unknown): Node => ({ type: 'div', props: { style: { display: 'flex', ...style }, children } })
const MONO = 'JetBrains Mono'
const dot = () => el({ width: 14, height: 14, borderRadius: 7, background: '#3a3b39' })

function card(o: PreviewOptions, page: Page): Node {
  const url = `${o.host}/${page.path.replace(/^og\//, 'experiments/').replace(/\.png$/, '')}`
  return el({ width: 1200, height: 630, boxSizing: 'border-box', padding: 44, background: '#0b0c0b', fontFamily: MONO }, [
    el({ flex: 1, flexDirection: 'column', border: '1px solid #2a2b29', borderRadius: 18, background: '#121312', overflow: 'hidden' }, [
      el({ alignItems: 'center', gap: 10, height: 58, padding: '0 24px', borderBottom: '1px solid #2a2b29', background: '#171816' }, [
        dot(), dot(), dot(),
        el({ marginLeft: 14, padding: '6px 14px', border: '1px solid #2a2b29', borderRadius: 8, background: '#121312', fontSize: 19, color: '#8d8f89' }, url),
      ]),
      el({ flex: 1, flexDirection: 'column', padding: '40px 48px 36px' }, [
        el({ alignItems: 'center', gap: 16, fontSize: 22, fontWeight: 500, color: '#b9bab5' }, [
          el({ width: 14, height: 14, borderRadius: 3, background: o.accent }),
          el({ color: o.accent, fontWeight: 700 }, o.code),
          el({}, o.name),
        ]),
        el({ marginTop: 'auto', fontSize: 132, fontWeight: 800, lineHeight: 0.9, letterSpacing: -8, color: o.accent }, page.number),
        el({ marginTop: 18, maxWidth: 960, fontSize: 52, fontWeight: 800, lineHeight: 1.08, letterSpacing: -2, color: '#e8e6e1' }, page.title),
        el({ marginTop: 28, justifyContent: 'space-between', fontSize: 21, color: '#8d8f89' }, [
          el({}, o.platform),
          el({}, [el({ color: '#4ade80', marginRight: 12 }, '>'), 'ECE Labs · Birzeit University']),
        ]),
      ]),
    ]),
  ])
}
