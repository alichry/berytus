import { db } from "@root/db";
import { useRequest } from "@root/hooks";
import { useLiveQuery } from "dexie-react-hooks";
import { useParams } from "react-router-dom";
import Loading from "../components/Loading";
import EnableE2EView from "../components/EnableE2EView";

export default function EnableE2E() {
    const { sessionId } = useParams<string>();
    const records = useLiveQuery(
        async () => {
            if (! sessionId) {
                return;
            }
            const session = await db.sessions.get(sessionId);
            if (! session) {
                return;
            }
            const channelId = session.channel.id;
            const channel = await db.channel.get(channelId);
            if (! channel) {
                throw new Error('Where is the channel?');
            }
            if (! channel.webAppSignature) {
                return;
            }
            return { session, channel };
        }
    );
    const { maybeResolve, maybeReject } = useRequest(records?.session?.requests[records?.session?.requests.length - 1]);
    if (! records || ! maybeReject || ! maybeResolve) {
        return <Loading />;
    }
    return <EnableE2EView />;
}