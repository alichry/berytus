import { PAGECONTEXT_WINDOW } from "@root/pagecontext";
import { paths, url } from "./paths";

const windowSize = { width: 600, height: 700 } as const;

export async function openWindow(relativePath: string, size: typeof windowSize = windowSize) {
    return browser.windows.create({
        type: "panel",
        url: url(relativePath, PAGECONTEXT_WINDOW),
        width: windowSize.width,
        height: windowSize.height
    });
}

export async function openLoadingWindow() {
    return openWindow(paths.loading);
}

export async function getFirstTabInWindow(windowId: number | chrome.windows.Window) {
    //const tabs = await browser.tabs.query({ windowId: windowId });
    //return tabs![0];
    const tabs = typeof windowId === "number" ?
        (await browser.windows.get(windowId)).tabs :
        windowId.tabs;
    if (! tabs) {
        throw new Error('Passed windowId does not have any tabs!');
    }
    const firstTab = tabs.filter(t => t.index === 0)[0];
    if (! firstTab) {
        throw new Error('Passed windowId did not have a tab at index 0!');
    }
    return firstTab;
}

export async function redirectTab(tabId: number, relativePath: string) {
    return browser.tabs.update(tabId, {
        url: url(relativePath, PAGECONTEXT_WINDOW)
    });
}

export async function redirectFirstTabInWindow(window: number | chrome.windows.Window, relativePath: string) {
    const firstTab = await getFirstTabInWindow(window);
    if (! firstTab.id) {
        throw new Error('Retrieved tab did not have an id!');
    }
    return redirectTab(firstTab.id, relativePath);
}

export async function redirectTabtoLoadingPage(tabId: number) {
    return redirectTab(tabId, paths.loading);
}