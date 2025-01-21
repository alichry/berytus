export const PAGECONTEXT_WINDOW = "window";
export const PAGECONTEXT_POPUP = "popup";

export const allowedPageContexts = [
    PAGECONTEXT_WINDOW,
    PAGECONTEXT_POPUP
] as const;

export type AppPageContext = typeof PAGECONTEXT_WINDOW | typeof PAGECONTEXT_POPUP;

export const isPopup = (pageContext: string) => {
    return pageContext === PAGECONTEXT_POPUP;
}