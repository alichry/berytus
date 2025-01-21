import { Session, db } from "@root/db/db";
import { openWindow, redirectTab } from "./window-utils";

export const getSessionRecord = async (sessionId: string) => {
    const record = await db.sessions.get(sessionId);
    if (! record) {
        throw new Error('Unable to find session record by id ' + sessionId);
    }
    return record;
}

/**
 * For each Authentication/Registration session, we create one window.
 * For subsequent requests, we update the tab's URL of the existing window
 * to process the new requests. However, when a request is rejected, the window
 * is closed. As such, the initially created window/tab does not exist anymore.
 * For this, subsequent requests should call `openWindowOrRedirectTab` to either
 * redirect the existing tab (if present) or create a new one. This is only
 * applicable in the MODE_EXTERNAL_WINDOW mode. If a new window/tab is created,
 * we update the session record to reflect the new tab id.
 * @param sessionId
 * @param tabId
 * @param relativePath
 */
export const openWindowOrRedirectTab = async (
    sessionId: string,
    tabId: number,
    relativePath: string
) => {
    try {
        await browser.tabs.get(tabId);
    } catch (e) {
        // BRTTODO: Check if exception indicates a Tab Not Found error.
        // no tab. create a window.
        const window = await openWindow(relativePath);
        // update session record to store the new tab id.
        const newTabId = window.tabs![0].id!
        const change: Pick<Session, 'tabId'> = { tabId: newTabId };
        await db.sessions.update(sessionId, change);
        return newTabId;
    }
    await redirectTab(tabId, relativePath);
}