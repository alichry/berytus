#!/usr/bin/env node
/*
 * At the moment, this express server is only ran using
 * npm run start. `npm run astro dev` does not start this server.
 * Environment Variables:
 * PORT=8080
 * HOST=0.0.0.0
 */
import express from 'express';
import { handler as ssrHandler } from './dist/server/entry.mjs';
import morgan from 'morgan';

const app = express();
app.set("trust proxy", true);
app.set('x-powered-by', false);
app.use(morgan('[:date[clf]] :remote-addr :req[x-forwarded-for] :method HTTP/:http-version :status ":url" ":referrer" ":user-agent"'));

// Change this based on your astro.config.mjs, `base` option.
// They should match. The default value is "/".
const base = '/';
app.use(base, express.static('dist/client/'));
app.use(ssrHandler);

const ip = process.env.HOST || "0.0.0.0";
const port = process.env.PORT || 8080

app.listen(port, ip);
console.log(`Berytus Web Server listening on ${ip}:${port}`);