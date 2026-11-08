// .vitepress/theme/index.ts
import { h } from 'vue'
import DefaultTheme from 'vitepress/theme'
import 'virtual:group-icons.css'
import './custom.css'

import RegisterSW from './components/RegisterSW.vue'
import ContinueReading from './components/ContinueReading.vue'
import Screenshot from './components/Screenshot.vue'

export default {
  extends: DefaultTheme,
  Layout() {
    return h(DefaultTheme.Layout, null, {
      'home-hero-actions-after': () => h(ContinueReading, { lab: 'encs4110', mode: 'card' }),
      'layout-bottom': () => [h(RegisterSW), h(ContinueReading, { lab: 'encs4110', mode: 'track' })],
    })
  },
  enhanceApp({ app }) {
    app.component('Screenshot', Screenshot)
  },
}
