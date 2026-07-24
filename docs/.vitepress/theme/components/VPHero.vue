<script setup>
import { VPButton } from 'vitepress/theme'
import { computed } from 'vue'

const props = defineProps({
  name: String,
  text: String,
  tagline: String,
  image: Object,
  actions: Array
})

const heroActions = computed(() => props.actions ?? [])
</script>

<template>
  <div class="VPHero">
    <div class="main">
      <h1 v-if="name" class="name">{{ name }}</h1>
      <p v-if="text" class="text">{{ text }}</p>
      <p v-if="tagline" class="tagline">{{ tagline }}</p>

      <div class="actions">
        <template v-for="action in heroActions" :key="action.text">
          <a
            v-if="action.target"
            class="VPButton medium"
            :class="action.theme"
            :href="action.link"
            :target="action.target"
            rel="noopener noreferrer"
          >
            {{ action.text }}
          </a>

          <VPButton
            v-else
            :theme="action.theme"
            :href="action.link"
          >
            {{ action.text }}
          </VPButton>
        </template>
      </div>
    </div>
  </div>
</template>