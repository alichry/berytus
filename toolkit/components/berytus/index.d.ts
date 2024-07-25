/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
    }
    interface IXPCOMUtils {
        defineLazyGetter<O, K extends string, F extends () => any>(
            output: O,
            outputKey: K,
            Fn: F
        ): asserts output is O & { [_ in K]: ReturnType<F> }
    }
    interface IComponentsUtils {
        cloneInto(obj: object, cloneScope: object, options?: unknown): object;
    }
    interface ActorMessage { name: string; data: any }
    abstract class JSWindowActorParent {
        abstract receiveMessage(msg: ActorMessage): any;
    }
    interface ChildActor {
        sendQuery(
            name: string,
            data: any
        ): Promise<any>;
    }
}