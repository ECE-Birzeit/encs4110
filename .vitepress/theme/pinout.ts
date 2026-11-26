// Whether the board-pinout sheet is open. Shared by its triggers: the floating button in
// PinoutPanel.vue and the "Pinout" button in JumpToProcedure.vue on every lab site with a pinout page.
import { ref } from 'vue'

export const pinoutOpen = ref(false)
