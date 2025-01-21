import { db } from "@root/db/db";
import { useLiveQuery } from "dexie-react-hooks";
import { useParams } from "react-router-dom";
import Loading from "@components/Loading";
import AccountView from "../components/AccountView";
import { useNavigateWithPageContextRoute } from "@root/hooks";
import { useState } from "react";

export default function Account() {
    const { accountId } = useParams<string>();
    const navigate = useNavigateWithPageContextRoute();
    const [deleting, setDeleting] = useState<boolean>(false);

    const account = useLiveQuery(
        async () => {
            if (! accountId) {
                return;
            }
            const accounts = await db.accounts.get(accountId);
            return accounts;
        }
    );
    if (! account) {
        return <Loading />;
    }

    return <AccountView
        account={account}
        deleting={deleting}
        onDelete={async () => {
            setDeleting(true);
            await db.accounts.delete(account.id);
            setDeleting(false);
            navigate("/account");
        }}
    />;
}