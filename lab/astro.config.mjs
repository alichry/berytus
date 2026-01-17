/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { defineConfig } from "astro/config";
import node from "@astrojs/node";
import react from "@astrojs/react";
import tailwindcss from "@tailwindcss/vite";
import fs from "node:fs";
import svgr from "vite-plugin-svgr";
import simpleStackQuery from "simple-stack-query";

// https://astro.build/config
export default defineConfig({
  output: "server",
  adapter: node({
    mode: "standalone"
  }),
  publicDir: "./public",
  integrations: [react(), simpleStackQuery()],
  vite: {
    plugins: [tailwindcss(), svgr()],
    server: {
      https: {
        key: fs.readFileSync(process.env.SERVER_KEY_PATH),
        cert: fs.readFileSync(process.env.SERVER_CERT_PATH),
      },
    }
  }
});