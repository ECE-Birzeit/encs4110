<script setup lang="ts">
import { onBeforeMount, ref } from 'vue'

const offlineReady = ref(false)
function onOfflineReady() {
  offlineReady.value = true
}
async function close() {
  offlineReady.value = false
}

// GitHub Pages serves sw.js with a fixed Cache-Control (no way to set
// per-file headers there), which can let a plain reload be satisfied
// straight from the browser's own HTTP cache without ever checking the
// server for a newer version. Poll for a genuinely fresh copy on an
// interval instead, explicitly bypassing cache -- this is vite-plugin-pwa's
// own documented workaround for hosts with this exact limitation.
const CHECK_INTERVAL_MS = 30 * 1000

function registerPeriodicUpdateCheck(swUrl: string, registration: ServiceWorkerRegistration) {
  setInterval(async () => {
    if (registration.installing || !navigator.onLine) return
    const resp = await fetch(swUrl, {
      cache: 'no-store',
      headers: { 'cache-control': 'no-cache' },
    })
    if (resp?.status === 200) await registration.update()
  }, CHECK_INTERVAL_MS)
}

onBeforeMount(async () => {
  const { registerSW } = await import('virtual:pwa-register')
  registerSW({
    immediate: true,
    onOfflineReady,
    // registerType: 'autoUpdate' (see config.mts) makes a waiting service
    // worker activate on its own -- this callback doesn't need to do
    // anything, but vite-plugin-pwa's documented auto-update recipe keeps
    // it defined rather than omitted.
    onNeedRefresh() {},
    onRegisteredSW(swUrl, registration) {
      console.info('Service Worker registered')
      if (registration) registerPeriodicUpdateCheck(swUrl, registration)
    },
    onRegisterError(e) {
      console.error('Service Worker registration error!', e)
    },
  })
})
</script>

<template>
  <template v-if="offlineReady">
    <div
      class="pwa-toast"
      role="alertdialog"
      aria-labelledby="pwa-message"
    >
      <div id="pwa-message" class="mb-3">
        App ready to work offline
      </div>
      <button
        type="button"
        class="pwa-cancel"
        @click="close"
      >
        Close
      </button>
    </div>
  </template>
</template>

<style>
    .pwa-toast {
        position: fixed;
        right: 0;
        bottom: 0;
        margin: 16px;
        padding: 12px;
        border: 1px solid #8885;
        border-radius: 4px;
        z-index: 100;
        text-align: left;
        box-shadow: 3px 4px 5px 0 #8885;
        background-color: white;
    }
    .pwa-toast #pwa-message {
        margin-bottom: 8px;
    }
    .pwa-toast button {
        border: 1px solid #8885;
        outline: none;
        margin-right: 5px;
        border-radius: 2px;
        padding: 3px 10px;
    }
</style>