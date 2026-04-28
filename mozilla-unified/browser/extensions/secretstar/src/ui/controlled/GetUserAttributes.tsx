import { useParams } from "react-router-dom";
import { useAbortRequestOnWindowClose, useRequest, useNavigateWithPageContextRoute, useSettings, useIdentity, useCipherbox } from "../../hooks";
import GetUserAttributesView from "@components/GetUserAttributesView";
import type { UserAttributeKey, UserAttributes as BerytusUserAttributes } from "@berytus/types";
import { UserAttribute } from "@root/db/db";
import { Session, db, RRequiredUserAttributes, RUserAttributes } from '@root/db/db';
import { useLiveQuery } from "dexie-react-hooks";
import Loading from "../components/Loading";
import { UserAttributes } from "@root/db";
import { useCallback, useEffect, useState } from "react";
import { userAttributesLabels } from "../utils/userAttributesLabels";
import { stringifyUserAttributeValue } from "../components/AccountView";
import { InternalError } from "@root/errors/InternalError";
import ErrorContainer from "../components/ErrorContainer";

function getMissingAttributes(requiredAttributes: RRequiredUserAttributes, userAttributes: Partial<UserAttributes>) {
    return (Object.keys(requiredAttributes) as Array<keyof typeof requiredAttributes>)
        .map(key => {
            const attr = userAttributes[key];
            if (
                requiredAttributes[key] &&
                (! attr || stringifyUserAttributeValue(attr).length === 0)
            ) {
                return key;
            }
            return null;
        })
        .filter(key => !!key);
}

function hasAllRequiredUserAttributes(requiredAttributes: RRequiredUserAttributes, attributes: Partial<UserAttributes>) {
    const missing =  getMissingAttributes(requiredAttributes, attributes);
    return missing.length === 0;
}

function isAllAttributesOptional(requiredAttributes: RRequiredUserAttributes): boolean {
    return (Object.values(requiredAttributes) as Array<boolean>)
        .every(v => v === false);
}

export default function GetUserAttributes() {
    const settings = useSettings();
    const identity = useIdentity();
    const [error, setError] = useState<Error>();
    const { sessionId, afterVersion } = useParams<string>();
    const query = useLiveQuery(
        async () => {
            if (! sessionId) {
                return;
            }
            const record = await db.sessions.get(sessionId);
            if (! record) {
                return;
            }
            if (afterVersion && record.version <= Number(afterVersion) ) {
                return;
            }
            const channel = await db.channel.get(record.channel.id);
            if (! channel) {
                setError(new Error('Channel not found!'));
                return;
            }
            return {
                session: record,
                channel
            };
        }
    );
    const { session, channel } = query || {};
    const tabId = session?.context.document.id;
    const navigate = useNavigateWithPageContextRoute();
    const onProcessed = useCallback(() => {
        navigate('/loading');
    }, [navigate]);
    const { cipherbox, loading: cipherboxLoading } = useCipherbox(channel);
    const preResolveCb = useCallback(async (value: BerytusUserAttributes): Promise<BerytusUserAttributes> => {
        if (cipherboxLoading) {
            throw new InternalError("Cipherbox not loaded in CreateField preResolve()");
        }
        if (! cipherbox) {
            return value; // e2e not enabled
        }
        return Promise.all(value.map(async (v) => {
            const encrypted = await cipherbox.encrypt(v.value);
                if (encrypted === null) {
                throw new InternalError("cipherbox.encrypt() mistakenly returned null for non-null value");
            }
            return {
                ...v,
                value: encrypted
            };
        }))
    }, [cipherboxLoading, cipherbox]);
    const { maybeResolve, maybeReject } = useRequest<"AccountCreation_GetUserAttributes">(
        session?.requests[session?.requests.length - 1],
        {
            onProcessed,
            preResolve: preResolveCb
        }
    );
    useAbortRequestOnWindowClose({ maybeReject, tabId });
    const [seamlessTried, setSeamlessTried] = useState<boolean>(false);
    const requiredUserAttributes = session?.requiredUserAttributes;
    const user = identity?.userAttributes;
    const loaded = !cipherboxLoading && settings && maybeResolve && maybeReject && session && requiredUserAttributes && identity;

    const approve = useCallback(async (selectedUserAttributes: UserAttributeKey[], overrides: Partial<Record<UserAttributeKey, UserAttribute>>): Promise<boolean> => {
        if (! loaded) {
            setError(new Error('Approve called before all the goods have been loaded'));
            return false;
        }
        const transformedUser = (Object.keys(userAttributesLabels) as Array<keyof typeof userAttributesLabels>)
            .reduce((prev, curr) => {
                if (selectedUserAttributes.indexOf(curr) === -1) {
                    return prev;
                }
                const customValue = overrides[curr];
                if (customValue !== undefined) {
                    prev[curr] = {
                        ...customValue
                    };
                    return prev;
                }
                prev[curr] = user ? user[curr] : undefined;
                return prev;
            }, {} as Partial<RUserAttributes>);
        if (typeof transformedUser.picture === "string") {
            const picture = await db.picture.get(transformedUser.picture);
            if (! picture) {
                setError(new Error('Cannot find picture!'));
                return false;
            }
            transformedUser.picture = {
                id: "picture",
                mimeType: picture.type,
                value: db.pictureToDataUrl(picture)
            };
        }
        const missingAttributes = getMissingAttributes(requiredUserAttributes, transformedUser);
        if (missingAttributes.length > 0) {
            setError(new Error('The following fields are required: ' + missingAttributes.join(', ') + '. Please enter a value before clicking approve.'))
            return false;
        }
        const change: Pick<Session, 'userAttributes'> = {
            userAttributes: transformedUser
        };
        await db.sessions.update(session.id, change);
        const asArray = Object.values(transformedUser).filter(u => !!u);
        const { sent } = maybeResolve(asArray as Array<Exclude<typeof asArray[0], undefined>>);
        if (! sent) {
            setError(new Error('Unable to resolve request. The request has been previously resolved.'));
            return false;
        }
        return true;
    }, [maybeResolve]);

    useEffect(() => {
        if (! loaded) {
            return;
        }
        const run = async (): Promise<boolean> => {
            if (
                settings.seamless.signup &&
                hasAllRequiredUserAttributes(
                    requiredUserAttributes,
                    user || {}
                )
            ) {
                return approve((Object.keys(requiredUserAttributes) as Array<keyof typeof session.requiredUserAttributes>), {});
            }
            if (isAllAttributesOptional(requiredUserAttributes) && (!user || Object.keys(user).length === 0)) {
                return approve([], {});
            }
            return false;
        }
        run()
            .then((submitted) => {
                if (! submitted) {
                    setSeamlessTried(true);
                }
            })
            .catch(e => {
                setSeamlessTried(true);
                setError(e);
            })
    }, [approve, loaded]);

    if (! loaded || ! seamlessTried) {
        if (error) {
            return <ErrorContainer error={error} />;
        }
        // BRTTODO: Put an error since the session record should exist by now.
        // Or is that the useLiveQuery is still fetching obj...
        // Maybe use a different hook.
        return <Loading />
    }

    return <div>
        <GetUserAttributesView
            onCancel={() => window.close()}
            requiredAttributes={requiredUserAttributes}
            userAttributes={user}
            onApprove={approve}
            onReject={() => window.close()}
            error={error}
        />
    </div>
}