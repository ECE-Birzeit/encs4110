<script setup lang="ts">
import { onBeforeMount, ref } from 'vue'

const offlineReady = ref(false)
function close() {
  offlineReady.value = false
}

// registerType: 'autoUpdate' (see config.mts) makes a new service worker
// activate on its own, and the plugin then reloads every open tab. The site is
// a single-page app, so a page that stays open never navigates and the browser
// never checks for a new version by itself. Check here instead:
// - right away, so every launch looks for a new deploy;
// - when the tab becomes visible or the window gets focus. Safari keeps a page
//   "visible" while other apps' windows cover it, so switching back to an
//   installed web app fires only focus, never visibilitychange;
// - when the network comes back;
// - once an hour, as a fallback (background timers get throttled).
// Checks less than a minute apart are skipped. registration.update() fetches
// sw.js past the HTTP cache, so GitHub Pages' max-age=600 does not hide a new
// version.
const CHECK_INTERVAL_MS = 60 * 60 * 1000
const MIN_GAP_MS = 60 * 1000

function registerUpdateChecks(registration: ServiceWorkerRegistration) {
  let lastCheck = 0
  const check = () => {
    if (registration.installing || !navigator.onLine) return
    if (Date.now() - lastCheck < MIN_GAP_MS) return
    lastCheck = Date.now()
    registration.update().catch(() => {})
  }
  check()
  setInterval(check, CHECK_INTERVAL_MS)
  document.addEventListener('visibilitychange', () => {
    if (document.visibilityState === 'visible') check()
  })
  window.addEventListener('focus', check)
  window.addEventListener('online', check)
}

onBeforeMount(async () => {
  const { registerSW } = await import('virtual:pwa-register')
  registerSW({
    immediate: true,
    onOfflineReady() {
      offlineReady.value = true
    },
    onRegisteredSW(_swUrl, registration) {
      if (registration) registerUpdateChecks(registration)
    },
    onRegisterError(e) {
      console.error('Service worker registration failed', e)
    },
  })
})
</script>

<template>
  <div v-if="offlineReady" class="pwa-toast" role="status">
    <span>Ready to work offline</span>
    <button type="button" @click="close">Close</button>
  </div>
</template>

<style>
.pwa-toast {
  position: fixed;
  right: 16px;
  bottom: 16px;
  z-index: 100;
  display: flex;
  align-items: center;
  gap: 12px;
  max-width: calc(100vw - 32px);
  padding: 10px 12px 10px 16px;
  border: 1px solid var(--vp-c-divider);
  border-radius: 8px;
  background: var(--vp-c-bg-elv);
  color: var(--vp-c-text-1);
  box-shadow: var(--vp-shadow-3);
  font-size: 14px;
}
.pwa-toast button {
  padding: 4px 12px;
  border: 1px solid var(--vp-c-divider);
  border-radius: 6px;
  background: var(--vp-c-bg-soft);
  color: var(--vp-c-text-1);
  font-weight: 500;
}
.pwa-toast button:hover {
  border-color: var(--vp-c-brand-1);
  color: var(--vp-c-brand-1);
}
</style>
