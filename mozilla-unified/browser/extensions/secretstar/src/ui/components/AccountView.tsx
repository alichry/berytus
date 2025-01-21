import { Account, Field, fieldOptionValueToString } from "@root/db/db";
import { Container } from "./Container";
import TopBar from "../controlled/TopBar";
import Breadcrumbs from "./Breadcrumbs";
import Folder from "./Folder";
import { faLocationDot, faKey, faUser, faArrowRight, faArrowLeft } from "@fortawesome/free-solid-svg-icons"
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import KeyValue from "./KeyValue";
import { cn } from "../utils/joinClassName";
import { userAttributesLabels } from "../utils/userAttributesLabels";
import Select from "./Select";
import { useState } from 'react';
import { fieldOptionsLabels } from "../utils/fieldOptionsLabels";
import { ab2base64, formatBase64AsPem } from "@root/key-utils";
import { ClassNameProp } from "../props/ClassNameProp";
import { PropsWithChildren } from 'react';
import SelectContent from "./SelectContent";
import Button from "./Button";
import { EBerytusFieldType } from "@berytus/enums";
import { UserAttribute } from "@root/db/db";

export interface AccountViewProps {
    account: Account;
    deleting: boolean;
    onDelete(): void;
}

// TODO(berytus): Move this somewhere else
export const stringifyUserAttributeValue = (attr: UserAttribute) => {
    if (typeof attr.value === "string") {
        return attr.value;
    }
    if (attr.value instanceof ArrayBuffer) {
        return ab2base64(attr.value);
    }
    return `TODO(berytus): Stringify Encrypted Packet in User Attributes`;
}

function BagedFieldValue({ children, className }: PropsWithChildren<ClassNameProp>) {
    return (
        <p className={cn('text-[#57ff99] bg-black select-text py-0.5 px-1.5 mt-0.5 rounded', className)}>
            {children}
        </p>
    );
}

export default function AccountView({ account, onDelete, deleting }: AccountViewProps) {
    // TODO: not pure.
    const [fieldPos, setFieldPos] = useState<number>(0);
    const keyValueProps = {
        className:"text-xs",
        valueClassName:"text-sm font-value",
        dir:"col",
        gap: 'gap-0',
        pairGap: 'gap-4',
        pairDir: "row"
    } as const;
    return <Container>
        <TopBar />
        <div className="mx-8">
            <Breadcrumbs
                headerIcon={faUser}
                locations={[
                    "Account",
                    account.id
                ]}
            />
        </div>
        <div className="ml-6 mr-8">
            <Folder
                tabs={[
                    [
                        <FontAwesomeIcon
                            icon={faLocationDot}
                            color="white"
                        />,
                        <>
                            <KeyValue
                                {...keyValueProps}
                                rows={[
                                    [
                                        <p>ID:</p>,
                                        <p >{account.id}</p>
                                    ],
                                    [
                                        <p>Date:</p>,
                                        <p >{account.date}</p>
                                    ]
                                ]}
                            />
                            <KeyValue
                                {...keyValueProps}
                                className={cn(keyValueProps.className, "mt-4")}
                                rows={[
                                    ...("originalUri" in account.webAppActor
                                        ? [
                                            [
                                                <p>Domain:</p>,
                                                <p >{account.webAppActor.originalUri.hostname}</p>
                                            ] as const,
                                            [
                                                <p>Scheme:</p>,
                                                <p >{account.webAppActor.originalUri.scheme}</p>
                                            ] as const,
                                        ]
                                        : [
                                            [
                                                <p>App Ed25519:</p>,
                                                <p >{account.webAppActor.ed25519Key}</p>
                                            ] as const,
                                        ]
                                    ),
                                    [
                                        <p>Category:</p>,
                                        <p>{account.metadata.category}</p>
                                    ],
                                    [
                                        <p>Version:</p>,
                                        <p>{account.metadata.version}</p>
                                    ],
                                    [
                                        <p>Status:</p>,
                                        <p>{account.metadata.status}</p>
                                    ],
                                    [
                                        <p>Sign up URI:</p>,
                                        <p>
                                            {account.registrationUri.uri}
                                        </p>
                                    ],
                                    [
                                        <p>Change Password URI:</p>,
                                        <p>{account.metadata.changePassUrl || "N/A"}</p>
                                    ],
                                ]}
                            />
                        </>
                    ],
                    [
                        <FontAwesomeIcon
                            icon={faUser}
                            color="white"
                        />,
                        <KeyValue
                            {...keyValueProps}
                            rows={
                                (Object.keys(account.userAttributes) as Array<keyof typeof account.userAttributes>)
                                    .filter(key => !!account.userAttributes[key])
                                    .map(key => (
                                        [
                                            <p>{userAttributesLabels[key]}</p>,
                                            <p>{stringifyUserAttributeValue(account.userAttributes[key]!)}</p>
                                        ] as const
                                    ))
                            }
                        />
                    ],
                    [
                        <FontAwesomeIcon
                            icon={faKey}
                            color="white"
                        />,
                        <>
                            <SelectContent
                                position={fieldPos}
                                setPosition={setFieldPos}
                                items={Object.values(account.fields).map((field, i) => ({
                                    selectText: `#${field.id}`,
                                    content: (
                                        <KeyValue
                                            {...keyValueProps}
                                            rows={
                                                [
                                                    ...(Object.keys(field.options) as Array<keyof typeof field.options>).map(opts => ([
                                                        <p>{fieldOptionsLabels[opts]}</p>,
                                                        <p className="mt-0.5"> {/* mt-0.5 to vertically align with FieldIdValue */}
                                                            {fieldOptionValueToString(opts, field.options[opts])}
                                                        </p>
                                                    ]) as const),
                                                    [
                                                        <p>Value:</p>,
                                                        <BagedFieldValue className="whitespace-pre-wrap">{field.type !== EBerytusFieldType.Key
                                                            ? field.value
                                                            : formatBase64AsPem(field.value, false) /* we always store the private key, regardless whether it is shared (private === true) or not */
                                                            }</BagedFieldValue>
                                                    ]
                                                ]
                                            }
                                        />
                                    )
                                }))}
                            />
                        </>
                    ]
                ]}
            />
            <div className="flex flex-row mt-2">
                <div className="flex-1"/>
                <Button
                    color="red"
                    text="Delete Account"
                    disabled={deleting}
                    onClick={() => {
                        onDelete();
                    }}
                />
            </div>
        </div>
    </Container>
}