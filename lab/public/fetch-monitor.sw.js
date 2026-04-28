/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

self.addEventListener("install", (event) => {
  self.skipWaiting();
});
self.addEventListener("activate", (event) => {
  event.waitUntil(clients.claim());
});


/**
 * @param {*} clientId
 */
const getClient = async (clientId) => {
    const client = await self.clients.get(clientId);
    // From MDN: Exit early if we don't get the client.
    // Eg, if it closed.
    if (!client) {
        console.warn("(fetch-monitor.sw.js): TODO(berytus): client unavailable");
        return;
    }
    return client;
}

/**
 * @typedef {Object} FetchEvent
 * @property {Request} request
 * @property {string | undefined} clientId
 * @property {Promise} waitUntil
 * @property {Promise<Response> | Response} respondWith
 *
 * @param {FetchEvent} event
 */
const fetchHandler = (event) => {
    // From MDN: Exit early if we don't have access to the client.
    // Eg, if it's cross-origin.
    //
    if (! event.clientId) {
        console.warn("(fetch-monitor.sw.js): TODO(berytus): client id unavailable");
        return;
    }
    event.respondWith(
        (async () => {
            const clonedReq = event.request.clone();
            const resp = await fetch(event.request);
            try {
                const client = await getClient(event.clientId);
                if (! client) {
                    return resp;
                }
                const requestBody = await clonedReq.arrayBuffer();
                const clonedResp = resp.clone();
                const responseBody = await clonedResp.arrayBuffer();
                client.postMessage({
                    name: "fetch-result",
                    source: "ServiceWorker",
                    error: null,
                    url: clonedReq.url,
                    originUrl: client.url,
                    originType: client.type,
                    request: {
                        url: clonedReq.url,
                        method: clonedReq.method,
                        mode: clonedReq.mode,
                        destination: clonedReq.destination,
                        integrity: clonedReq.integrity,
                        credentials: clonedReq.credentials,
                        referrer: clonedReq.referrer,
                        referrerPolicy: clonedReq.referrerPolicy,
                        redirect: clonedReq.redirect,
                        keepalive: clonedReq.keepalive,
                        headers: clonedReq.headers.entries().reduce((prev, [key, val]) => {
                            prev[key] = val;
                            return prev;
                        }, {}),
                        body: requestBody
                    },
                    response: {
                        type: clonedResp.type,
                        status: clonedResp.status,
                        statusText: clonedResp.statusText,
                        url: clonedResp.url,
                        ok: clonedResp.ok,
                        redirected: clonedResp.redirected,
                        headers: clonedResp.headers.entries().reduce((prev, [key, val]) => {
                            prev[key] = val;
                            return prev;
                        }, {}),
                        body: responseBody
                    }
                });
            } catch (e) {
                console.error('(fetch-monitor.sw.js): Error processing fetch event:', e);
            } finally {
                return resp;
            }
        })()
    );
}

addEventListener('fetch', fetchHandler);