import { db } from "@root/db";
import { useAbortRequestOnWindowClose, useNavigateWithPageContextRoute, useRequest, useSeamless, useSettings } from "@root/hooks";
import { useLiveQuery } from "dexie-react-hooks";
import { useParams } from "react-router-dom";
import Loading from "../components/Loading";
import TransitionToAuthView from "../components/TransitionToAuthView";
import { useState } from "react";
import { atLeastOneErrorSet } from "@root/utils";
import ConcatErrorMessages from "../components/ConcatErrorMessages";

export default function TransitionToAuth() {
    const navigate = useNavigateWithPageContextRoute();
    const { currentSessionId, nextSessionId } = useParams<string>();
    const [error, setError] = useState<Error | undefined>();
    const records = useLiveQuery(
        async () => {
            if (! currentSessionId || ! nextSessionId) {
                return;
            }
            const nextSession = await db.sessions.get(nextSessionId);
            if (! nextSession) {
                return;
            }
            const currSession = await db.sessions.get(currentSessionId);
            if (! currSession) {
                return;
            }
            if (! nextSession.selectedAccountId) {
                setError(new Error('Expecting selectedAccountId to be set in nextSession'));
                return;
            }
            const account = await db.accounts.get(nextSession.selectedAccountId);
            if (! account) {
                setError(new Error(
                    'Expecting selectedAccountId to point to a valid account in nextSession'
                ));
                return;
            }
            return { currSession, nextSession, account };
        }
    );
    const { maybeResolve, maybeReject } = useRequest<"AccountCreation_ApproveTransitionToAuthOp">(records?.nextSession.requests[records?.nextSession?.requests.length - 1]);
    const tabId =  records?.currSession.context.document.id;
    useAbortRequestOnWindowClose({ maybeReject, tabId });
    const loading = ! records || ! maybeReject || ! maybeResolve;
    const confirm = () => {
        if (loading) {
            throw new Error('Confirm called before all the goods have been loadded');
        }
        maybeResolve()
        navigate('/loading')
    }
    const { tried: seamlessTried, error: seamlessError } = useSeamless("login", async () => {
        if (loading) {
            return false;
        }
        confirm();
        return true;
    }, [loading])

    const errors = [error, seamlessError];
    if (atLeastOneErrorSet(errors)) {
        return <ConcatErrorMessages errors={errors} />
    }

    if (loading || ! seamlessTried) {
        return <Loading />;
    }

    return <TransitionToAuthView
        onCancel={() => window.close()}
        onConfirm={() => confirm()}
        onDeny={() => window.close()}
        selectedAccount={records.account}
    />;
}