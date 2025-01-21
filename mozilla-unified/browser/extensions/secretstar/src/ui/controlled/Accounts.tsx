import { useLiveQuery } from "dexie-react-hooks";
import AccountListView from "../components/AccountListView";
import { db } from "@root/db/db";
import { useNavigateWithPageContextRoute } from "@root/hooks";
import { useState } from "react";

export default function Accounts() {
    const [deleting, setDeleting] = useState<boolean>(false);
    const accounts = useLiveQuery(
        async () => {
            const accounts = await db.accounts.toArray();
            return accounts;
        }
    );
    const navigate = useNavigateWithPageContextRoute();

    return <AccountListView
        accounts={accounts || []}
        onClick={(acc) => navigate(`/account/${acc.id}`)}
        deleting={deleting}
        onDeleteAll={async () => {
            if (! accounts || accounts.length === 0) {
                return;
            }
            setDeleting(true);
            await db.accounts.bulkDelete(
                accounts.map(a => a.id)
            );
            setDeleting(false);
        }}
    />
}