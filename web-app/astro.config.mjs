import { defineConfig } from 'astro/config';
import alpine from '@astrojs/alpinejs';
import tailwind from "@astrojs/tailwind";

import preact from "@astrojs/preact";

import netlify from "@astrojs/netlify";

// https://astro.build/config
export default defineConfig({
  integrations: [alpine(), tailwind(), preact()],
  vite: {
    build: {
      minify: false,
    },
  },
  output: "server",
  // TODO(berytus): Add build option to build for node
  adapter: netlify()
});