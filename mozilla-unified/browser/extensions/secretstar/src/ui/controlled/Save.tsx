import { useParams } from 'react-router-dom';
import { useLiveQuery } from 'dexie-react-hooks';
import { Session, db } from "@root/db/db";
import Loading from "../components/Loading";
import { useEffect } from "react";
import { useNavigateWithPageContextRoute, useRequest } from '@root/hooks';

export default function Save() {
    const navigate = useNavigateWithPageContextRoute();
    // TODO: No need to pass accountId as parameer.
    // it is saved by bg script in sess record
    const { sessionId, accountId } = useParams<string>();
    const session = useLiveQuery(
        async () => {
            if (! sessionId) {
                return;
            }
            const record = await db.sessions.get(sessionId);
            if (! record) {
                return;
            }
            if (! record.closed) {
                return;
            }
            return record;
        }
    );
    const { maybeResolve, maybeReject } = useRequest<"Login_CloseOperation">(session?.requests[session?.requests.length - 1]);
    useEffect(() => {
        if (! session || ! accountId || ! maybeResolve) {
            return;
        }
        const change: Pick<Session, 'version' | 'saved'> = {
            saved: true,
            version: session.version + 1
        };
        db.sessions.update(session.id, change)
            .then(() => maybeResolve().sent && navigate('/account/' + accountId))
    }, [session, maybeResolve]);

    return <Loading />;
}