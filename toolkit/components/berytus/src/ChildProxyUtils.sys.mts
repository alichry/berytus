/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

export const getChildActor = (
    browsingContext: BrowsingContext,
    actorName: string
): ChildActor => {
    const IS_CONTENT =
        Services.appinfo.processType == Services.appinfo.PROCESS_TYPE_CONTENT;
    if (!IS_CONTENT) {
        throw new Error(
            'Unable to get child acttor; caller attempted to retrieve '
            + 'child actor from a non-content process'
        );
    }
    const window = browsingContext.window;
    if (! window) {
        throw new Error(
            'Unable to get child actor; window is not set.'
        );
    }
    return window.windowGlobalChild.getActor(actorName);
}

/* CommonProxy: */

function createFunctionProxy(target: unknown, receiver: unknown, fn: Function) {
    if (fn instanceof MozQueryInterface) {
        // QueryInterface does not have an `apply` method, return as is.
        return fn;
        // TODO(berytus): Consider removing/amending this as we do
        // not want to proxy a QueryInterface anyway.
    }
    return function(this: unknown, ...args: any[]) {
        if (this !== receiver) {
            throw new Error('Are you trying to bind a proxied function to a different `this` (other than the proxy object)? Not allowed.');
        }
        return fn.apply(target, args);
    }
}

type Fill<T extends object> = (target: T, prop: string, receiver: any) => any;

/**
 * Proxy that redirects everything to the target if exists (including functions)
 * and calls fill(...) when the prop was not found.
 */
function CommonProxy<T extends object>(target: T, fill: Fill<T>) {
    return new Proxy(target, {
        get(target, prop, receiver) {
            if (typeof prop === "symbol") {
                throw new Error('Proxying symbols is not supported');
            }
            if (prop in target) {
                const value = target[prop as keyof typeof target];
                if (value instanceof Function) {
                    return createFunctionProxy(target, receiver, value);
                }
                return value;
            }
            return fill(target, prop, receiver);
        },
    });
}

/* PassthroughFallbackProxy: */

export interface IBaseChildProxy {
    browsingContext: BrowsingContext;
}

export interface ChildProxyConstructor<Func extends (...args: any[]) => any> {
    new(...args: Parameters<Func>): ReturnType<Func>;
}

export function PassthroughFallbackProxy<P extends object, T extends IBaseChildProxy>(
    target: T,
    methods: Array<string & keyof P>,
    actorName: string
): T & Omit<P, keyof T> {
    const proxy = CommonProxy(
        target,
        (target, prop, receiver) => {
            if (methods.indexOf(prop as keyof P & string) !== -1) {
                throw new Error(
                    'PasshthroughFallbackProxy Error: ' +
                    'Accessed property is not a method ' +
                    '(only methods are proxied)'
                );
            }
            const routine = prop;
            return async function proxyCall(...args: any[]) {
                const actor = getChildActor(target.browsingContext, actorName);
                const result = await actor.sendQuery(
                    `${actorName}:${routine}`,
                    args
                );
                return result;
            }
        }
    );
    return proxy as typeof proxy & Omit<P, keyof typeof proxy>;
}