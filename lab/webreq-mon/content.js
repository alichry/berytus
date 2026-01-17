/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

const getServiceWorkerUrl = () => {
    return new Promise((resolve) => {
        const script = document.createElement("script");
        const id = "Ky7rf1zC58ZAelUW0JeHRHCrcAx8OqoksHlmwhSe77ff";
        script.id = id;
        let observer = new MutationObserver((mutations) => {
            mutations.forEach((mutation) => {
                if (
                    mutation.type === "attributes" &&
                    mutation.attributeName === "data-swurl"
                ) {
                    const scriptUrl = script.dataset.swurl;
                    script.remove();
                    observer.disconnect();
                    observer = null;
                    return resolve(scriptUrl);
                }
            });
        });
        observer.observe(script, { attributes: true });
        script.textContent = `
          document.getElementById('${id}').dataset.swurl =
                navigator.serviceWorker?.controller?.scriptURL;`;
        document.head.appendChild(script);
    });
};


class WebReqMonitor {
    #port;

    #listener = (message) => {
        this.#forward(message);
    }

    #forward(message) {
        window.postMessage(message);
    }

    async connect() {
        this.#port = browser.runtime.connect();
        this.#port.postMessage({
            name: "setup",
            serviceWorkerUrl: await getServiceWorkerUrl()
        })
        this.#port.onMessage.addListener(this.#listener)
    }

    disconnect() {
        browser.runtime.onMessage.removeListener(this.#listener);
        if (this.#port) {
            this.#port.disconnect();
        }
    }
}

const run = () => {
    console.debug("webreq-monitor(content): setup()");
    const mon = new WebReqMonitor();
    const disconnect = () => {
        mon.disconnect();
        window.postMessage({ name: "web-req-monitor:disconnected" }, "*");
    }
    window.addEventListener("message", async (event) => {
        if (event.source !== window) {
            return;
        }
        if (typeof event.data !== "object" || event.data === null) {
            return;
        }
        if (event.data.name === 'web-req-monitor:connect') {
            console.debug("webreq-monitor(content): connecting to background");
            await mon.connect();
            window.postMessage({ name: "web-req-monitor:connected" }, "*");
        }
        if (event.data.name === 'web-req-monitor:disconnect') {
            console.debug("webreq-monitor(content): Disconnecting from background");
            disconnect();
        }
    });
    window.addEventListener("beforeunload", () => {
        disconnect();
    });
}

if (window.document.readyState === "loading") {
  window.document.addEventListener("DOMContentLoaded", run);
} else {
  run();
}