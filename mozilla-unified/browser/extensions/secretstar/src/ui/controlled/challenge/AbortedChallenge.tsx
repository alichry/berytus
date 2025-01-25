import { useCallback, useEffect, useState } from "react";
import { useParams } from "react-router-dom";
import { useLiveQuery } from "dexie-react-hooks";
import { db } from "@root/db";
import RespondToMessageView from "@root/ui/components/RespondToMessageView";
import Loading from "@root/ui/components/Loading";
import Notice from "@root/ui/components/Notice";
import { useRequest, useSettings } from "@root/hooks";
import Button from "@root/ui/components/Button";

export function useHourglass(
    seconds: number,
    cb: (secondsLeft: number) => void,
    enabled: boolean = true
) {
    const [count, setCount] = useState<number | undefined>();
    useEffect(() => {
        if (! enabled) {
            setCount(undefined);
            return;
        }
        setCount(seconds);
    }, [seconds, enabled]);
    useEffect(() => {
        if (count === undefined) {
            return;
        }
        cb(count);
        if (count === 0) {
            return;
        }
        const handle = setTimeout(() => {
            setCount(count - 1);
        }, 1000);
        return () => clearInterval(handle);
    }, [count, cb]);
}

export default function AbortedChallenge() {
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
        if (challenge.abortionReasonCode) {
            // challenge not yet updated. We wait till its update so we can
            // get the latest request (and resolve it).
            return;
        }
        return { session: record, challenge };
    }, [sessionId, challengeId]);
    const settings = useSettings();
    const { maybeResolve, maybeReject, processed, processing } = useRequest<"AccountAuthentication_AbortChallenge">(query?.session.requests[query?.session.requests.length - 1]);
    // TODO(berytus): Use useSeamless instead, check other components too.
    const [seamlessTried, setSeamlessTried] = useState<boolean>(false);
    const submit = async () => {
        if (! loaded) {
            setError(new Error('Submit called before the goods have been loaded'));
            return false;
        }
        try {
            const { sent } = maybeResolve();
            if (! sent) {
                setError(new Error("Unable to resolve request; request already consumed."));
                return false;
            }
            return true;
        } catch (e) {
            setError(e as Error);
            return false;
        }
    }
    const loaded = !!(query && maybeResolve && maybeReject && settings);
    const [buttonText, setButtonText] = useState<string>("Okay (5)");
    const timedCb = useCallback((secondsLeft: number) => {
        if (secondsLeft === 0) {
            if (! processed && ! processing) {
                submit();
            }
            setButtonText(`...`);
            return;
        }
        setButtonText(`Okay (${secondsLeft})`);
    }, [setButtonText, processed, processing, submit]);
    useHourglass(5, timedCb, seamlessTried && loaded );

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
            type="error"
            text={`Unfortunately, the web application aborted the challenge with the following code: ${challenge.abortionReasonCode}`}
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