import { useLiveQuery } from "dexie-react-hooks";
import { Account, db, Field, Identity, Picture } from "@root/db";
import { useCallback, useEffect, useState } from "react";
import { NavigateFunction, NavigateOptions, To, useNavigate, useParams } from "react-router-dom";
import { MODE, MODE_PAGE_ACTION } from './env';
import { url } from "./workers/paths";
import { PAGECONTEXT_POPUP } from "./pagecontext";
import { Schema } from "yup";
import type { WebAppActor, Request, RequestType, RequestHandlerFunctionReturnType } from "@berytus/types";
import { ERejectionCode } from "@berytus/enums";
import { Channel } from "./db/Channel";
import JWEPacketCipherBox from "./crypto/JWEPacketBox";

interface MaybeResult {
    sent: boolean;
    promise: Promise<void>;
}

type MaybeResolve<ER extends RequestType> =
    RequestHandlerFunctionReturnType<ER> extends undefined | void
    ? (value?: undefined) => MaybeResult
    : (value: RequestHandlerFunctionReturnType<ER>) => MaybeResult;

type MaybeReject = (reason: ERejectionCode) => MaybeResult;

const resolveIf = (hasBeenProcessed: boolean, requestId: string, value: unknown | Promise<unknown>): MaybeResult => {
    if (hasBeenProcessed) {
        return {
            sent: false,
            promise: Promise.resolve()
        };
    }
    const prom = Promise.resolve(value)
        .then(value => browser.runtime.sendMessage({
            type: "resolveRequest",
            data: {
                requestId,
                value
            }
        }))
        .catch(e => {
            console.error("An error has occurre during browser.runtime.sendMessage('resolveRequest'):", e.message);
            console.error(e);
            throw e;
        });
    return {
        sent: true,
        promise: prom
    };
}
const rejectIf = (hasBeenProcessed: boolean, requestId: string, value: unknown): MaybeResult => {
    if (hasBeenProcessed) {
        return {
            sent: false,
            promise: Promise.resolve()
        };
    }
    if (value instanceof Promise) {
        return {
            sent: false,
            promise: Promise.reject(new Error("rejectIf() does not accept Promise-wrapped values."))
        }
    }
    const prom = browser.runtime.sendMessage({
        type: "rejectRequest",
        data: {
            requestId,
            value
        }
    }).catch(e => {
        console.error("An error has occurre during browser.runtime.sendMessage('rejectRequest'):", e.message);
        console.error(e);
        throw e;
    });
    return {
        sent: true,
        promise: prom
    };
}

const createMaybeFunction = (
    resolveOrReject: "resolve" | "reject",
    hasBeenProcessed: boolean,
    setProcessed: (newValue: true) => void,
    setIgnored: (newValue: true) => void,
    setProcessing: (newValue: boolean) => void,
    setError: (e: unknown) => void,
    requestId: string
) => {
    const target = resolveOrReject === "resolve" ? resolveIf : rejectIf;
    return (value: unknown): MaybeResult => {
        setProcessing(true);
        const res = target(hasBeenProcessed, requestId, value);
        if (! res.sent) {
            setIgnored(true);
            setProcessing(false);
            return res;
        }
        res.promise
            .then(() => {
                setProcessed(true);
            })
            .catch(e => {
                setError(e);
            })
            .finally(() => {
                setProcessing(false);
            })
        return res;
    }
}

interface UseRequestHook<ER extends RequestType> {
    processing: boolean;
    processed: boolean;
    maybeResolve?: MaybeResolve<ER>;
    maybeReject?: MaybeReject;
    error?: unknown;
}

interface UseRequestOptions {
    onProcessed?(): void;
    onIgnored?(): void;
    cipherbox?: JWEPacketCipherBox;
}

/**
 * Note(berytus):
 * @note Make sure callbacks are memoised using useCallback;
 * otherwise, they will get invoked on each render if the
 * request has been processed or ignored.
 */
export function useRequest<ER extends RequestType>(
    req: Request | undefined,
    {
        onProcessed,
        onIgnored,
        cipherbox
    }: UseRequestOptions = {}
): UseRequestHook<ER> {
    const [processing, setProcessing] = useState<boolean>(false);
    const [processed, setProcessed] = useState<boolean>(false);
    const [ignored, setIgnored] = useState<boolean>(false);
    const [maybeResolve, setMaybeResolve] = useState<MaybeResolve<ER>>();
    const [maybeReject, setMaybeReject] = useState<MaybeReject>();
    const [error, setError] = useState<unknown>();

    useEffect(() => {
        if (! req) {
            setMaybeResolve(undefined);
            setMaybeReject(undefined);
            return;
        }
        setMaybeResolve(
            () => {
                const maybe = createMaybeFunction("resolve", processed, setProcessed, setIgnored, setProcessing, setError, req.id);
                return (value: unknown) => {
                    if (cipherbox) {
                        return maybe(cipherbox.encrypt(value));
                    }
                    return maybe(value);
                }
            }
        );
        setMaybeReject(
            () => createMaybeFunction("reject", processed, setProcessed, setIgnored, setProcessing, setError, req.id)
        );
    }, [processed, req, cipherbox]);
    useEffect(() => {
        if (ignored) {
            if (! onIgnored) {
                return;
            }
            onIgnored();
            return;
        }
        if (processed) {
            if (! onProcessed) {
                return;
            }
            onProcessed();
            return;
        }
    }, [processed, ignored, onIgnored, onProcessed]);
    return {
        processing,
        processed,
        maybeResolve,
        maybeReject,
        error
    }
}

export function useAbortRequestOnWindowClose({ maybeReject, tabId }: { maybeReject?: (reason: ERejectionCode) => void, tabId: number | undefined }) {
    useEffect(() => {
        if (! maybeReject) {
            return;
        }
        const cb = () => {
            if (tabId && MODE === MODE_PAGE_ACTION) {
                browser.pageAction.setPopup({
                    tabId: tabId,
                    popup: url('/', PAGECONTEXT_POPUP)
                });
            }
            console.warn("Rejecting request since user cancelled/closed the window");
            maybeReject(ERejectionCode.OperationAborted);
        }
        window.addEventListener('beforeunload', cb);
        return () => window.removeEventListener('beforeunload', cb);
    }, [maybeReject])
}

const constructDoNavigate = (tabId: number | undefined, navigate: NavigateFunction): (relativePath: string) => void => {
    return (relativePath: string) => {
        if (tabId && MODE === MODE_PAGE_ACTION) {
            browser.pageAction.setPopup({
                tabId: tabId,
                popup: url(relativePath, PAGECONTEXT_POPUP)
            });
        }
        return navigate(relativePath);
    }
}

const constructDoNavigateWithPageContextRoute = (pageContext: string, navigate: NavigateFunction): NavigateFunction => {
    return (relativePath: To | number, options?: NavigateOptions) => {
        if (typeof relativePath === "number") {
            throw new Error("delta navigation is not supported");
        }
        if (typeof relativePath === "object") {
            throw new Error('Partial path URL navigation is not supported');
        }
        const to = (pageContext ? '/' + pageContext : '') +
            (relativePath.startsWith('/') ? '' : '/')
            + relativePath;
        return navigate(to, options);
    }
}

export function useIdentity(): Identity | undefined {
    const identity = useLiveQuery(async () => {
        const res = await db.getIdentity();
        return res;
    });
    return identity;
}

export function useSettings() {
    const settings = useLiveQuery(() => {
        return db.getSettings();
    });
    return settings;
}

export function useAccounts(
    givenActor?: WebAppActor,
    category?: string,
    version?: number,
): undefined | Array<Account> {
    const accounts = useLiveQuery(async () => {
        if (! givenActor) {
            return [];
        }
        return db.findCorrespondingAccounts(givenActor, category, version)
            .toArray();
    }, [givenActor, category, version]);
    return accounts;
}

export function usePicture(pictureId: string | undefined) {
    const [picture, setPicture] = useState<Picture | undefined>();
    useEffect(() => {
        (async () => {
            if (pictureId === undefined) {
                setPicture(undefined);
                return;
            }
            const pic = await db.picture.get(pictureId);
            setPicture(pic);
        })();
    }, [pictureId]);
    return picture;
}

export function useNavigateWithPageContextRoute() {
    const { pageContext } = useParams();
    const navigate = useNavigate();
    const [doNavigate, setDoNavigate] = useState(() => constructDoNavigateWithPageContextRoute(
        pageContext || "", navigate
    ));
    useEffect(() => {
        //let pageContext: string = getPageContextFromLocation(location) || "";
        if (! pageContext) {
            console.warn("No page context was found!");
        }
        setDoNavigate(() => constructDoNavigateWithPageContextRoute(pageContext || '', navigate));
    }, [pageContext]);
    return doNavigate;
}

export function useNavigateWithPopupContextAndPageContextRoute(tabId: number | undefined) {
    const navigate = useNavigateWithPageContextRoute();
    const [doNavigate, setDoNavigate] = useState(() => constructDoNavigate(tabId, navigate));
    useEffect(() => {
        setDoNavigate(() => constructDoNavigate(tabId, navigate));
    }, [tabId]);
    return doNavigate;
}

export function useAccount(accountId: string | undefined) {
    const [error, setError] = useState<Error>();
    const account = useLiveQuery(async () => {
        if (! accountId) {
            return;
        }
        const record = await db.accounts.get(accountId);
        if (! record) {
            setError(new Error('The selected account id is invalid'));
            return;
        }
        return record;
    }, [accountId]);
    return { account, error };
}

export type UseAccountFieldsHookResult<T extends Array<string> | string | undefined> =
    T extends string ? {
        fields?: Field;
        error?: Error;
    } : T extends Array<string> ? {
        fields?: Array<Field>;
        error?: Error;
    } : {
        fields?: undefined;
        error?: Error;
    }

export function useAccountFields<T extends Array<string> | string | undefined>(
    account: Account | undefined,
    fieldIdOrIdList: T
): UseAccountFieldsHookResult<T> {
    const [fields, setFields] = useState<Array<Field>>();
    const [error, setError] = useState<Error>();
    useEffect(() => {
        if (! account || ! fieldIdOrIdList) {
            if (fields !== undefined) {
                setFields(undefined);
            }
            return;
        }
        const fieldIds: string | Array<string> = Array.isArray(fieldIdOrIdList) ?
            fieldIdOrIdList :
            [fieldIdOrIdList];
        const f = Object.values(account.fields)
            .filter(f => fieldIds.indexOf(f.id) !== -1);
        if (fieldIds.length !== f.length) {
            setError(
                new Error(
                    'One of the requested fields is missing. ' +
                    'TODO: Convey the field id that is missing'
                )
            );
            return;
        }
        setFields(f);
    }, [account, fieldIdOrIdList]);
    if (typeof fieldIdOrIdList === "string") {
        return {
            fields: fields ? fields[0] : undefined,
            error
        } as UseAccountFieldsHookResult<T>;
    }
    return { fields, error } as UseAccountFieldsHookResult<T>;
}

export function useValueOverrides<K extends string = string, V extends string = string>() {
    const [overrides, setOverrides] = useState<Record<K, V>>({} as Record<K, V>);
    const [override, setOverride] = useState<(key: K, value: V) => void>(
        () => {
            return (key: K, value: V) => {
                setOverrides({
                    ...overrides,
                    [key]: value
                });
            }
        }
    );
    const [getDesiredValues, setGetDesiredValues] = useState<(defaultValues: Record<K, V>) => Record<K, V>>(
        () => {
            return (defaultValues: Record<K, V>) => {
                const desiredValues: Record<K, V> = {
                    ...defaultValues
                };
                (Object.keys(overrides) as Array<keyof typeof overrides>)
                    .forEach(id => {
                        if (! (id in defaultValues)) {
                            throw new Error(
                                'An Override value id was registered despite it ' +
                                'being unregistered in the default values.'
                            );
                        }
                        desiredValues[id] = overrides[id];
                    });
                return desiredValues;
            }
        }
    );
    useEffect(() => {
        setOverride(() => {
            return (key: K, value: V) => {
                setOverrides({
                    ...overrides,
                    [key]: value
                });
            }
        });
        setGetDesiredValues(() => {
            return (defaultValues: Record<K, V>) => {
                const desiredValues: Record<K, V> = {
                    ...defaultValues
                };
                (Object.keys(overrides) as Array<keyof typeof overrides>)
                    .forEach(id => {
                        if (! (id in defaultValues)) {
                            throw new Error(
                                'An Override value id was registered despite it ' +
                                'being unregistered in the default values.'
                            );
                        }
                        desiredValues[id] = overrides[id];
                    });
                return desiredValues;
            }
        })
    }, [overrides]);
    return { override, getDesiredValues };
}

export type YupValidationHookResult<T extends Schema> = {
    loading: true;
    error?: undefined;
    value?: undefined;
} | {
    loading: false;
    error: Error;
    value?: undefined;
} | {
    loading: false;
    error?: undefined;
    value: Awaited<ReturnType<T['validate']>>;
}

export function useYupValidation<T extends Schema>(schema: T, value: any): YupValidationHookResult<T> {
    const [loading, setLoading] = useState<boolean>(true);
    const [error, setError] = useState<Error>();
    const [outputValue, setOutputValue] = useState<Awaited<ReturnType<T['validate']>>>();
    useEffect(() => {
        if (loading !== true) {
            setLoading(true);
        }
        if (outputValue !== undefined) {
            setOutputValue(undefined);
        }
        if (error !== undefined) {
            setError(undefined);
        }
        schema.validate(value)
            .then(r => {
                setOutputValue(r);
                setLoading(false);
            })
            .catch(e => {
                setLoading(false);
                setError(e as Error);
            });
    }, [value]);
    return {
        loading,
        error,
        value: outputValue
    } as YupValidationHookResult<T>;
}

export function useSeamless(type: "login" | "signup", cb: () => Promise<boolean>, dependencies: Array<unknown> = []) {
    const settings = useSettings();
    return useTryOnce(settings?.seamless[type], cb, dependencies)
}

export function useTryOnce(enable: boolean | undefined, cb: () => Promise<boolean>, dependencies: Array<unknown> = []) {
    const [running, setRunning] = useState<boolean>(false);
    const [tried, setTried] = useState<boolean>(false);
    const [error, setError] = useState<Error>();
    useEffect(() => {
        if (tried || running) {
            return;
        }
        if (enable === undefined) {
            // loading state
            return;
        }
        if (! enable) {
            setTried(true);
            return;
        }
        const run = async () => {
            setRunning(true);
            try {
                const completed = await cb();
                // when cb returns false, it is an indicator
                // that it is still "waiting" for something.
                // Otherwise, it has executed its "try once" attempt
                if (completed) {
                    setTried(true);
                }
            } catch (e) {
                setError(e as Error);
                setTried(true); // if an error was thrown, consider it as an attempt to "try" once.
            } finally {
                setRunning(false);
            }
        }
        run();
    }, [enable, running, ...dependencies]);
    return { tried, error };
}

export function useCipherbox(channel?: Channel) {
    const [box, setBox] = useState<JWEPacketCipherBox | undefined>(undefined);
    const [loading, setLoading] = useState<boolean>(true);
    useEffect(() => {
        if (!channel) {
            if (box !== undefined) {
                setBox(undefined);
            }
            if (loading === false) {
                setLoading(true);
            }
            return;
        }
        if (! channel.e2eeKey) {
            setLoading(false);
            setBox(undefined);
            return;
        }
        setBox(new JWEPacketCipherBox({
            key: channel.e2eeKey,
            avoidReEncryption: true
        }));
        setLoading(false);
    }, [channel]);
    return {
        loading,
        cipherbox: box
    };
}