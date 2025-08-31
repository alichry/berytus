/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import type { FetchEntry } from "../signals/fetch-entries";

const stringifyBody = (headers: Record<string, string>, body: ArrayBuffer) => {
    const ct = headers['content-type'];
    if (/^application\/json;?/.test(ct)) {
        const decoder = new TextDecoder();
        return JSON.stringify(JSON.parse(decoder.decode(body)), null, 2);
    }
    if (/^text\//.test(ct) || /^application\/x-www-form-urlencoded/.test(ct)) {
        const decoder = new TextDecoder();
        return decoder.decode(body);
    }
    const bytes = new Uint8Array(body);
    return `data:${ct};base64,${
        // @ts-ignore: Firefox only.
        bytes.toBase64()
        }`;
}

type FetchMonitorCallback = (entry: FetchEntry) => void;

export abstract class FetchMonitor {
    #cb: FetchMonitorCallback;
    handler = (event: MessageEvent<any>) => {
        if (typeof event.data !== "object" || event.data === null) {
            return;
        }
        if (event.data.name !== 'fetch-result') {
            return;
        }
        const { data } = event;
        if (data.source === 'ServiceWorker') {
            data.request.body = stringifyBody(data.request.headers, data.request.body);
            data.response.body = stringifyBody(data.response.headers, data.response.body);
        }
        this.#cb(data);
    }

    constructor(cb: FetchMonitorCallback) {
        this.#cb = cb;
    }

    abstract start(): Promise<void>;
    abstract stop(): Promise<void>;
}

export class FetchMonitorViaServiceWorker
    extends FetchMonitor {
    #worker: ServiceWorker;
    #listener = (event: MessageEvent) => {
        if (event.source !== this.#worker) {
            return;
        }
        this.handler(event);
    }

    constructor(cb: FetchMonitorCallback) {
        super(cb);
        // Note, we want to register the service worker
        // before listening to events. we want to register
        // as soon as ViaExtensions queries for registered
        // service workers to include them in the filter.
        this.setup();
    }

    async start() {
        navigator.serviceWorker.addEventListener(
            'message',
            this.#listener
        );
        this.#worker = await this.setup();
    }

    async stop() {
        navigator.serviceWorker.removeEventListener(
            'message',
            this.#listener
        );
    }

    async setup() {
        const reg = await navigator.serviceWorker.register(
            "/fetch-monitor.sw.js",
            {
                scope: '/',
            }
        );
        if (reg.active) {
            return reg.active;
        }
        const worker = reg.waiting || reg.installing;
        return new Promise<ServiceWorker>((resolve) => {
            const listener = (e: Event) => {
                // #assert(e.target === worker);
                if (worker.state !== 'activated') {
                    return;
                }
                worker.removeEventListener('statechange', listener);
                resolve(worker);
            }
            worker.addEventListener("statechange", listener);
        });
    }
}

export interface TaskRunnerWithRetryArguments<T = unknown> {
    maxAttempts: number;
    handler: () => Promise<T>;
};

export class TaskRunnerWithRetry<T = unknown> {
    #args: TaskRunnerWithRetryArguments<T>;

    constructor(args: TaskRunnerWithRetryArguments<T>) {
        if (args.maxAttempts < 0) {
            throw new Error("maxAttempts must be >= 0");
        }
        this.#args = args;
    }

    async run(): Promise<T> {
        for (let i = 0; i < this.#args.maxAttempts - 1; i++) {
            try {
                const result = await this.#args.handler();
                return result;
            } catch (e) {}
        }
        return this.#args.handler();
    }
}

export class FetchMonitorViaWebExtensions
    extends FetchMonitor {
    #listener = (event: MessageEvent) => {
        if (event.source !== window) {
            return;
        }
        this.handler(event);
    }

    constructor(cb: FetchMonitorCallback) {
        super(cb);
    }

    async start(maxAttempts = 3): Promise<void> {
        const runner = new TaskRunnerWithRetry({
            maxAttempts,
            handler: () => this.#start()
        });
        await runner.run();
    }

    #start(): Promise<void> {
        const timeout = new Promise<void>((_, reject) => {
            setTimeout(() => {
                reject(new Error(
                    "Fetch Monitor Web Extension seems to be uninstalled "
                    + "or unresponsive; message timed out."
                ));
            }, 10 * 1000);
        })
        const out = new Promise<void>((resolve, reject) => {
            const listener = (event: MessageEvent) => {
                if (event.source !== window) {
                    return;
                }
                if (typeof event.data !== "object" || event.data === null) {
                    return;
                }
                if (event.data.name === "web-req-monitor:connected") {
                    window.removeEventListener("message", listener);
                    window.addEventListener("message", this.#listener);
                    resolve();
                }
            }
            try {
                window.addEventListener("message", listener);
                window.postMessage({ name: "web-req-monitor:connect" }, "*");
            } catch (e) {
                reject(e);
            }
        });
        return Promise.race([out, timeout]);
    }

    stop(): Promise<void> {
        const timeout = new Promise<void>((_, reject) => {
            setTimeout(() => {
                reject(new Error(
                    "Fetch Monitor Web Extension seems to be unresponsive; "
                    + "message timed out."
                ));
            }, 10 * 1000);
        });
        const out = new Promise<void>((resolve, reject) => {
            const listener = (event: MessageEvent) => {
                if (event.source !== window) {
                    return;
                }
                if (typeof event.data !== "object" || event.data === null) {
                    return;
                }
                if (event.data.name === "web-req-monitor:disconnected") {
                    window.removeEventListener("message", listener);
                    resolve();
                }
            }
            try {
                window.removeEventListener("message", this.#listener);
                window.addEventListener("message", listener);
                window.postMessage({ name: "web-req-monitor:disconnect" }, "*");
            } catch (e) {
                reject(e);
            }
        });
        return Promise.race([out, timeout]);
    }
}