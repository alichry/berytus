import { PutField as TPutField, Session, db, Field } from "@root/db/db";
import { useRequest, useAbortRequestOnWindowClose, useNavigateWithPageContextRoute, useSettings } from "@root/hooks";
import { useLiveQuery } from "dexie-react-hooks";
import { useParams } from "react-router-dom";
import Loading from "@components/Loading";
import PutFieldView from "../components/PutFieldView";
import { useCallback, useEffect, useState } from "react";

export default function PutField() {
    const { sessionId, fieldId } = useParams<string>();
    const settings = useSettings();
    const session = useLiveQuery(
        async () => {
            if (! sessionId || ! fieldId) {
                return;
            }
            const record = await db.sessions.get(sessionId);
            if (! record || ! record.putFields) {
                return;
            }
            if (! record.putFields.find(f => f.id === fieldId)) {
                // this is used to wait until the PutField is inserted into
                // the session record. BRTTODO: a better alternative is to use
                // record versioning. This would allow us to setError when
                // the latest session record is retrieved without the corresponding
                // PutField
                return;
            }
            return record;
        },
        [sessionId, fieldId]
    );

    const tabId = session?.context.document.id;
    const navigate = useNavigateWithPageContextRoute();
    const onProcessed = useCallback(() => {
        navigate('/loading');
    }, [navigate]);
    const { maybeResolve, maybeReject } = useRequest<"AccountCreation_AddField">(
        session?.requests[session?.requests.length - 1],
        { onProcessed }
    );
    useAbortRequestOnWindowClose({ maybeReject, tabId });
    const [field, setField] = useState<TPutField>();
    const [error, setError] = useState<Error>();
    const [seamlessTried, setSeamlessTried] = useState<boolean>(false);

    const loaded = session && maybeReject && maybeResolve && settings && field;

    useEffect(() => {
        if (! session) {
            return;
        }
        const f = session.putFields?.find(f => f.id === fieldId);
        if (! f) {
            // This is practically never set since the session object
            // is only set when we find the PutField, hehe.
            setError(new Error('Unable to find PutField ' + fieldId));
            return;
        }
        setField(f);
    }, [session]);

    const submit = async (): Promise<boolean> => {
        try {
            if (! loaded) {
                setError(new Error('Submit called before the goods have been loaded'));
                return false;
            }
            let newField: Field;
            // if (field.type === EBerytusFieldType.Key) {
            //     newField = { options: { ...field.options, private: false }, value: field.value };
            // } else {
            //     newField = { options: field.options, value: field.value };
            // }
            newField = { ...field }
            const change: Pick<Session, 'fields'> = {
                fields: (session.fields || []).concat(newField)
            };
            await db.sessions.update(session.id, change);
            const { sent } = maybeResolve(null);
            if (! sent) {
                setError(new Error("Unable to resolve request; this request has been already resolved."));
                return false;
            }
            return true;
        } catch (e) {
            setError(e as Error);
            return false;
        }
    };

    useEffect(() => {
        if (! loaded) {
            return;
        }
        const run = async (): Promise<boolean> => {
            if (settings.seamless.signup) {
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
        // If error is set while loaded = false, we should show it
        // This shouldn't happen until we change how we are retrieving
        // the session obj-PutField. Check comments above.
        return <Loading />
    }
    if (! session.createFieldOptions?.length) {
        return <p className="text-lg red text-center mt-4">Where createFieldOptions?</p>
    }

    return <PutFieldView
        uri={session.context.document.uri}
        field={field}
        value={field.value}
        onCancel={() => window.close()}
        onSubmit={submit}
        error={error}
    />;
}