// `<<< @/snippets/file.asm#name` for Keil assembly. VitePress's own snippet regions only know
// comment styles such as // and #, not armasm's `;`, so this block rule runs first for .asm
// files and handles `; #region name` ... `; #endregion name`. Every region marker inside the
// slice is dropped, so a `full` region around the whole file shows the program without them.
import { readFileSync } from 'node:fs'
import { join } from 'node:path'
import type MarkdownIt from 'markdown-it'

const LINE = /^<<<\s+@\/(\S+\.asm)#([\w-]+)(?:\{([\w-]*)\})?\s*$/
const MARK = /^\s*;\s*#(end)?region\b/

export function asmRegions(md: MarkdownIt, srcDir: string) {
  // Ahead of VitePress's own `snippet` rule, which would otherwise take the line first
  const rule = (state: any, startLine: number, _endLine: number, silent: boolean) => {
    const start = state.bMarks[startLine] + state.tShift[startLine]
    const m = state.src.slice(start, state.eMarks[startLine]).match(LINE)
    if (!m) return false
    if (silent) return true
    const [, file, name, lang] = m
    let content: string
    try {
      const lines = readFileSync(join(srcDir, file), 'utf8').replace(/\r\n/g, '\n').split('\n')
      const from = lines.findIndex((l) => new RegExp(`^\\s*;\\s*#region ${name}\\s*$`).test(l))
      const to = lines.findIndex((l, i) => i > from && new RegExp(`^\\s*;\\s*#endregion ${name}\\s*$`).test(l))
      content = from === -1 || to === -1
        ? `Region "${name}" not found in ${file}`
        : lines.slice(from + 1, to).filter((l) => !MARK.test(l)).join('\n') + '\n'
    } catch {
      content = `Code snippet path not found: ${file}`
    }
    const token = state.push('fence', 'code', 0)
    token.info = lang || 'asm'
    token.content = content
    token.map = [startLine, startLine + 1]
    state.line = startLine + 1
    return true
  }
  const names = (md.block.ruler as any).__rules__.map((r: { name: string }) => r.name)
  md.block.ruler.before(names.includes('snippet') ? 'snippet' : 'fence', 'asm_region', rule)
}
