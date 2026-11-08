<script setup lang="ts">
// Buttons at the top of an experiment page: "Jump to the procedure" (the first section after
// Background: Examples, Configuration Steps, ...) and, when the page's front matter has a
// `manual:` link, "Lab manual (PDF)" opening the experiment's chapter. Same component on every lab site.
import { computed, nextTick, onMounted, ref, watch } from 'vue'
import { useData, useRoute, withBase } from 'vitepress'

const route = useRoute()
const { frontmatter } = useData()
const target = ref<{ id: string } | null>(null)
const IDS = ['procedure', 'examples', 'example', 'lab-work', 'tasks']
const isExperiment = computed(() => /\/experiments\/exp\d+-/.test(route.path))
const manual = computed(() => (isExperiment.value && frontmatter.value.manual ? withBase(frontmatter.value.manual) : ''))

function find() {
  target.value = null
  if (!isExperiment.value) return
  const h2s = Array.from(document.querySelectorAll<HTMLElement>('.vp-doc h2[id]'))
  const bg = h2s.findIndex((h) => h.id === 'background')
  if (bg !== -1 && h2s[bg + 1]) { target.value = { id: h2s[bg + 1].id }; return }
  for (const id of IDS) {
    const h = h2s.find((x) => x.id === id || x.id.startsWith(id + '-'))
    if (h) { target.value = { id: h.id }; return }
  }
}
onMounted(find)
watch(() => route.path, () => nextTick(find))

function go(e: MouseEvent) {
  const h = target.value && document.getElementById(target.value.id)
  if (!h) return
  e.preventDefault()
  history.pushState(null, '', '#' + target.value!.id)
  h.scrollIntoView({ behavior: matchMedia('(prefers-reduced-motion: reduce)').matches ? 'auto' : 'smooth', block: 'start' })
}
</script>

<template>
  <div v-if="target || manual" class="jump-row">
    <a v-if="manual" class="jump alt" :href="manual" target="_blank" rel="noopener">Lab manual (PDF) <span aria-hidden="true">↗</span></a>
    <a v-if="target" class="jump" :href="'#' + target.id" @click="go">Jump to the procedure <span aria-hidden="true">↓</span></a>
  </div>
</template>

<style scoped>
.jump-row { display: flex; flex-wrap: wrap; justify-content: flex-end; gap: 8px; margin-bottom: 8px }
.jump {
  display: inline-flex; align-items: center; gap: 8px; min-height: 36px; padding: 0 14px;
  border: 1px solid var(--vp-c-brand-1); border-radius: 18px; color: var(--vp-c-brand-1);
  font-size: 13.5px; font-weight: 600; text-decoration: none; transition: background .2s, color .2s, border-color .2s;
}
.jump:hover { background: var(--vp-c-brand-1); color: var(--vp-c-bg) }
.jump.alt { border-color: var(--vp-c-divider); color: var(--vp-c-text-2) }
.jump.alt:hover { border-color: var(--vp-c-text-1); background: transparent; color: var(--vp-c-text-1) }
.jump span { transition: transform .3s }
.jump:hover span { transform: translateY(2px) }
.jump.alt:hover span { transform: translate(2px, -2px) }
@media (prefers-reduced-motion: reduce) { .jump span { transition: none } }
</style>
