#!/usr/bin/env node
/*
 * At the moment, this express server is only ran using
 * npm run start. `npm run astro dev` does not start this server.
 * Environment Variables:
 * LISTEN_PORT (required)
 * LISTEN_HOST=0.0.0.0
 * SERVER_KEY_PATH (optional)
 * SERVER_CERT_PATH (optional)
 */
import express from 'express';
import { handler as ssrHandler } from './dist/server/entry.mjs';
import morgan from 'morgan';
import * as https from 'https';
import { strict as assert } from "node:assert";
import * as fs from 'node:fs';

const app = express();
app.set("trust proxy", true);
app.set('x-powered-by', false);
app.use(morgan('[:date[clf]] :remote-addr :req[x-forwarded-for] :method HTTP/:http-version :status ":url" ":referrer" ":user-agent"'));

// Change this based on your astro.config.mjs, `base` option.
// They should match. The default value is "/".
const base = '/';
app.use(base, express.static('dist/client/'));
app.use(ssrHandler);

const ip = process.env.LISTEN_HOST || "0.0.0.0";
const port = process.env.LISTEN_PORT;
assert(port, "LISTEN_PORT environment variable is required");

if (process.env.SERVER_KEY_PATH) {
    const server = https.createServer({
        cert: fs.readFileSync(process.env.SERVER_CERT_PATH, 'utf8'),
        key: fs.readFileSync(process.env.SERVER_KEY_PATH, 'utf8')
    }, app);
    server.listen(port, ip);
} else {
    app.listen(port, ip);
}

console.log(`Berytus Web Server listening on ${ip}:${port}`);