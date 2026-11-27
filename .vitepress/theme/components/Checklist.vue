<script setup lang="ts">
// A checklist students tick as they go: the markdown list inside it gets a checkbox per item.
// Ticks are kept in this browser (localStorage), so a reload doesn't lose them. Lab PCs are
// shared between sections, so "Clear all" unticks every checklist on the page at once.
//
//   <Checklist id="activity" title="Activity checklist">
//
//   - Did you see ...?
//
//   </Checklist>
import { ref, onMounted, onBeforeUnmount } from "vue";

const props = defineProps<{ id: string; title?: string }>();
const root = ref<HTMLElement>();
const done = ref(0);
const total = ref(0);
let boxes: HTMLInputElement[] = [];

const key = () => `checklist:${location.pathname}:${props.id}`;

function save() {
  try {
    localStorage.setItem(key(), JSON.stringify(boxes.map((b) => b.checked)));
  } catch {}
}

function count() {
  done.value = boxes.filter((b) => b.checked).length;
}

function clear() {
  boxes.forEach((b) => (b.checked = false));
  try {
    localStorage.removeItem(key());
  } catch {}
  count();
}

function clearPage() {
  window.dispatchEvent(new CustomEvent("checklist-clear"));
}

onMounted(() => {
  let saved: boolean[] = [];
  try {
    saved = JSON.parse(localStorage.getItem(key()) || "[]");
  } catch {}
  const items = root.value!.querySelectorAll<HTMLLIElement>(".checklist-body > ul > li");
  items.forEach((li, i) => {
    const box = document.createElement("input");
    box.type = "checkbox";
    box.checked = !!saved[i];
    box.addEventListener("change", () => {
      save();
      count();
    });
    const text = document.createElement("span");
    text.append(...Array.from(li.childNodes));
    const label = document.createElement("label");
    label.append(box, text);
    li.append(label);
    boxes.push(box);
  });
  total.value = boxes.length;
  count();
  window.addEventListener("checklist-clear", clear);
});

onBeforeUnmount(() => window.removeEventListener("checklist-clear", clear));
</script>

<template>
  <div ref="root" class="checklist" :class="{ complete: total && done === total }">
    <div class="checklist-head">
      <span class="checklist-title">{{ title || "Checklist" }}</span>
      <span class="checklist-count">{{ done }} / {{ total }}</span>
      <button type="button" class="checklist-clear" title="Untick every checklist on this page" @click="clearPage">
        Clear all
      </button>
    </div>
    <div class="checklist-body"><slot /></div>
  </div>
</template>

<style>
.checklist {
  margin: 24px 0;
  border: 1px solid var(--vp-c-divider);
  border-left: 4px solid var(--vp-c-brand-1);
  border-radius: 8px;
  background: var(--vp-c-bg-soft);
}
.checklist.complete {
  border-left-color: var(--vp-c-green-1);
}
.checklist-head {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 10px 16px;
  border-bottom: 1px solid var(--vp-c-divider);
}
.checklist-title {
  font-weight: 600;
}
.checklist-count {
  color: var(--vp-c-text-2);
  font-size: 14px;
  font-variant-numeric: tabular-nums;
}
.checklist-clear {
  margin-left: auto;
  padding: 2px 12px;
  border: 1px solid var(--vp-c-divider);
  border-radius: 6px;
  font-size: 13px;
  color: var(--vp-c-text-2);
  background: var(--vp-c-bg);
}
.checklist-clear:hover {
  color: var(--vp-c-text-1);
  border-color: var(--vp-c-brand-1);
}
.vp-doc .checklist-body ul {
  list-style: none;
  margin: 0;
  padding: 8px 16px;
}
.vp-doc .checklist-body li {
  margin: 0;
}
.vp-doc .checklist-body li + li {
  border-top: 1px solid var(--vp-c-divider);
}
.checklist-body label {
  display: flex;
  gap: 12px;
  align-items: baseline;
  padding: 8px 0;
  cursor: pointer;
}
.checklist-body input[type="checkbox"] {
  flex: none;
  width: 16px;
  height: 16px;
  accent-color: var(--vp-c-brand-1);
  transform: translateY(2px);
  cursor: pointer;
}
.checklist-body label:has(input:checked) {
  color: var(--vp-c-text-2);
}
</style>
