<script setup lang="ts">
// A "Jump to the procedure" button at the top of an experiment page, for students who know the
// background and want the hands-on part. It links to the first section after Background (Examples,
// or Configuration Steps, ...), or failing that to a Procedure, Examples or Lab work heading, and only
// shows when the page has one. Same component on every lab site.
import { nextTick, onMounted, ref, watch } from 'vue'
import { useRoute } from 'vitepress'

const route = useRoute()
const target = ref<{ id: string } | null>(null)
const IDS = ['procedure', 'examples', 'example', 'lab-work', 'tasks']

function find() {
  target.value = null
  if (!/\/experiments\/exp\d+-/.test(route.path)) return
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
  <div v-if="target" class="jump-row">
    <a class="jump" :href="'#' + target.id" @click="go">Jump to the procedure <span aria-hidden="true">↓</span></a>
  </div>
</template>

<style scoped>
.jump-row { display: flex; justify-content: flex-end; margin-bottom: 8px }
.jump {
  display: inline-flex; align-items: center; gap: 8px; min-height: 36px; padding: 0 14px;
  border: 1px solid var(--vp-c-brand-1); border-radius: 18px; color: var(--vp-c-brand-1);
  font-size: 13.5px; font-weight: 600; text-decoration: none; transition: background .2s, color .2s;
}
.jump:hover { background: var(--vp-c-brand-1); color: var(--vp-c-bg) }
.jump span { transition: transform .3s }
.jump:hover span { transform: translateY(2px) }
@media (prefers-reduced-motion: reduce) { .jump span { transition: none } }
</style>
