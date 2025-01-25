import { useCallback, useEffect, useState } from "react";
import { useParams } from "react-router-dom";
import { useLiveQuery } from "dexie-react-hooks";
import { db } from "@root/db";
import RespondToMessageView from "@root/ui/components/RespondToMessageView";
import Loading from "@root/ui/components/Loading";
import Notice from "@root/ui/components/Notice";
import { useRequest, useSettings } from "@root/hooks";
import Button from "@root/ui/components/Button";
import { useHourglass } from "./AbortedChallenge";

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
    const { maybeResolve, maybeReject, processed, processing } = useRequest<"AccountAuthentication_CloseChallenge">(query?.session.requests[query?.session.requests.length - 1]);
    const [seamlessTried, setSeamlessTried] = useState<boolean>(false);
    const settings = useSettings();
    const [buttonText, setButtonText] = useState<string>("Okay (5)");
    const loaded = !!(query && maybeResolve && maybeReject && settings);
    const submit = async () => {
        if (! loaded) {
            setError(new Error('Submit called before the goods have been loaded'));
            return false;
        }
        try {
            const { sent } = maybeResolve();
            if (! sent) {
                setError(new Error("Unable to resolve request; request already consumed."));
            }
        } catch (e) {
            setError(e as Error);
        }
        return true;
    }
    const timedCb = useCallback((secondsLeft: number) => {
        if (secondsLeft === 0) {
            if (! processed) {
                submit();
            }
            setButtonText(`...`);
            return;
        }
        setButtonText(`Okay (${secondsLeft})`);
    }, [setButtonText, processed, submit]);
    useHourglass(5, timedCb, seamlessTried && loaded);

    useEffect(() => {
        if (! loaded) {
            return;
        }
        const run = async (): Promise<boolean> => {
            if (settings.seamless.login) {
                return submit();
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
            });
    }, [loaded]);

    if (! loaded || ! seamlessTried) {
        return <Loading />;
    }
    const { challenge } = query;

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
        { ! processed && ! processing ? (
            <Button
                text={buttonText}
                className="w-full mb-3"
                onClick={() => {
                    submit();
                }}
            />
        ) : null}
    </RespondToMessageView>
}