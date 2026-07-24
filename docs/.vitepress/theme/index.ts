// .vitepress/theme/index.ts
import { h } from 'vue'
import DefaultTheme from 'vitepress/theme'
import 'virtual:group-icons.css'
import './custom.css'

import RegisterSW from './components/RegisterSW.vue'
import VPHero from './components/VPHero.vue'


export default {
  extends: DefaultTheme,
  Layout() {
    return h(DefaultTheme.Layout, null, {
      'layout-bottom': () => h(RegisterSW),
    })
  },
  enhanceApp({ app }) {
    app.component('VPHero', VPHero)
  },
}
