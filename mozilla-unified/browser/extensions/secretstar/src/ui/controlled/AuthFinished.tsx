import { useEffect } from "react";
import Breadcrumbs from "../components/Breadcrumbs";
import { Container } from "../components/Container";
import Notice from "../components/Notice";
import TopBar from "./TopBar";
import { faKey } from "@fortawesome/free-solid-svg-icons";
import { useParams } from "react-router-dom";
import { useLiveQuery } from "dexie-react-hooks";
import { db } from "@root/db/db";
import { useRequest } from "@root/hooks";
import Loading from "../components/Loading";

export default function AuthFinished() {
    const { sessionId } = useParams();
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
    const { maybeResolve, maybeReject } = useRequest<"AccountAuthentication_CloseChallenge">(session?.requests[session?.requests.length - 1]);
    useEffect(() => {
        if (! maybeResolve) {
            return;
        }
        maybeResolve();
    }, [maybeResolve]);
    if (! session || ! maybeReject || ! maybeResolve) {
        return <Loading />;
    }
    return (
        <Container>
            <TopBar />
            <div className="mx-8">
                <Breadcrumbs
                    headerIcon={faKey}
                    locations={[
                        "Authentication",
                        "Finished"
                    ]}
                />
            </div>
            <div className="mx-6">
                <Notice
                    type="info"
                    text="Authentication succeeded! 🎉"
                />
            </div>
        </Container>
    )
}