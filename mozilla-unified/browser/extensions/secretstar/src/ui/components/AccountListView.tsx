import { Account } from "@root/db/db";
import { Container } from "./Container";
import TopBar from "../controlled/TopBar";
import Breadcrumbs from "./Breadcrumbs";
import { faUser } from "@fortawesome/free-regular-svg-icons";
import { ValueColumn } from "./CondensedTable";
import { faFolderOpen } from "@fortawesome/free-regular-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import Table from "./Table";
import CommonBodyContainer from "./CommonBodyContainer";
import Button from "./Button";
import { EBerytusFieldType } from "@berytus/enums";

export interface AccountListViewProps {
    accounts: Array<Account>;
    onClick: (account: Account) => void;
    deleting: boolean;
    onDeleteAll(): void;
}

const getAccountFirstIdentity = (acc: Account) => {
    const field = acc.fields.find(( { type } ) => type === EBerytusFieldType.Identity || type === EBerytusFieldType.ForeignIdentity);
    if (! field) {
        return null;
    }
    return field.value as string;
}

export default function AccountListView({ accounts, onClick, onDeleteAll, deleting }: AccountListViewProps) {
    return (
        <Container>
            <TopBar />
            <CommonBodyContainer>
                <Breadcrumbs
                    headerIcon={faUser}
                    locations={[
                        "Accounts",
                        "List"
                    ]}
                />

                <Table
                    className="mb-8 text-sm"
                    scrollable={{
                        maxWidth: "max-w-sm",
                        maxHeight: "max-h-60",
                        fullWidth: true,
                        fullHeight: true
                    }}
                    dataRowClassName="hover:cursor-pointer [&:hover>*]:!bg-violet-800 [&:hover]:!bg-violet-800  hover:!text-white"
                    //dataRowColClassName="peer hover:!bg-violet-800 "
                    cols={
                        [
                            <ValueColumn text="#" className="text-sm" />,
                            <ValueColumn text="Domain" className="text-sm" />
                        ]
                    }
                    rows={
                        Object.values(accounts).map(account => ([
                            <ValueColumn
                                onClick={() => onClick(account)}
                                textSize="text-sm"
                                text={
                                    <div className="flex flex-row">
                                        <FontAwesomeIcon
                                            icon={faFolderOpen}
                                            className="cursor-pointer mt-1 mr-3"
                                            />
                                        {getAccountFirstIdentity(account) || <div className="italic color-black">Empty</div>}
                                    </div>
                                }
                            />,
                            <ValueColumn
                                textSize="text-sm"
                                onClick={() => onClick(account)}
                                text={"originalUri" in account.webAppActor
                                    ? account.webAppActor.originalUri.hostname
                                    : account.webAppActor.ed25519Key
                                }
                            />,
                        ]))
                    }
                />
                { accounts.length > 0 ? (
                    <div className="flex flex-row mt-2">
                        <div className="flex-1"/>
                        <Button
                            color="red"
                            text="Delete All"
                            disabled={deleting}
                            onClick={() => {
                                onDeleteAll();
                            }}
                        />
                    </div>
                ) : null}

            </CommonBodyContainer>
        </Container>
    );
}