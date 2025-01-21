import { ClassNameProp } from "@props/ClassNameProp";
import styles from '@styles/IntentAction.module.scss';
import { cn } from "@root/ui/utils/joinClassName";
import Button from "./Button";
import { Account } from "@root/db";
import { AccountIdCardView } from "./AccountIdCardView";
import { ELoginUserIntent, EBerytusFieldType } from "@berytus/enums";

export interface IntentActionProps extends ClassNameProp {
    checkpointCapability: (typeof ELoginUserIntent)[keyof typeof ELoginUserIntent];
    accounts?: Account[];
    onLoginIntent: (selectedAccount: Account) => void;
    onRegisterIntent: () => void;
}

export default function IntentAction({ checkpointCapability, accounts = [], onLoginIntent, onRegisterIntent, className }: IntentActionProps) {
    if (
        checkpointCapability === ELoginUserIntent.Authenticate &&
        accounts.length === 0
    ) {
        return <div className={cn(styles.container, className)}>
            <div>
                <p className="text-base text-center my-4 text-red-400">Oh no!</p>
                <p className="text-sm my-4 text-red-400">The web application is not accepting new account registrations and you currently do not have any saved accounts in the database.</p>
            </div>
        </div>
    }
    if (checkpointCapability === ELoginUserIntent.Register) {

    }
    const canAuth = accounts.length > 0 && checkpointCapability !== ELoginUserIntent.Register;
    const canRegister = checkpointCapability !== ELoginUserIntent.Authenticate;
    return (
        <div className={cn(styles.container, className)}>
            {canAuth ? (
                <div>
                    <p className="text-base my-4">Login with an existing account</p>
                    {/* <AccountSelectorTableView */}
                    <AccountSelectorListView
                        accounts={accounts}
                        onSelect={onLoginIntent}
                    />
                </div>
            ) : null}
            {canRegister ? (
                <div>
                    {canAuth
                        ? <p className="text-base mb-3">
                            Or, create a new account
                        </p>
                        : <p className="text-sm mb-3">
                            You currently do not have any accounts associated with the above domain, would you like to create a new account?
                        </p>
                    }
                    <Button
                        text="Sign up"
                        className="w-full"
                        onClick={() => onRegisterIntent()}
                    />
                </div>
            ) : null}
        </div>
    )
}

interface AccountSelectorViewProps {
    accounts: Account[];
    onSelect(selectedAccount: Account): void;
}

function AccountSelectorTableView({ accounts, onSelect }: AccountSelectorViewProps) {
    return (
        <div className={styles.savedAccountsContainer}>
            {accounts.map((a,i) => <p
                key={i}
                className={styles.savedAccountRow}
                onClick={() => onSelect(a)}
                >
                    {a.fields.find(
                        a => (a.type === EBerytusFieldType.Identity || a.type === EBerytusFieldType.ForeignIdentity)
                    )?.value || "Error: Where is identity?"}
                </p>
            )}
        </div>
    );
}

function AccountSelectorListView({ accounts, onSelect }: AccountSelectorViewProps) {
    return (
        <div className="max-h-36 overflow-auto">
            {accounts.map((a,i) =>
                <AccountIdCardView
                    key={i}
                    overrideColor="bg-[#870049]"
                    account={a}
                    onClick={() => {
                        onSelect(a);
                    }}
                />
            )}
        </div>
    )
}