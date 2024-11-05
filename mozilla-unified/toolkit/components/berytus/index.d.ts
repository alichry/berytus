/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { ModuleMap } from './esm-import-map.index';

export { };


declare global {
    /* mozIDOMWindow */
    type IDOMWindow = any;
    interface BrowsingContext {
        top: BrowsingContext;
        isContent: boolean;
        currentWindowContext: WindowContext;
        documentPrincipal?: {
            isContentPrincipal: boolean;
            isAddonOrExpandedAddonPrincipal: boolean;
        };
        currentURI: {
            spec: string;
            scheme: string;
            host: string;
            port: number;
            filePath: string;
        };
        /**
         * content window, might be unavailable in the parent process or
         * in other cases that I am not aware of.
         */
        window: null | {
            windowGlobalChild: WindowGlobalChild;
        };
    }
    interface WindowContext {
        innerWindowId: number;
        outerWindowId: number;
    }
    interface WindowGlobalChild extends WindowContext {
        getActor(name: string): ChildActor;
    }
    var ChromeUtils: IChromeUtils;
    var Ci: any;
    var Cr: any; // Components.results
    var Cu: IComponentsUtils;
    var Cc: any;
    var Components: { Exception: new (str: string, nsResult: any) => any };
    var Services: any;
    var MozQueryInterface: Function;

    type IQueryInterface = (...args: any[]) => any;
    type nsIURI = any;
    interface ISupports {
        QueryInterface(...args: any[]): any;
    }
    type IVariant = any;
    interface IPropertyBag {
        getProperty(key: string): IVariant;
        enumerator: Iterable<{ name: string, value: any }>;
    }
    interface IWritablePropertyBag2 extends IPropertyBag {
        setPropertyAsBool(key: string, value: boolean): void;
        setPropertyAsAString(key: string, value: string): void;
        setPropertyAsInt64(key: string, value: number): void;
    }

    interface IChromeUtils {
        generateQI(list: Array<string>): IQueryInterface;
        defineESModuleGetters<O, M extends Record<string, keyof ModuleMap>>(
            output: O,
            imports: M
        ): asserts output is O & { [k in keyof M]: ModuleMap[M[k]] extends Record<k, infer P> ? P : never }
    }
    interface IComponentsUtils {
        cloneInto<O extends unknown>(obj: O, cloneScope: object, options?: unknown): O;
    }
    interface ActorMessage { name: string; data: any }
    abstract class JSWindowActorParent {
        abstract receiveMessage(msg: ActorMessage): any;
    }
    abstract class JSWindowActorChild {
        contentWindow: object;
        receiveMessage(msg: ActorMessage): any;
        sendAsyncMessage(aName: string, aData: unknown, transferables: Array<unknown>): void;
        sendQuery(msgName: string, data: unknown): Promise<unknown>;
    }
    interface ChildActor {
        sendQuery<T = any, R = any>(
            name: string,
            data: T
        ): Promise<R>;
    }
}