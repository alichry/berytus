const dummy = (...args: any) => { console.warn('berytus dummy called with:', ...args) };
const dummyAsync = async (...args: any) => { console.warn('berytus dummy called with:', ...args) };
// for PURE components to render.

// safari defines browser in the global namespace. so we have to check
// for "authRealm" too
if (typeof browser === "undefined" || browser.berytus === undefined) {
    global.browser = {
        ...global.browser, // global.browser is undefined, just for TS to compile.
        pageAction: {
            ...global.browser?.pageAction, // same as above
            setPopup: dummy
        },
        berytus: {
            unregisterRequestHandler: dummy,
            registerRequestHandler: dummy,
            resolveRequest: dummyAsync,
            rejectRequest: dummyAsync,
            openPageActionPopupIfNecessary: dummy
        }
    };

}

export {};