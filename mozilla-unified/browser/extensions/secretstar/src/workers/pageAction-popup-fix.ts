/**
 * browser.pageAction.openPopup() is a nuanced API.
 * - Cannot be called unless a user action status is set; fair.
 * - Would close a popup if it's open; making things difficult.
 *      - And, it's impossible to check whether the popup is already
 *        open without losing the user action status.
 * => Therefore, calling browser.pageAction.openPopup() whenever
 *    we want the user to resolve a request could potentially close
 *    an already open popup.
 *
 * In Berytus 0.1, I had implemented a WebExtension API for this purpose,
 * called openPageActionPopupIfNecessary (under browser.authrealm).
 * It had enabled extensions to open the popup, if necessary, only when
 * a request is pending and has not been resolved/reject yet. While this is
 * certainly not ideal to deploy for all extensions, it served as a remedy.
 *
 * In Berytus 0.2, I wrote the below code as a workaround without
 * relying on the introduction of a new WebExtensions API. Essentially,
 * the popup should send a message to the background script asking it
 * to resolve/reject the request. Here, the background script would
 * first hide the popup from the tab, ensuring that it is closed,
 * and it cannot be opened by the user, before resolving/rejecting the
 * request. Hence, whenever a new request comes from Berytus, the popup
 * is guaranteed to be closed, and browser.pageAction.openPopup() would
 * certainly open the popup. Sadly, this means that the popup page
 * should not directly call browser.berytus.(resolve/reject)Request.
 */

import { setPageActionUrlInTab } from "./pageAction-utils";

interface OpenOptions {
    requestId: string;
    tabId: number;
}

export const openPageActionPopupIfNecessary = async ({ requestId, tabId }: OpenOptions, absolutePath: string) => {
    browser.pageAction.show(tabId);
    const prom0 = associateTabIdWithRequest(requestId, tabId);
    const prom1 = setPageActionUrlInTab(tabId, absolutePath, false);
    const prom2 = (browser.pageAction as any).openPopup();
    await prom0;
    await prom1;
    await prom2;
}

const tabDictKey = (requestId: string) => `tabdict:${requestId}`;

const associateTabIdWithRequest = async (requestId: string, tabId: number) => {
    await browser.storage.session.set({
        [tabDictKey(requestId)]: tabId
    });
}

const getAssociatedTabId = async (requestId: string): Promise<number | undefined> => {
    const key = tabDictKey(requestId);
    const res = await browser.storage.session.get(key);
    return res[key];
}

const deleteTabIdAssociation = async (requestId: string): Promise<void> => {
    await browser.storage.session.remove(tabDictKey(requestId));
}

type MessageRequests = {
    type: "resolveRequest" | "rejectRequest"
    data: {
        requestId: string;
        value: unknown
    }
} /* | ... */

browser.runtime.onMessage.addListener(
    function(request: MessageRequests, sender, sendResponse) {
        console.log("onMessage:", request, sender)
        if (sender.id !== "secretstar@alichry") {
            return;
        }
        const hide = async () => {
            const tabId = await getAssociatedTabId(request.data.requestId);
            if (! tabId) {
                console.warn("secretstar: can't hide popup, tabId undefined.");
                return;
            }
            try {
                await Promise.resolve(browser.pageAction.hide(tabId));
            } catch (e) {
                console.warn("secretstar: error hiding popup:");
                console.error(e)
            } finally {
                await deleteTabIdAssociation(request.data.requestId);
            }
        }
        const send = async (type: "resolve" | "reject") => {
            try {
                await (
                    type === "resolve"
                        ? browser.berytus.resolveRequest(request.data.requestId, request.data.value)
                        : browser.berytus.rejectRequest(request.data.requestId, request.data.value)
                );
                console.debug("secretstar@alichry: settled request", request.data.requestId);
            } catch (e) {
                console.warn("secretstar: error resolving request:");
                console.error(e);
            }
        }
        if (request.type === "resolveRequest") {
            hide().then(() => send("resolve"));
            sendResponse();
            return;
        }
        if (request.type === "rejectRequest") {
            hide().then(() => send("reject"));
            sendResponse();
            return;
        }
    }
);