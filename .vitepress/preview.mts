// Preview pages: reworked experiments that TAs test before they replace the current ones.
// encs4110 only (lab.mts is shared by every lab site, so this stays out of it).
//
//   preview/expN-slug.md   a reworked experiment; front matter `current:` links back to the page it
//                          will replace, and that page's front matter `preview:` links here.
//                          JumpToProcedure.vue shows both as buttons.
//
// previewGroup() adds them to the sidebar as one group, just above Downloads.
import { existsSync, readdirSync, readFileSync } from 'node:fs'
import { join } from 'node:path'
import type { DefaultTheme } from 'vitepress'

type Item = DefaultTheme.SidebarItem

function field(text: string, key: string): string | undefined {
  const fm = text.match(/^---\r?\n([\s\S]*?)\r?\n---/)
  const m = fm?.[1].match(new RegExp(`^${key}:\\s*(.+)$`, 'm'))
  return m?.[1].trim().replace(/^(["'])(.*)\1$/, '$2')
}

export function previewGroup(srcDir: string, items: Item[]): Item[] {
  const dir = join(srcDir, 'preview')
  if (!existsSync(dir)) return items
  const pages = readdirSync(dir)
    .map((f) => f.match(/^exp(\d+)-[a-z0-9-]+\.md$/))
    .filter((m): m is RegExpMatchArray => !!m)
    .sort((a, b) => Number(a[1]) - Number(b[1]))
    .map((m) => {
      const text = readFileSync(join(dir, m[0]), 'utf8')
      const name = field(text, 'short') || field(text, 'title') || m[0]
      return { text: `${m[1]}. ${name}`, link: `/preview/${m[0].replace(/\.md$/, '')}` }
    })
  if (!pages.length) return items
  const group: Item = { text: 'Preview: new format', collapsed: false, items: pages }
  const at = items.findIndex((i) => i.text === 'Downloads')
  const out = items.slice()
  out.splice(at === -1 ? out.length : at, 0, group)
  return out
}
