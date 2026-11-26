<script setup lang="ts">
// The board pinout, one tap away on every experiment page: a floating button opens a sheet with
// the top and bottom views of each board. JumpToProcedure.vue's "Pinout" button opens the same
// sheet (both share ../pinout.ts). The full page with tables is /setup/pinout.
// Same component on every lab site that has a pinout page; the site passes its boards, whose
// drawings are /images/pinout/<key>-top.svg and <key>-bottom.svg, `width` px wide.
import { computed, nextTick, onMounted, onUnmounted, ref, watch } from 'vue'
import { useRoute, withBase } from 'vitepress'
import { pinoutOpen } from '../pinout'

const props = defineProps<{ boards: { key: string; label: string }[]; width: number }>()
const route = useRoute()
const onExperiment = computed(() => /\/(experiments|preview)\/exp\d+-/.test(route.path))
const view = ref<'top' | 'bottom'>('top')
const board = ref(props.boards[0].key)
const zoomed = ref(false)
const src = computed(() => withBase(`/images/pinout/${board.value}-${view.value}.svg`))
const boardLabel = computed(() => props.boards.find((b) => b.key === board.value)?.label ?? '')

// Remember the reader's board between pages (a convenience: it can fail in private windows)
const STORE = 'pinout-board'
function pick(key: string) {
  board.value = key
  try { localStorage.setItem(STORE, key) } catch {}
}
const closeBtn = ref<HTMLButtonElement>()
let opener: HTMLElement | null = null
const open = () => { pinoutOpen.value = true }
const close = () => { pinoutOpen.value = false }

watch(pinoutOpen, (open) => {
  if (open) {
    opener = document.activeElement as HTMLElement | null
    zoomed.value = window.innerWidth < 900          // Fitted to a phone, the labels are too small to read
    document.documentElement.style.overflow = 'hidden'
    nextTick(() => closeBtn.value?.focus())
  } else {
    document.documentElement.style.overflow = ''
    zoomed.value = false
    opener?.focus()
  }
})
watch(() => route.path, () => { pinoutOpen.value = false })

function onKey(e: KeyboardEvent) {
  if (pinoutOpen.value && e.key === 'Escape') pinoutOpen.value = false
}
onMounted(() => {
  window.addEventListener('keydown', onKey)
  try {
    const saved = localStorage.getItem(STORE)
    if (saved && props.boards.some((b) => b.key === saved)) board.value = saved
  } catch {}
})
onUnmounted(() => window.removeEventListener('keydown', onKey))
</script>

<template>
  <button
    v-if="onExperiment"
    class="pinout-fab"
    type="button"
    aria-haspopup="dialog"
    @click="open"
  >
    <svg viewBox="0 0 24 24" width="18" height="18" aria-hidden="true"><rect x="6" y="6" width="12" height="12" rx="2" fill="none" stroke="currentColor" stroke-width="1.8"/><path d="M9 2v4M15 2v4M9 18v4M15 18v4M2 9h4M2 15h4M18 9h4M18 15h4" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"/></svg>
    <span>Pinout</span>
  </button>

  <Teleport to="body">
    <div v-if="pinoutOpen" class="pinout-backdrop" @click.self="close">
      <div class="pinout-sheet" role="dialog" aria-modal="true" aria-labelledby="pinout-title">
        <header class="pinout-head">
          <h2 id="pinout-title">Board pinout</h2>
          <div v-if="boards.length > 1" class="pinout-views" role="group" aria-label="Board">
            <button v-for="b in boards" :key="b.key" type="button" :aria-pressed="board === b.key" @click="pick(b.key)">{{ b.label }}</button>
          </div>
          <div class="pinout-views" role="group" aria-label="View">
            <button type="button" :aria-pressed="view === 'top'" @click="view = 'top'">Top</button>
            <button type="button" :aria-pressed="view === 'bottom'" @click="view = 'bottom'">Bottom</button>
          </div>
          <button type="button" class="pinout-text-btn" :aria-pressed="zoomed" @click="zoomed = !zoomed">{{ zoomed ? 'Fit' : 'Zoom' }}</button>
          <a class="pinout-text-btn" :href="withBase('/setup/pinout')" @click="close">Full page</a>
          <button ref="closeBtn" type="button" class="pinout-close" aria-label="Close" @click="close">
            <svg viewBox="0 0 16 16" width="16" height="16" aria-hidden="true"><path d="M4 4l8 8M12 4l-8 8" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"/></svg>
          </button>
        </header>
        <div class="pinout-body" :class="{ zoomed }">
          <img :src="src" :alt="`${boardLabel} header pinout, ${view} view`" :style="zoomed ? { width: width + 'px' } : undefined" @click="zoomed = !zoomed" />
        </div>
        <p class="pinout-hint">{{ boardLabel }}, {{ view === 'top' ? 'top view: the board the right way up.' : 'bottom view: the board flipped left-to-right.' }} Click the drawing to zoom.</p>
      </div>
    </div>
  </Teleport>
</template>

<style scoped>
.pinout-fab {
  position: fixed;
  right: 24px;
  bottom: 24px;
  z-index: 30;
  display: inline-flex;
  align-items: center;
  gap: 8px;
  min-height: 44px;
  padding: 0 16px;
  border: 1px solid var(--vp-c-divider);
  border-radius: 22px;
  background: var(--vp-c-bg-elv);
  color: var(--vp-c-text-1);
  font-size: 14px;
  font-weight: 600;
  box-shadow: var(--vp-shadow-3);
  transition: border-color .2s, color .2s;
}
.pinout-fab:hover { border-color: var(--vp-c-brand-1); color: var(--vp-c-brand-1) }

.pinout-backdrop {
  position: fixed;
  inset: 0;
  z-index: 200;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 16px;
  background: rgba(0, 0, 0, .55);
}
.pinout-sheet {
  display: flex;
  flex-direction: column;
  width: min(1600px, 100%);
  max-height: 100%;
  border-radius: 12px;
  background: var(--vp-c-bg);
  box-shadow: var(--vp-shadow-5);
  overflow: hidden;
}
.pinout-head {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 8px 12px;
  padding: 12px 16px;
  border-bottom: 1px solid var(--vp-c-divider);
}
.pinout-head h2 { margin: 0 auto 0 0; font-size: 16px; font-weight: 700 }
.pinout-views { display: inline-flex; border: 1px solid var(--vp-c-divider); border-radius: 8px; overflow: hidden }
.pinout-views button { padding: 4px 12px; font-size: 13px; font-weight: 600; color: var(--vp-c-text-2) }
.pinout-views button[aria-pressed='true'] { background: var(--vp-c-brand-soft); color: var(--vp-c-brand-1) }
.pinout-text-btn {
  padding: 4px 10px;
  border: 1px solid var(--vp-c-divider);
  border-radius: 8px;
  font-size: 13px;
  font-weight: 600;
  color: var(--vp-c-text-2);
  text-decoration: none;
}
.pinout-text-btn:hover { color: var(--vp-c-text-1); border-color: var(--vp-c-text-3) }
.pinout-close { display: inline-flex; padding: 6px; border-radius: 8px; color: var(--vp-c-text-2) }
.pinout-close:hover { background: var(--vp-c-default-soft); color: var(--vp-c-text-1) }
.pinout-body { overflow: auto; background: var(--vp-c-bg); min-height: 0 }
.pinout-body img { display: block; width: 100%; height: auto; cursor: zoom-in }
.pinout-body.zoomed img { max-width: none; cursor: zoom-out }
.pinout-body.zoomed { flex: 1 }
.pinout-hint { margin: 0; padding: 8px 16px; font-size: 12.5px; color: var(--vp-c-text-2); border-top: 1px solid var(--vp-c-divider) }
@media (max-width: 640px) {
  .pinout-backdrop { padding: 0 }
  .pinout-sheet { border-radius: 0; height: 100% }
  .pinout-fab span { display: none }
  .pinout-fab { padding: 0 13px; right: 16px; bottom: 16px }
}
</style>
