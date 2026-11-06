<script setup lang="ts">
// "Continue where you left off".
// mode="track" (in layout-bottom): remembers the last experiment page and section read.
// mode="card" (in home-hero-actions-after): shows a link back to it on the home page.
// The entry lives in a cookie on .birzeit.me, so birzeit.me can show it too.
import { ref, watch, onMounted, onBeforeUnmount, nextTick } from 'vue'
import { useRoute, useData } from 'vitepress'

const props = defineProps<{ lab: string; mode: 'track' | 'card' }>()

type Entry = { lab: string; n: string; title: string; section?: string; url: string; t: number }

const KEY = `ece-last-${props.lab}`
const MAX_AGE = 60 * 60 * 24 * 180 // 180 days
const EXP = /\/experiments\/(?:exp)?(\d+)/

const route = useRoute()
const { page } = useData()
const last = ref<Entry | null>(null)

function read(): Entry | null {
  const m = document.cookie.match(new RegExp('(?:^|; )' + KEY + '=([^;]*)'))
  if (!m) return null
  try {
    const e = JSON.parse(decodeURIComponent(m[1]))
    return e && typeof e.url === 'string' && e.url.startsWith(location.protocol + '//') ? e : null
  } catch {
    return null
  }
}

function write(e: Entry) {
  const domain = location.hostname.endsWith('birzeit.me') ? '; domain=.birzeit.me' : ''
  const secure = location.protocol === 'https:' ? '; Secure' : ''
  document.cookie = `${KEY}=${encodeURIComponent(JSON.stringify(e))}; path=/; max-age=${MAX_AGE}; SameSite=Lax${secure}${domain}`
}

function clean(text: string) {
  return text.replace(/[​#]/g, '').replace(/\s+/g, ' ').trim()
}

// The last h2 scrolled past the top of the viewport.
function currentSection(): { id: string; text: string } | null {
  let found: HTMLElement | null = null
  for (const h of document.querySelectorAll<HTMLElement>('.vp-doc h2[id]')) {
    if (h.getBoundingClientRect().top > 140) break
    found = h
  }
  return found ? { id: found.id, text: clean(found.textContent || '') } : null
}

function save() {
  const m = route.path.match(EXP)
  if (!m) return
  const s = currentSection()
  write({
    lab: props.lab,
    n: m[1],
    title: clean(page.value.title || '').replace(/^\d+\.\s*/, ''),
    section: s?.text,
    url: location.origin + route.path + (s ? '#' + s.id : ''),
    t: Date.now(),
  })
}

let timer: ReturnType<typeof setTimeout> | undefined
function onScroll() {
  clearTimeout(timer)
  timer = setTimeout(save, 500)
}
function onHide() {
  if (document.visibilityState === 'hidden') save()
}

onMounted(() => {
  if (props.mode === 'card') {
    last.value = read()
    return
  }
  nextTick(save)
  window.addEventListener('scroll', onScroll, { passive: true })
  document.addEventListener('visibilitychange', onHide)
})

onBeforeUnmount(() => {
  clearTimeout(timer)
  window.removeEventListener('scroll', onScroll)
  document.removeEventListener('visibilitychange', onHide)
})

if (props.mode === 'track') {
  watch(() => route.path, () => nextTick(save))
}

</script>

<template>
  <div v-if="mode === 'card' && last" class="continue">
    <a class="continue-link" :href="last.url">
      <span class="continue-kicker">Continue where you left off</span>
      <span class="continue-title">Exp {{ last.n }} · {{ last.title }}</span>
      <span v-if="last.section" class="continue-section">{{ last.section }}</span>
      <span class="continue-arrow" aria-hidden="true">→</span>
    </a>
  </div>
</template>

<style scoped>
.continue {
  position: relative;
  display: flex;
  max-width: 520px;
  margin-top: 24px;
}
.continue-link {
  flex: 1;
  display: grid;
  grid-template-columns: 1fr auto;
  column-gap: 16px;
  row-gap: 2px;
  padding: 14px 20px 14px 18px;
  border: 1px solid var(--vp-c-brand-2);
  border-radius: 12px;
  background: var(--vp-c-brand-soft);
  color: var(--vp-c-text-1);
  text-decoration: none;
  transition: background-color 0.2s;
}
.continue-link:hover {
  background: color-mix(in srgb, var(--vp-c-brand-soft) 60%, var(--vp-c-brand-2) 15%);
}
.continue-kicker {
  grid-column: 1;
  font-size: 12px;
  font-weight: 500;
  color: var(--vp-c-brand-1);
}
.continue-title {
  grid-column: 1;
  font-size: 16px;
  font-weight: 600;
  line-height: 1.35;
}
.continue-section {
  grid-column: 1;
  font-size: 13px;
  color: var(--vp-c-text-2);
}
.continue-arrow {
  grid-column: 2;
  grid-row: 1 / span 3;
  align-self: center;
  font-size: 20px;
  color: var(--vp-c-brand-1);
}
@media (max-width: 959px) {
  .continue {
    margin-inline: auto;
  }
}
</style>
