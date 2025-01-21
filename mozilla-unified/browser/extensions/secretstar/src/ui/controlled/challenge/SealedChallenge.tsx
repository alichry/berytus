import { useEffect, useState } from "react";
import { useParams } from "react-router-dom";
import { useLiveQuery } from "dexie-react-hooks";
import { db } from "@root/db";
import RespondToMessageView from "@root/ui/components/RespondToMessageView";
import Loading from "@root/ui/components/Loading";
import Notice from "@root/ui/components/Notice";
import { useRequest } from "@root/hooks";

export default function SealedChallenge() {
    const { sessionId, challengeId } = useParams();
    const [error, setError] = useState<Error>();
    const query = useLiveQuery(async () => {
        if (! sessionId || ! challengeId) {
            return;
        }
        const record = await db.sessions.get(sessionId);
        if (! record) {
            setError(new Error('Session not found'));
            return;
        }
        if (! record.challenges) {
            setError(new Error('Bad session. No challenges were found'));
            return;
        }
        const challenge = record.challenges[challengeId];
        if (! challenge) {
            setError(new Error('Bad session. Could not find challenge id: ' + challengeId));
            return;
        }
        if (! challenge.closed) {
            // challenge not yet updated. We wait till its update so we can
            // get the latest request (and resolve it).
            return;
        }
        return { session: record, challenge };
    }, [sessionId, challengeId]);
    const { maybeResolve, maybeReject } = useRequest(query?.session.requests[query?.session.requests.length - 1]);
    useEffect(() => {
        if (! maybeResolve) {
            return;
        }
        maybeResolve();
    }, [maybeResolve]);
    const loading = ! query || ! maybeResolve || ! maybeReject;

    if (loading) {
        return <Loading />;
    }
    const { challenge, session } = query;

    return <RespondToMessageView challengeType={challenge.type}>
        <Notice
            type="info"
            text={`Challenge sealed successfully 🎉`}
        />
        { error ? (
            <Notice
                className="mb-3"
                type="error"
                text={`QueryError: ${error.message}`}
            />
        ) : null}
    </RespondToMessageView>
}