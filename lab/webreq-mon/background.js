/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

const URL_FILTER = [
    "http://localhost/*",
    "https://localhost/*",
    "http://127.0.0.1/*",
    "https://127.0.0.1/*"
];

const bitwiseNegate = (n) => {
    // the below right shift operator
    // returns an unsigned representation of
    // the underlying bits. Not using this
    // operator along with a NOT op results
    // in a signed representation of the bits.
    return ~n >>> 0;
}

/**
 * @param {ArrayBuffer[]} bufs
 */
const concatenateBufs = (bufs) => {
    bufs.forEach(buf => {
        if (buf instanceof ArrayBuffer) {
            return;
        }
        throw new Error(
            "Malformed input passed to concatenateBufs(). "
            + "One of the bufs is not an instanceof ArrayBuffer."
        );
    })
    let size = bufs.reduce((prev, curr) => prev + curr.byteLength, 0);

    let result = new Uint8Array(size);
    let offset = 0;
    for (const buf of bufs) {
        result.set(new Uint8Array(buf), offset);
        offset += buf.byteLength;
    }
    return result;
}

/**
 *
 * @param {Array<{ name: string; value: string; }>} headers
 * @returns {Record<string, string>}
 */
const transformHeaders = (headers) => {
    return headers.reduce((prev, curr) => {
        prev[curr.name.toLowerCase()] = curr.value;
        return prev;
    }, {});
}

/**
 *
 * @param {WebRequestDetailsReqBody} body
 * @returns {string}
 */
const toDataUrl = (body) => {
    if (body.error) {
        return `data:text/plain,WebExtensions Error: ${body.error}`;
    }
    if (body.formData) {
        return `data:application/json,${JSON.stringify(body.formData, null, 2)}`;
    }
    const bytes = concatenateBufs(body.raw.map(r => r.bytes || new ArrayBuffer(0)));
    return `data:application/octet-stream;base64,${
        // @ts-ignore: Firefox only.
        bytes.toBase64()
    }`;
}

/**
 * @typedef {typeof browser.storage.session} StorageArea
 */
class QueuedStorageContainer {
    /**
     * @type {Promise<unknown>}
     */
    #promise = Promise.resolve();
    #storage;

    /**
     *
     * @param {StorageArea} storage - storage.StorageArea
     *   pass a storage container, i.e. browser.storage.<type>.
     * @url https://developer.mozilla.org/en-US/docs/Mozilla/Add-ons/WebExtensions/API/storage/StorageArea
     */
    constructor(storage = browser.storage.session) {
        this.#storage = storage;
    }

    async ready() {
        await this.#promise;
    }

    async set(...args) {
        await this.ready();
        this.#promise = this.#storage.set(...args);
        return this.#promise;
    }

    async get(...args) {
        await this.ready();
        this.#promise = this.#storage.get(...args);
        return this.#promise;
    }

    async remove(...args) {
        await this.ready();
        this.#promise = this.#storage.remove(...args);
        return this.#promise;
    }

    async clear(...args) {
        await this.ready();
        this.#promise = this.#storage.clear(...args);
        return this.#promise;
    }
}


const STAGE_INITIAL = 1;
const STAGE_REQ_BODY = 2;
const STAGE_REQ_HEADERS = 4;
const STAGE_RES_BODY = 8;
const STAGE_RES_HEADERS = 16;
const STAGE_STARTED = 1 << 16;
const STAGE_REDIRECTED = 1 << 17;
const STAGE_COMPLETE = 1 << 18;
const STAGE_ABORTED = 1 << 19;
const stages = {
    STAGE_INITIAL,
    STAGE_REQ_BODY,
    STAGE_REQ_HEADERS,
    STAGE_RES_BODY,
    STAGE_RES_HEADERS,
    STAGE_STARTED,
    STAGE_REDIRECTED,
    STAGE_COMPLETE,
    STAGE_ABORTED
};

/**
 * 
 * @param {number} n
 */
function extractFlags(n) {
    const res = [];
    for (let stage in stages) {
        if (n & stages[stage]) {
            res.push(stage);
        }
    }
    return res;
}

class CleanupRequests {
    static alarmName = 'cleanup-alarm';
    static #maxLife = 10 * 60 * 1000;
    static get maxLife() { return CleanupRequests.#maxLife; }

    /**
     * @type {CleanupRequests}
     */
    static #instance;

    /**
     * @type {*}
     */
    #storage;

    /**
     * @type {Function | null}
     */
    #listener = null;

    /**
     * @hideconstructor
     * @param {*} storage
     */
    constructor(storage) {
        this.#storage = storage;
    }

    async run() {
        const entries = await new WebReqRepository(this.#storage).list(null);
        await Promise.all(Object.keys(entries).map(async key => {
            if (! key.startsWith('request:')) {
                return;
            }
            const entry = entries[key];
            if (entry.createdAt + CleanupRequests.maxLife > new Date().getTime()) {
                return;
            }
            if (entry.stage & STAGE_STARTED) {
                // the request should get fulfilled since
                // onResponseStarted was reached.
                return;
            }
            // 10 minutes have passed and the entry's stage
            // is not STARTED, or it is _final_. We delete it.
            await this.#storage.remove(key);
        }));
    }

    attach() {
        if (this.#listener) {
            return;
        }
        this.#listener = this.#onAlarm.bind(this);
        browser.alarms.onAlarm.addListener(this.#listener);
    }

    detach() {
        if (! this.#listener) {
            return;
        }
        browser.alarms.onAlarm.removeListener(this.#listener);
        this.#listener = null;
    }

    #onAlarm(alarm) {
        switch (alarm.name) {
            case CleanupRequests.alarmName:
                this.run();
                break;
        }
    }

    /**
     * @param {*} storage
     */
    static async setup(storage) {
        if (! CleanupRequests.#instance) {
            CleanupRequests.#instance = new CleanupRequests(storage);
        }
        const task = CleanupRequests.#instance;
        task.attach();

        const existingAlarm = await browser.alarms.get(CleanupRequests.alarmName);
        if (existingAlarm) {
            return task;
        }
        await browser.alarms.create(CleanupRequests.alarmName, {
            periodInMinutes: 5
        });
        return task;
    }
}

class WebReqListener {
    /**
     * @type {Map<WebReqListener, true> | undefined}
     */
    static #instances;

    /**
     * @type {boolean}
     */
    #active = false;

    /**
     * @type {WebReqRepository}
     */
    #repository;

    /**
     * @typedef {Object} RequestSubFilter
     * @prop {Array<string>} types
     * @prop {number} tabId
     * @prop {boolean} incognito
     * @prop {string?} serviceWorkerUrl
     *
     * @type {RequestSubFilter}
     */
    #filter;

    /**
     * @typedef {Object} WebReqRecordInfo
     * @prop {number} requestId
     * @prop {number} tabId
     * @prop {number} stage
     * @prop {string | null} error
     * @prop {string} url
     * @prop {Object} request
     * @prop {Object} response
     *
     * @typedef {(info: WebReqRecordInfo) => void} WebReqListenerInfoCallback
     *
     * @type {WebReqListenerInfoCallback}
     */
    #infoCallback;

    /**
     * @typedef {(error: unknown) => void} WebReqListenerErrorCallback
     */
    #errorCallback;

    /**
     *
     * @param {WebReqRepository} repo
     * @param {RequestSubFilter} filter
     * @param {WebReqListenerInfoCallback} infoCb
     * @param {WebReqListenerErrorCallback} errorCb
     */
    constructor(repo, filter, infoCb, errorCb = (err) => { console.error(err) }) {
        this.#repository = repo;
        this.#filter = filter;
        this.#infoCallback = infoCb;
        this.#errorCallback = errorCb;
    }

    get active() {
        return this.#active;
    }

    start() {
        WebReqListener.#instances.set(this, true);
        this.#active = true;
    }

    stop() {
        WebReqListener.#instances.delete(this);
        this.#active = false;
    }

    async #handlePotentialRedirectedRequests(requestId) {
        const entry = await this.#repository.get(requestId);
        if (! entry) {
            console.warn("RequestId not found:", requestId);
            return;
        }
        if (entry.stage & STAGE_REDIRECTED && entry.stage & STAGE_RES_BODY) {
            try {
                await this.#send(requestId);
            } finally {
                await this.#repository.delete(requestId);
            }
        }
    }

    /**
     * 
     * @param {(...args: any[]) => any} handler
     * @returns {Promise<unknown>}
     */
    #createSafeHandler(handler) {
        return (...args) => {
            Promise.resolve(handler(...args))
                .catch(err => this.#reportError(err));
        }
    }

    async #send(requestId) {
        if (! this.#active) {
            return;
        }
        const entry = await this.#repository.get(requestId);
        this.#infoCallback({
            ...entry,
            flags: extractFlags(entry.stage)
        });
    }

    #reportError(err) {
        if (! this.#active) {
            return;
        }
        this.#errorCallback(err);
    }

    #handlers = {
        onBeforeRequest: this.#createSafeHandler((details) => {
            return Promise.all([
                this.#repository.register(details),
                new Promise((resolve, reject) => {
                    const filter = browser.webRequest.filterResponseData(details.requestId);

                    /**
                     * @type {ArrayBuffer[]}
                     */
                    const bufs = [];
                    filter.onerror = (event) => {
                        // TODO(alichry): I wonder if onErrorOccurred would be
                        // triggered?
                        reject(filter.error);
                    }
                    filter.ondata = (event) => {
                        bufs.push(event.data);
                        filter.write(event.data);
                    };
                    filter.onstop = (event) => {
                        // The extension should always call filter.close() or filter.disconnect()
                        // after creating the StreamFilter, otherwise the response is kept alive forever.
                        // If processing of the response data is finished, use close. If any remaining
                        // response data should be processed by Firefox, use disconnect.
                        filter.disconnect();
                        this.#repository.setResponseBody(details.requestId, bufs)
                            .then(async () => {
                                // after the response body has been captured,
                                // we check whether the request has been
                                // marked as 'redirected' as otherwise,
                                // onCompleted/onError would not be called.
                                this.#handlePotentialRedirectedRequests(details.requestId);
                            })
                            .then(() => { resolve(); })
                            .catch((err) => { reject(err) });
                    };
                })
            ])
        }),
        onBeforeSendHeaders: this.#createSafeHandler(async (details) => {
            const { requestId, requestHeaders } = details;
            await this.#repository.setRequestHeaders(requestId, requestHeaders);
        }),
        onHeadersReceived: this.#createSafeHandler(async (details) => {
            if (details.responseHeaders) {
                await this.#repository.setResponseHeaders(details.requestId, details.responseHeaders);
            }
        }),
        // TODO(alichry): For redirected requests, onResponseStarted
        // is not invoked. We need to find an alternative way of capturing
        // headers.
        onResponseStarted: this.#createSafeHandler(async (details) => {
            await this.#repository.markAsStarted(details.requestId);
            //const { requestId, responseHeaders } = details;
            //await this.#repository.setResponseHeaders(requestId, responseHeaders);
        }),
        onBeforeRedirect: this.#createSafeHandler(async details => {
            if (details.responseHeaders) {
                await this.#repository.setResponseHeaders(requestId, details.responseHeaders);
            }
            await this.#repository.markAsRedirected(details.requestId);
            await this.#handlePotentialRedirectedRequests(details.requestId);
        }),
        onCompleted: this.#createSafeHandler(async (details) => {
            try {
                await this.#repository.markAsComplete(
                    details.requestId,
                    details.statusCode
                );
                await this.#send(details.requestId);
            } finally {
                await this.#repository.delete(details.requestId);
            }
        }),
        onErrorOccurred: this.#createSafeHandler(async (details) => {
            try {
                await this.#repository.markAsAborted(details.requestId, details.error);
                await this.#send(details.requestId);
            } finally {
                await this.#repository.delete(details.requestId);
            }
        }),
    }

    /**
     * @typedef {Object} RequestFilter
     * @prop {string[]?} urls
     *
     * @param {RequestFilter} filter
     * @returns
     */
    static setup(filter = {}) {
        if (WebReqListener.#instances) {
            return;
        }
        WebReqListener.#instances = new Map();
        const events = [
            'onBeforeRequest',
            'onBeforeSendHeaders',
            'onHeadersReceived',
            'onBeforeRedirect',
            'onResponseStarted',
            'onCompleted',
            'onErrorOccurred'
        ];
        const extraInfoSpec = {
            onBeforeRequest: ['requestBody', 'blocking'],
            onBeforeSendHeaders: ['requestHeaders'],
            onHeadersReceived: ['responseHeaders'],
            onResponseStarted: ['responseHeaders'],
            onBeforeRedirect: ['responseHeaders']
        };
        events.forEach(event => {
            const args = [
                (...args) => {
                    const [details] = args;
                    for (const inst of WebReqListener.#instances.keys()) {
                        if (! inst.active) {
                            continue;
                        }
                        if (! inst.#handlers[event]) {
                            continue;
                        }
                        if (
                            "tabId" in inst.#filter &&
                            details.tabId !== inst.#filter.tabId && !(
                                details.tabId === -1 &&
                                inst.#filter.serviceWorkerUrl === details.originUrl
                            )
                        ) {
                            continue;
                        }
                        if (
                            "incognito" in inst.#filter &&
                            details.incognito !== inst.#filter.incognito
                        ) {
                            continue;
                        }
                        if (
                            "types" in inst.#filter &&
                            inst.#filter.types.indexOf(details.type) === -1
                        ) {
                            continue;
                        }
                        // TODO(alichry): An improvement - add URL filter
                        // we are doing the filtering ourselves since
                        // browser extensions cannot setup persistent listeners
                        // in an asynchronous nature. The effective listeners
                        // are registered once the content script is initialized.
                        inst.#handlers[event](...args);
                    }
                },
                {
                    ...filter,
                    urls: filter.urls?.length > 0 ? filter.urls : ['<all_urls>']
                }
            ];
            if (extraInfoSpec[event]) {
                args.push(extraInfoSpec[event]);
            }
            browser.webRequest[event].addListener(...args);
        });
    }
}

class WebReqRepository {
    /**
     * @type {QueuedStorageContainer}
     */
    #storage;

    /**
     * @typedef {string | number | null} WebReqPartitionKey
     * @type {WebReqPartitionKey}
     */
    #partitionKey;

    /**
     *
     * @param {StorageArea} storage - storage.StorageArea
     *   pass a storage container, i.e. browser.storage.<type>.
     * @param {WebReqPartitionKey} partitionKey
     */
    constructor(storage, partitionKey = null) {
        this.#storage = new QueuedStorageContainer(storage);
        WebReqRepository.#validatePartitionKey(partitionKey);
        this.#partitionKey = partitionKey;
    }

    static #validatePartitionKey(partitionKey) {
        if (
            typeof partitionKey !== 'string' &&
            typeof partitionKey !== 'number' &&
            partitionKey !== null
        ) {
            throw new Error("Bad partition key");
        }
    }

    get storage() {
        return this.#storage;
    }

    /**
     * 
     * @param {number} requestId
     * @returns {string}
     */
    #key(requestId) {
        if (null === this.#partitionKey) {
            return `request:${requestId}`;
        }
        return `request:${this.#partitionKey}:${requestId}`;
    }

    /**
     *
     * @param {WebReqPartitionKey} partitionKey
     */
    async list(overridePartitionKey) {
        let partitionKey;
        if (undefined !== overridePartitionKey) {
            WebReqRepository.#validatePartitionKey(overridePartitionKey);
            partitionKey = overridePartitionKey;
        } else {
            partitionKey = this.#partitionKey;
        }
        const entries = await this.#storage.get();
        const prefix = partitionKey
            ? `request:${partitionKey}:`
            : `request:`;
        for (const key in entries) {
            if (key.startsWith(prefix)) {
                continue;
            }
            delete entries[key];
        }
        return entries;
    }

    async get(requestId) {
        if (! requestId) {
            throw new Error("Missing request id.");
        }
        const key = this.#key(requestId);
        const { [key]: entry } = await this.#storage.get(key);
        return entry;
    }

    async #set(requestId, entry) {
        const key = this.#key(requestId);
        await this.#storage.set({
            [key]: entry
        });
    }


    /**
     * @typedef {Object} WebRequestUploadData
     * @prop {ArrayBuffer | undefined} bytes
     * @prop {string | undefined} file - file path
     *
     * @typedef {Object} WebRequestDetailsReqBody
     * @prop {string | undefined} error
     * @prop {Record<string, string | string[]> | undefined} formData
     * @prop {WebRequestUploadData[]} raw
     *
     * @typedef {Object} WebRequestDetails
     * @prop {string} method
     * @prop {string} requestId
     * @prop {WebRequestDetailsReqBody | undefined} requestBody
     * @prop {number} tabId
     * @prop {string} type
     * @prop {string} url
     * @prop {string} originUrl
     *
     * @param {WebRequestDetails} details
     */
    async register(details) {
        const requestBody = details.requestBody ? toDataUrl(
            details.requestBody
        ) : "data:,";
        await this.#set(details.requestId, {
            requestId: details.requestId,
            tabId: details.tabId,
            stage: STAGE_INITIAL | STAGE_REQ_BODY,
            originUrl: details.originUrl,
            request: {
                url: details.url,
                method: details.method,
                headers: {},
                body: requestBody
            },
            createdAt: new Date().getTime()
        });
    }

    /**
     * 
     * @param {number} requestId
     */
    async delete(requestId) {
        //await this.#storage.remove(this.#key(requestId));
    }

    /**
     * 
     * @param {number} requestId
     * @param {Record<string, string>} requestHeaders
     */
    async setRequestHeaders(requestId, requestHeaders) {
        const entry = await this.get(requestId);
        await this.#set(requestId, {
            ...entry,
            stage: entry.stage | STAGE_REQ_HEADERS,
            request: {
                ...entry.request,
                headers: transformHeaders(requestHeaders)
            }
        });
    }

    /**
     *
     * @param {number} requestId
     * @param {ArrayBuffer[]} bufs
     */
    async setResponseBody(requestId, bufs) {
        const entry = await this.get(requestId);
        const bytes = concatenateBufs(bufs);
        const body =  `data:application/octet-stream;base64,${
            // @ts-ignore: Firefox only.
            bytes.toBase64()
        }`;
        await this.#set(requestId, {
            ...entry,
            stage: entry.stage | STAGE_RES_BODY,
            response: {
                ...entry.response,
                body
            }
        });
    }

    /**
     * 
     * @param {number} requestId
     * @param {Record<string, string>} responseHeaders
     */
    async setResponseHeaders(requestId, responseHeaders) {
        const entry = await this.get(requestId);
        await this.#set(requestId, {
            ...entry,
            stage: entry.stage | STAGE_RES_HEADERS,
            response: {
                ...entry.response,
                headers: transformHeaders(responseHeaders)
            }
        });
    }

    /**
     *
     * @param {number} requestId
     */
    async markAsStarted(requestId) {
        const entry = await this.get(requestId);
        await this.#set(requestId, {
            ...entry,
            stage: entry.stage | STAGE_STARTED,
        });
    }

    /**
     *
     * @param {number} requestId
     * @param {number} statusCode
     */
    async markAsComplete(requestId, statusCode) {
        const entry = await this.get(requestId);
        await this.#set(requestId, {
            ...entry,
            stage: (entry.stage | STAGE_COMPLETE) & bitwiseNegate(STAGE_STARTED),
            response: {
                ...entry.response,
                status: statusCode
            }
        });
    }

    /**
     * 
     * @param {number} requestId
     * @param {string} error
     */
    async markAsAborted(requestId, error) {
        const entry = await this.get(requestId);
        await this.#set(requestId, {
            ...entry,
            stage: (entry.stage | STAGE_ABORTED) & bitwiseNegate(STAGE_STARTED),
            error: error
        });
    }

    /**
     * 
     * @param {number} requestId
     */
    async markAsRedirected(requestId) {
        const entry = await this.get(requestId);
        if (! entry) {
            console.warn("Unable to find request by id before marking it as redirected:" + requestId);
            return;
        }
        await this.#set(requestId, {
            ...entry,
            stage: (entry.stage | STAGE_REDIRECTED) & bitwiseNegate(STAGE_STARTED)
        });
    }
}

const storage = browser.storage.local;
CleanupRequests.setup(storage);
WebReqListener.setup();

const run = () => {
    browser.runtime.onConnect.addListener(port => {
        const { tab } = port.sender;
        if (! tab || ! tab.id) {
            port.disconnect();
            return;
        }
        /**
         * @type {WebReqListener | null}
         */
        let listener = null;
        const repo = new WebReqRepository(storage, tab.id);
        port.onMessage.addListener(event => {
            if (typeof event !== "object" || event === null || event.name !== "setup") {
                port.disconnect();
                throw new Error("bad client");
            }
            if (event.serviceWorkerUrl) {
                serviceWorkerUrl = event.serviceWorkerUrl;
            }
            listener = new WebReqListener(
                repo,
                {
                    tabId: tab.id,
                    windowId: tab.windowId,
                    serviceWorkerUrl: event.serviceWorkerUrl
                },
                async (entry) => {
                    await port.postMessage({
                        name: "fetch-result",
                        source: "WebExtensions",
                        ...entry
                    });
                }
            );
            listener.start();
        }, { once: true });

        port.onDisconnect.addListener(ev => {
            listener?.stop();
        });
    });
}
run();