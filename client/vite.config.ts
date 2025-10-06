import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import tailwindcss from '@tailwindcss/vite'
import { BEATFOX_CONFIG } from '../config'

export default defineConfig({
  plugins: [react(), tailwindcss()],
  base: '/',
  server: {
    host: BEATFOX_CONFIG.domain,
    port: BEATFOX_CONFIG.ports.client,
    strictPort: false,
  },
})
