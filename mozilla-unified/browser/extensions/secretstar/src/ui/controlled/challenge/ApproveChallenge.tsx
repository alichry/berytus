import { useParams } from "react-router-dom";
import Loading from "../../components/Loading";
import ApproveChallengeView from "../../components/ApproveChallengeView";
import { useLiveQuery } from "dexie-react-hooks";
import { db } from "@root/db";
import { useState } from "react";
import { useAbortRequestOnWindowClose, useNavigateWithPopupContextAndPageContextRoute, useRequest, useSeamless, useSettings } from "@root/hooks";
import { atLeastOneErrorSet } from "@root/utils";
import ConcatErrorMessages from "@root/ui/components/ConcatErrorMessages";

export default function ApproveChallenge() {
    const { sessionId, challengeId } = useParams();
    const [error, setError] = useState<Error>();
    const query = useLiveQuery(async () => {
        if (
            sessionId === undefined ||
            challengeId === undefined
        ) {
            return;
        }
        const record = await db.sessions.get(sessionId);
        if (! record) {
            setError(new Error('Session was not found!'));
            return;
        }
        if (! record.challenges) {
            setError(new Error('Session is missing a challenge dictionary! What?'));
            return;
        }
        if (! (challengeId in record.challenges)) {
            // challenge not added yet
            return;
        }
        const challenge = record.challenges[challengeId];
        return {
            session: record,
            challenge
        };
    }, [sessionId, challengeId]);
    const request = query?.session.requests[query!.session.requests.length - 1];
    const { maybeResolve, maybeReject } = useRequest(request);
    const tabId = query?.session.context.document.id;
    useAbortRequestOnWindowClose({ maybeReject, tabId });
    const navigate = useNavigateWithPopupContextAndPageContextRoute(tabId);

    const loading = query === undefined || maybeResolve === undefined || maybeReject === undefined;

    const approve = () => {
        if (loading) {
            setError(new Error('Approve called before all the goods have been loaded'));
            return;
        }
        maybeResolve(undefined);
        navigate('/loading');
    }

    const { tried: seamlessTried, error: seamlessError } = useSeamless("login", async () => {
        if (loading) {
            return false;
        }
        approve();
        return true;
    }, [loading]);

    const errors = [error, seamlessError];
    if (atLeastOneErrorSet(errors)) {
        return <ConcatErrorMessages errors={errors} />
    }

    if (loading || ! seamlessTried) {
        return <Loading />;
    }

    const { challenge, session } = query;

    return (
        <ApproveChallengeView
            challenge={challenge}
            onApprove={() => approve()}
            onReject={() => window.close()}
        />
    );
}