import { defineConfig } from 'astro/config';
import alpine from '@astrojs/alpinejs';
import tailwind from "@astrojs/tailwind";

import preact from "@astrojs/preact";
import node from "@astrojs/node";
import netlify from "@astrojs/netlify";

import { strict as assert } from "node:assert";

const buildTarget = process.env.BUILD_TARGET || "node";

assert(buildTarget === "node" || buildTarget === "netlify");

// https://astro.build/config
export default defineConfig({
  integrations: [alpine(), tailwind(), preact()],
  vite: {
    build: {
      minify: false,
    },
  },
  output: "server",

  adapter: buildTarget === "node" ? node({
    mode: "middleware"
  }) : netlify()
});