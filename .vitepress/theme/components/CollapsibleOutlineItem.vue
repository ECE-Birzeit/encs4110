<script setup lang="ts">
// One level of CollapsibleOutline: a link per heading, and a chevron that folds its subheadings.
import { inject, type Ref } from 'vue'
import type { MenuItem } from 'vitepress/dist/client/theme-default/composables/outline'

defineProps<{ headers: MenuItem[]; root?: boolean }>()
const { isOpen, toggle, active } = inject('outline') as {
  isOpen: (link: string) => boolean
  toggle: (link: string) => void
  active: Ref<string | null>
}

function onClick({ target: el }: Event) {
  const id = (el as HTMLAnchorElement).href.split('#')[1]
  document.getElementById(decodeURIComponent(id))?.focus({ preventScroll: true })
}
</script>

<template>
  <ul class="item" :class="root ? 'root' : 'nested'">
    <li v-for="{ children, link, title } in headers" :key="link">
      <div class="row">
        <a class="outline-link" :class="{ active: active === link }" :href="link" :title="title" @click="onClick">{{ title }}</a>
        <button
          v-if="children?.length"
          class="fold"
          type="button"
          :aria-expanded="isOpen(link)"
          :aria-label="(isOpen(link) ? 'Fold ' : 'Unfold ') + title"
          @click="toggle(link)"
        >
          <svg viewBox="0 0 16 16" width="12" height="12" aria-hidden="true" :class="{ open: isOpen(link) }"><path d="M6 4l4 4-4 4" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"/></svg>
        </button>
      </div>
      <CollapsibleOutlineItem v-if="children?.length" v-show="isOpen(link)" :headers="children" />
    </li>
  </ul>
</template>

<style scoped>
.root { position: relative; z-index: 1 }
.nested { padding-left: 13px }
.row { display: flex; align-items: center; gap: 2px }
.outline-link {
  flex: 1;
  min-width: 0;
  display: block;
  line-height: 32px;
  font-size: 14px;
  font-weight: 400;
  color: var(--vp-c-text-2);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  transition: color 0.5s;
}
.outline-link:hover, .outline-link.active { color: var(--vp-c-text-1); transition: color 0.25s }
.fold {
  flex: none;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  width: 24px;
  height: 24px;
  border-radius: 6px;
  color: var(--vp-c-text-3);
  transition: color 0.25s, background-color 0.25s;
}
.fold:hover { color: var(--vp-c-text-1); background-color: var(--vp-c-default-soft) }
.fold svg { transition: transform 0.2s }
.fold svg.open { transform: rotate(90deg) }
@media (prefers-reduced-motion: reduce) { .fold svg { transition: none } }
</style>
