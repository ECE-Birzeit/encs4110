// Two markdown-it plugins for register-heavy experiment pages, both run at build time.
//
// registerDiagrams: a two-column table whose header is exactly | Bits | Field | becomes an inline
//   SVG bit diagram (bits 31-16 above, 15-0 below; four rows of 8 on phones), drawn like the
//   manual's bytefield figures.
//   Colours come from the theme's CSS variables (see .reg-diagram in custom.css), so the same SVG
//   follows light and dark mode. A table whose bits don't cover 31-0 exactly is left as a table.
//
// datasheetRefs: a ### or #### heading that names a register (GPTMCTL, (STCTRL), ADCSSMUXn, or a
//   **Register:** `RCGCGPIO` line right under it) gets a "Datasheet p. N" link to that register's
//   page in the local datasheet copy. (The chapter as a whole is the "Datasheet (PDF)" button at the
//   top of the page, from front matter `datasheet:`; see JumpToProcedure.vue.) Page numbers come from
//   datasheet-pages.json (scripts/datasheet-pages.py); printed page numbers equal PDF page indexes.
import type MarkdownIt from 'markdown-it'

type Token = ReturnType<MarkdownIt['parse']>[number]

const esc = (s: string) => s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;')

interface Field { hi: number; lo: number; name: string }

function parseBits(s: string): [number, number] | null {
  const r = s.match(/^(\d+)\s*[–-]\s*(\d+)$/)
  if (r) return [Number(r[1]), Number(r[2])]
  const one = s.match(/^(\d+)$/)
  return one ? [Number(one[1]), Number(one[1])] : null
}

const CELL = 40, HEAD = 16, BOX = 32, GAP = 10
const ROW = HEAD + BOX

// perRow 16: bits 31-16 over 15-0, for wide screens. perRow 8: four rows, for phones.
function svg(fields: Field[], label: string, perRow: 16 | 8, cls: string): string {
  const rows = 32 / perRow
  const W = perRow * CELL, H = rows * ROW + (rows - 1) * GAP
  const out: string[] = []
  out.push(`<svg class="${cls}" viewBox="-1 -1 ${W + 2} ${H + 2}" role="img" aria-label="${esc(label)}">`)
  for (let k = 0; k < rows; k++) {
    const r = 31 - k * perRow, top = k * (ROW + GAP)
    const bot = r - perRow + 1
    for (let b = r; b >= bot; b--) {
      out.push(`<text class="n" x="${(r - b) * CELL + CELL / 2}" y="${top + 11}">${b}</text>`)
    }
    for (const f of fields) {
      const hi = Math.min(f.hi, r), lo = Math.max(f.lo, bot)
      if (hi < lo) continue
      const x = (r - hi) * CELL, w = (hi - lo + 1) * CELL
      const reserved = /^reserved$/i.test(f.name)
      out.push(`<rect class="${reserved ? 'r' : 'f'}" x="${x}" y="${top + HEAD}" width="${w}" height="${BOX}"/>`)
      // A one- or two-bit "reserved" would be too small to read; the grey fill says it already
      if (reserved && hi - lo < 2) continue
      const room = w - 6
      const size = Math.min(12, room / (0.6 * f.name.length))
      const fit = size < 7 ? ` textLength="${room}" lengthAdjust="spacingAndGlyphs"` : ''
      out.push(`<text class="${reserved ? 'lr' : 'l'}" x="${x + w / 2}" y="${top + HEAD + BOX / 2}" font-size="${Math.max(size, 7).toFixed(1)}"${fit}>${esc(f.name)}</text>`)
    }
  }
  out.push('</svg>')
  return out.join('')
}

export function registerDiagrams(md: MarkdownIt) {
  md.core.ruler.push('register_diagrams', (state) => {
    const t = state.tokens
    for (let i = 0; i < t.length; i++) {
      if (t[i].type !== 'table_open') continue
      const close = t.findIndex((x, j) => j > i && x.type === 'table_close')
      const slice = t.slice(i, close + 1)
      const cells = (open: string) => {
        const rows: string[][] = []
        let row: string[] | null = null
        for (let k = 0; k < slice.length; k++) {
          if (slice[k].type === 'tr_open') row = []
          else if (slice[k].type === 'tr_close' && row) { if (row.length) rows.push(row); row = null }
          else if (slice[k].type === open && row) row.push(slice[k + 1].content.trim())
        }
        return rows
      }
      const head = cells('th_open')
      if (head.length !== 1 || head[0].join('|') !== 'Bits|Field') continue
      const fields: Field[] = []
      let ok = true
      for (const [bits, name] of cells('td_open')) {
        const p = parseBits(bits)
        if (!p || name === undefined) { ok = false; break }
        fields.push({ hi: p[0], lo: p[1], name })
      }
      // Must cover 31..0 exactly, no gaps or overlaps, else keep the table
      const seen = new Array(32).fill(0)
      for (const f of fields) for (let b = f.lo; b <= f.hi; b++) seen[b]++
      if (!ok || seen.some((n) => n !== 1)) {
        console.warn(`  register diagram: kept a table in ${state.env?.relativePath}, its bits don't cover 31-0 once each`)
        continue
      }
      const label = 'Register bits: ' + fields.map((f) => `${f.hi === f.lo ? f.hi : `${f.hi}–${f.lo}`} ${f.name}`).join(', ')
      const tok = new state.Token('html_block', '', 0)
      tok.content = `<figure class="reg-diagram">${svg(fields, label, 16, 'wide')}${svg(fields, label, 8, 'narrow')}</figure>\n`
      tok.map = t[i].map
      t.splice(i, close - i + 1, tok)
    }
  })
}

export interface DatasheetOptions {
  pdf: string // e.g. '/datasheets/tm4c123gh6pm.pdf'
  name: string // for the link title, e.g. 'TM4C123GH6PM datasheet'
  pages: Record<string, number> // register mnemonic -> page
  alias?: Record<string, string> // names used on the site -> datasheet mnemonic
}

export function datasheetRefs(md: MarkdownIt, o: DatasheetOptions) {
  const lookup = (word: string): [string, number] | null => {
    const w = o.alias?.[word] ?? word
    if (o.pages[w]) return [w, o.pages[w]]
    // ADCSSMUXn -> ADCSSMUX0: the n-th copies share a layout, the first one stands for all
    if (/n$/.test(w) && o.pages[w.slice(0, -1) + '0']) return [w.slice(0, -1) + '0', o.pages[w.slice(0, -1) + '0']]
    return null
  }
  const find = (text: string) => {
    for (const word of text.match(/[A-Z][A-Za-z0-9_]{2,}/g) ?? []) {
      const hit = lookup(word)
      if (hit) return hit
    }
    return null
  }
  const badge = (state: any, page: number, title: string) => {
    const tok = new state.Token('html_inline', '', 0)
    tok.content = ` <a class="ds-ref ignore-header" href="${o.pdf}#page=${page}" target="_blank" rel="noopener" title="${esc(title)}">Datasheet p.&nbsp;${page}</a>`
    return tok
  }
  md.core.ruler.push('datasheet_refs', (state) => {
    if (!/^experiments\/exp\d+-/.test(state.env?.relativePath ?? '')) return
    const t = state.tokens
    for (let i = 0; i < t.length; i++) {
      if (t[i].type !== 'heading_open') continue
      const inline = t[i + 1]
      const text = inline.content
      if (t[i].tag !== 'h3' && t[i].tag !== 'h4') continue
      let hit = find(text)
      // "#### Step 1 — Enable the GPIO Port Clock" followed by "**Register:** `RCGCGPIO` — ..."
      if (!hit && t[i + 3]?.type === 'paragraph_open' && /^\*\*Register:\*\*/.test(t[i + 4]?.content ?? '')) {
        const code = (t[i + 4].children ?? []).filter((c: Token) => c.type === 'code_inline').map((c: Token) => c.content).join(' ')
        hit = find(code)
      }
      if (hit) inline.children!.push(badge(state, hit[1], `${o.name}, page ${hit[1]}: ${hit[0]}`))
    }
  })
}
