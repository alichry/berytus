import UserIntentView from "@components/UserIntentView";
import { useParams } from "react-router-dom";
import { useAbortRequestOnWindowClose, useRequest, useNavigateWithPopupContextAndPageContextRoute, useAccounts, useNavigateWithPageContextRoute } from "../../hooks";
import { Session, db } from "@root/db/db";
import { useLiveQuery } from "dexie-react-hooks";
import Loading from "../components/Loading";
import { ELoginUserIntent } from "@berytus/enums";
import { useCallback } from "react";

export default function UserIntent() {
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
    const accounts = useAccounts(
        session?.channel.webAppActor,
        session?.channel.constraints?.account?.category,
        session?.channel.constraints?.account?.schemaVersion
    );
    const tabId = session?.context.document.id;
    const navigate = useNavigateWithPageContextRoute();
    const onProcessed = useCallback(() => {
            navigate('/loading');
        }, [navigate]);
    const { maybeResolve, maybeReject } = useRequest<"Login_ApproveOperation">(
        session?.requests[0],
        { onProcessed }
    );
    useAbortRequestOnWindowClose({ maybeReject, tabId });
    console.log(!!session, !!maybeResolve, !!maybeReject);
    if (! session || ! maybeResolve || ! maybeReject) {
        // BRTTODO: Put an error since the session record should exist by now.
        // Or is that the useLiveQuery is still fetching obj...
        // Maybe use a different hook.
        return <Loading>
            <p className="text-lg text-center mt-4">Fetching...</p>
        </Loading>
    }
    return <UserIntentView
        accounts={accounts}
        uri={session.context.document.uri}
        checkpointCapability={session.operation.intent}
        onIntent={async (intent, account) => {
            if (intent === ELoginUserIntent.Authenticate) {
                if (! account) {
                    throw new Error('Expecting account to be set when intent = auth');
                }
                const change: Pick<Session, 'selectedAccountId'> = {
                    selectedAccountId: account?.id
                };
                await db.sessions.update(session,  change);
            }
            maybeResolve(intent);
        }}
        onCancel={() => window.close()}
    />
}