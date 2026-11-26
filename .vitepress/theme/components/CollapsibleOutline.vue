<script setup lang="ts">
// The "On this page" outline, with every heading that has subheadings folding away.
// Replaces VitePress's VPDocAsideOutline through a Vite alias in config.mts. Same component on
// every lab site; it works on VitePress 1 and 2 (getHeaders and resolveTitle are in both).
//
// The group holding the heading you're reading opens by itself, so the marker always has a
// visible link to sit on; the others stay folded unless you open them with the chevron.
// What you open or close by hand is kept until you scroll into another top-level section.
import { onContentUpdated, useData } from 'vitepress'
import { computed, nextTick, onMounted, onUnmounted, provide, reactive, ref, shallowRef, watch } from 'vue'
import { getHeaders, resolveTitle, type MenuItem } from 'vitepress/dist/client/theme-default/composables/outline'
import OutlineItem from './CollapsibleOutlineItem.vue'

const { frontmatter, theme, site } = useData()
const headers = shallowRef<MenuItem[]>([])
const active = ref<string | null>(null)
const manualOpen = reactive(new Set<string>())
const manualClosed = reactive(new Set<string>())
const container = ref<HTMLElement>()
const marker = ref<HTMLElement>()

onContentUpdated(() => {
  headers.value = getHeaders(frontmatter.value.outline ?? theme.value.outline)
  manualOpen.clear()
  manualClosed.clear()
  update()
})

// Each link's chain of parent links, for opening the active one's groups
const parents = computed(() => {
  const map = new Map<string, string[]>()
  const walk = (items: MenuItem[], chain: string[]) => {
    for (const h of items) {
      map.set(h.link, chain)
      if (h.children?.length) walk(h.children, [...chain, h.link])
    }
  }
  walk(headers.value, [])
  return map
})
const topOf = (link: string | null) => (link ? (parents.value.get(link)?.[0] ?? link) : null)

function isOpen(link: string) {
  if (manualClosed.has(link)) return false
  if (manualOpen.has(link)) return true
  return !!active.value && (parents.value.get(active.value) ?? []).includes(link)
}
function toggle(link: string) {
  if (isOpen(link)) { manualOpen.delete(link); manualClosed.add(link) }
  else { manualClosed.delete(link); manualOpen.add(link) }
  nextTick(placeMarker)
}
provide('outline', { isOpen, toggle, active })

// Which heading is being read: the last one above the top of the window, as VitePress does it
function update() {
  const offset = typeof site.value.scrollOffset === 'number' ? site.value.scrollOffset : 134
  const all: { link: string; top: number }[] = []
  const walk = (items: MenuItem[]) => {
    for (const h of items) {
      const el = document.getElementById(decodeURIComponent(h.link.slice(1)))
      if (el) all.push({ link: h.link, top: el.getBoundingClientRect().top + window.scrollY })
      if (h.children?.length) walk(h.children)
    }
  }
  walk(headers.value)
  all.sort((a, b) => a.top - b.top)
  let now: string | null = null
  const atBottom = Math.abs(window.scrollY + window.innerHeight - document.body.offsetHeight) < 1
  if (window.scrollY >= 1 && all.length) {
    if (atBottom) now = all[all.length - 1].link
    else for (const h of all) { if (h.top > window.scrollY + offset + 4) break; now = h.link }
  }
  if (topOf(now) !== topOf(active.value)) { manualOpen.clear(); manualClosed.clear() }
  active.value = now
  nextTick(placeMarker)
}

function placeMarker() {
  const m = marker.value, c = container.value
  if (!m || !c) return
  const link = active.value ? c.querySelector<HTMLElement>(`a[href="${CSS.escape(decodeURIComponent(active.value))}"]`) : null
  if (link && link.offsetParent) {
    const top = link.getBoundingClientRect().top - c.getBoundingClientRect().top
    m.style.top = top + 7 + 'px'
    m.style.opacity = '1'
  } else {
    m.style.opacity = '0'
  }
}

let timer: number | undefined
function onScroll() {
  if (timer) return
  timer = window.setTimeout(() => { timer = undefined; update() }, 100)
}
onMounted(() => { requestAnimationFrame(update); window.addEventListener('scroll', onScroll, { passive: true }) })
onUnmounted(() => window.removeEventListener('scroll', onScroll))
watch(headers, () => nextTick(update))
</script>

<template>
  <nav aria-labelledby="doc-outline-aria-label" class="VPDocAsideOutline" :class="{ 'has-outline': headers.length > 0 }">
    <div class="content" ref="container">
      <div class="outline-marker" ref="marker" />
      <div aria-level="2" class="outline-title" id="doc-outline-aria-label" role="heading">{{ resolveTitle(theme) }}</div>
      <OutlineItem :headers="headers" :root="true" />
    </div>
  </nav>
</template>

<style scoped>
.VPDocAsideOutline { display: none }
.VPDocAsideOutline.has-outline { display: block }
.content {
  position: relative;
  border-left: 1px solid var(--vp-c-divider);
  padding-left: 16px;
  font-size: 13px;
  font-weight: 500;
}
.outline-marker {
  position: absolute;
  top: 32px;
  left: -1px;
  z-index: 0;
  opacity: 0;
  width: 2px;
  border-radius: 2px;
  height: 18px;
  background-color: var(--vp-c-brand-1);
  transition: top 0.25s cubic-bezier(0, 1, 0.5, 1), background-color 0.5s, opacity 0.25s;
}
.outline-title { line-height: 32px; font-size: 14px; font-weight: 600 }
</style>
