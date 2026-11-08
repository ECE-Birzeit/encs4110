// The shared lab-site conventions, the same file in every ECE lab site and in lab-template:
//
//   experiments/expN-slug.md   one page per experiment; N orders them, the slug is free
//   experiments/index.md       the Experiment Navigator
//   setup/index.md             the Setup Guide; setup/<tool>.md one page per tool or project
//
// The sidebar is built from those files, so adding a page is the only step: no sidebar to edit.
// Front matter an experiment can use (all optional except title):
//   title:   the page title (else its first "# " heading)
//   short:   a shorter name for the sidebar
//   part:    a group heading, e.g. "Part 1: ARM Assembly"; experiments without one go under "Experiments"
// and a setup page: title, short, order (a number; lower comes first).
//
// redirects() keeps old addresses working after a page moves: each one gets a tiny page that sends
// the reader (and search engines, via the canonical link) to the new address, keeping any #anchor.
import { existsSync, mkdirSync, readdirSync, readFileSync, writeFileSync } from 'node:fs'
import { dirname, join } from 'node:path'
import type { DefaultTheme, SiteConfig } from 'vitepress'

type Item = DefaultTheme.SidebarItem

function frontMatter(file: string): Record<string, string> {
  const text = readFileSync(file, 'utf8')
  const out: Record<string, string> = {}
  const m = text.match(/^---\r?\n([\s\S]*?)\r?\n---/)
  if (m) {
    for (const line of m[1].split(/\r?\n/)) {
      const kv = line.match(/^([A-Za-z_]+):\s*(.*)$/)
      if (kv && kv[2] !== '') out[kv[1]] = kv[2].trim().replace(/^(["'])(.*)\1$/, '$2')
    }
  }
  if (!out.title) {
    const h = text.replace(/^---[\s\S]*?---/, '').match(/^#\s+(.+)$/m)
    if (h) out.title = h[1].trim()
  }
  return out
}

export interface Experiment { n: number; slug: string; link: string; title: string; short: string; part?: string }

export function experiments(srcDir: string): Experiment[] {
  const dir = join(srcDir, 'experiments')
  if (!existsSync(dir)) return []
  return readdirSync(dir)
    .map((f) => f.match(/^exp(\d+)-([a-z0-9-]+)\.md$/))
    .filter((m): m is RegExpMatchArray => !!m)
    .map((m) => {
      const fm = frontMatter(join(dir, m[0]))
      const title = fm.title || m[2]
      return { n: Number(m[1]), slug: m[0].replace(/\.md$/, ''), link: `/experiments/${m[0].replace(/\.md$/, '')}`, title, short: fm.short || title, part: fm.part }
    })
    .sort((a, b) => a.n - b.n)
}

function setupPages(srcDir: string): Item[] {
  const dir = join(srcDir, 'setup')
  if (!existsSync(dir)) return []
  return readdirSync(dir)
    .filter((f) => f.endsWith('.md') && f !== 'index.md')
    .map((f) => ({ f, fm: frontMatter(join(dir, f)) }))
    .sort((a, b) => Number(a.fm.order ?? 99) - Number(b.fm.order ?? 99) || (a.fm.title || a.f).localeCompare(b.fm.title || b.f))
    .map(({ f, fm }) => ({ text: fm.short || fm.title || f, link: `/setup/${f.replace(/\.md$/, '')}` }))
}

export interface SidebarOptions {
  downloads?: Item[] // tools and the manual PDF
  docs?: Item[] // datasheets and references
}

// Same order on every lab site: Experiment Navigator, Getting Started, the experiments (grouped by
// part when they have one), then Downloads and Documentation (pushed to the bottom by custom.css).
export function sidebar(srcDir: string, o: SidebarOptions = {}): Item[] {
  const out: Item[] = [{ text: 'Experiment Navigator', link: '/experiments/' }]
  if (existsSync(join(srcDir, 'setup', 'index.md'))) {
    const items = setupPages(srcDir)
    out.push({ text: 'Getting Started', link: '/setup/', collapsed: items.length ? false : undefined, items: items.length ? items : undefined })
  }
  const groups: { text: string; items: Item[] }[] = []
  for (const e of experiments(srcDir)) {
    const text = e.part || 'Experiments'
    let g = groups.find((x) => x.text === text)
    if (!g) groups.push((g = { text, items: [] }))
    g.items.push({ text: `${e.n}. ${e.short}`, link: e.link })
  }
  for (const g of groups) out.push({ text: g.text, collapsed: false, items: g.items })
  if (o.downloads?.length) out.push({ text: 'Downloads', collapsed: true, items: o.downloads })
  if (o.docs?.length) out.push({ text: 'Documentation', collapsed: true, items: o.docs })
  return out
}

// redirects({ '/experiments/1': '/experiments/exp1-assembly-basics' }, host) -> a buildEnd hook
export function redirects(map: Record<string, string>, host: string) {
  return (site: SiteConfig) => {
    const base = site.site.base || '/'
    for (const [from, to] of Object.entries(map)) {
      const target = base.replace(/\/$/, '') + to
      const file = join(site.outDir, from.replace(/^\//, '').replace(/\/$/, '/index') + '.html')
      mkdirSync(dirname(file), { recursive: true })
      // Chat apps and search engines read this page without following the redirect, so it carries the
      // target's own link preview (title, description, image) copied from the target's built page
      const built = join(site.outDir, to.replace(/^\//, '').replace(/\/$/, '/index') + '.html')
      const page = existsSync(built) ? readFileSync(built, 'utf8') : ''
      const title = (page.match(/<title>([^<]*)<\/title>/) || [])[1] || 'This page moved'
      const preview = (page.match(/<meta (?:property="og:[^"]+"|name="(?:description|twitter:[^"]+)") content="[^"]*">/g) || [])
        .filter((m) => !m.includes('og:url')).join('\n')
      writeFileSync(file, `<!doctype html>
<html lang="en"><head><meta charset="utf-8"><title>${title}</title>
<meta name="robots" content="noindex">
${preview}
<meta property="og:url" content="https://${host}${to}">
<link rel="canonical" href="https://${host}${to}">
<meta http-equiv="refresh" content="0; url=${target}">
<script>location.replace(${JSON.stringify(target)} + location.hash)</script>
</head><body><p>This page moved to <a href="${target}">${host}${to}</a>.</p></body></html>
`)
    }
    if (Object.keys(map).length) console.log(`  redirects: ${Object.keys(map).length} old addresses kept working`)
  }
}

// The first experiment has no "Previous page" and the last no "Next page": the sidebar around them is
// setup pages and downloads, not experiments. For transformPageData.
export function experimentEdges(srcDir: string) {
  return (pageData: { relativePath: string; frontmatter: Record<string, any> }) => {
    const list = experiments(srcDir)
    if (!list.length) return
    const path = pageData.relativePath.replace(/\.md$/, '')
    if (path === `experiments/${list[0].slug}` && pageData.frontmatter.prev === undefined) pageData.frontmatter.prev = false
    if (path === `experiments/${list[list.length - 1].slug}` && pageData.frontmatter.next === undefined) pageData.frontmatter.next = false
  }
}

// Several buildEnd hooks in order (link previews, redirects, ...)
export function all(...hooks: ((site: SiteConfig) => unknown)[]) {
  return async (site: SiteConfig) => { for (const h of hooks) await h(site) }
}
