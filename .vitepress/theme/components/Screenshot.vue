<script setup lang="ts">
// A screenshot from public/screenshots/<src>. Until that file exists the
// figure renders as a placeholder that says what to capture and where to save
// it, so the guides can be written ahead of the Windows screenshots.
import { computed } from "vue";
import { withBase } from "vitepress";
import { data as available } from "../screenshots.data";

const props = defineProps<{ src: string; alt: string }>();
const ready = computed(() => available.includes(props.src));
</script>

<template>
  <figure class="screenshot">
    <img v-if="ready" :src="withBase(`/screenshots/${src}`)" :alt="alt" loading="lazy" />
    <div v-else class="screenshot-placeholder" role="img" :aria-label="`Screenshot placeholder: ${alt}`">
      <span class="screenshot-badge">Screenshot placeholder · Windows</span>
      <p class="screenshot-capture"><slot /></p>
      <code>public/screenshots/{{ src }}</code>
    </div>
    <figcaption>{{ alt }}</figcaption>
  </figure>
</template>
