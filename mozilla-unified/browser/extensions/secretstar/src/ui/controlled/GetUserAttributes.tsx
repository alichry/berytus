import { useParams } from "react-router-dom";
import { useAbortRequestOnWindowClose, useRequest, useNavigateWithPopupContextAndPageContextRoute, useSettings, useIdentity } from "../../hooks";
import GetUserAttributesView from "@components/GetUserAttributesView";
import type { UserAttributeKey } from "@berytus/types";
import { UserAttribute } from "@root/db/db";
import { Session, db, RRequiredUserAttributes, RUserAttributes } from '@root/db/db';
import { useLiveQuery } from "dexie-react-hooks";
import Loading from "../components/Loading";
import { UserAttributes } from "@root/db";
import { useEffect, useState } from "react";
import { userAttributesLabels } from "../utils/userAttributesLabels";
import { stringifyUserAttributeValue } from "../components/AccountView";

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
        })
        .filter(key => !!key);
}

function hasAllRequiredUserAttributes(requiredAttributes: RRequiredUserAttributes, attributes: Partial<UserAttributes>) {
    const missing =  getMissingAttributes(requiredAttributes, attributes);
    return missing.length === 0;
}

export default function GetUserAttributes() {
    const settings = useSettings();
    const identity = useIdentity();
    const [error, setError] = useState<Error>();
    const { sessionId, afterVersion } = useParams<string>();
    const session = useLiveQuery(
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
            return record;
        }
    );
    const tabId = session?.context.document.id;
    const { maybeResolve, maybeReject } = useRequest(session?.requests[session?.requests.length - 1]);
    useAbortRequestOnWindowClose({ maybeReject, tabId });
    const navigate = useNavigateWithPopupContextAndPageContextRoute(tabId);
    const [seamlessTried, setSeamlessTried] = useState<boolean>(false);
    const requiredUserAttributes = session?.requiredUserAttributes;
    const user = identity?.userAttributes;
    const loaded =  settings && maybeResolve && maybeReject && session && requiredUserAttributes && identity;

    const approve = async (selectedUserAttributes: UserAttributeKey[], overrides: Partial<Record<UserAttributeKey, UserAttribute>>): Promise<boolean> => {
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
        if (maybeResolve(transformedUser)) {
            navigate('/loading');
            return true;
        }
        setError(new Error('Refusing to resolve request. The request has been previously resolved.'));
        return false;
    }

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
    }, [loaded]);

    if (! loaded || ! seamlessTried) {
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