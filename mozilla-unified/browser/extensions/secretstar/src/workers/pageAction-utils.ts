import { PAGECONTEXT_POPUP } from '@root/pagecontext';
import { url, paths } from './paths';

export function setPageActionUrlInTab(tabIdWherePageActionIs: number, relativePath: string) {
    return browser.pageAction.setPopup({
        tabId: tabIdWherePageActionIs,
        popup: url(relativePath, PAGECONTEXT_POPUP)
    });
}

export function setPageActionToLoadingPage(tabIdWherePageActionIs: number) {
    return setPageActionUrlInTab(tabIdWherePageActionIs, paths.loading);
}

/**
 * browser.pageAction.openPopup() has several limitations:
 * 1. It only opens up the page action popup in the selected tab.
 * 2. If the popup is already open, it closes it. Making it difficult
 * to open the popup if necessary.
 * 3. If we wish to highlight the relevant tab where the popup should open,
 * we lose the `user action` status since doing so is asynchronous, and
 * we cannot open the popup unless the correct tab is highlighted.
 * We have created browser.authRealm.openPageActionPopupIfNecessary
 * to open the popup in the relevant tab (the tab belonging to the authRealm
 * session).
 * @param tabIdWherePageActionIs
 */
export async function openPageActionInTabId(tabIdWherePageActionIs: number) {
    const tab = await browser.tabs.get(tabIdWherePageActionIs);
    await browser.tabs.highlight({ windowId: tab.windowId, tabs: [tab.index] });
    // Firefox only.
    (browser.pageAction as any).openPopup();
}