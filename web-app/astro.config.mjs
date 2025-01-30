import { defineConfig } from 'astro/config';
import alpine from '@astrojs/alpinejs';
import tailwind from "@astrojs/tailwind";

//import node from "@astrojs/node";

import node from "@astrojs/node";

import preact from "@astrojs/preact";

// https://astro.build/config
export default defineConfig({
  integrations: [alpine(), tailwind(), preact()],
  vite: {
    build: {
      minify: false,
    },
  },
  output: "server",
  adapter: node({
    mode: "middleware"
  })
});