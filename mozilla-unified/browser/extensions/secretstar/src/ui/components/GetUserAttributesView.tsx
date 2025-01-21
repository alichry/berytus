import { Container } from "./Container";
import { ReactNode, RefObject, createRef, useState } from "react";
import { cn } from "@root/ui/utils/joinClassName";
import TopBar from "../controlled/TopBar";
import CondensedTable, { ValueColumn, ValueTypeColumn } from "./CondensedTable";
import Button from "./Button";
import CommonBodyContainer from "./CommonBodyContainer";
import Breadcrumbs from "./Breadcrumbs";
import { faEdit, faSave, faUserPlus } from "@fortawesome/free-solid-svg-icons";
import { Identity, UserAttributes } from "@root/db";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import BorderlessInput from "./BorderlessInput";
import { userAttributesLabels } from "../utils/userAttributesLabels";
import Notice from "./Notice";
import UploadImage from "../controlled/UploadImage";
import { scrollTopSmooth } from "@root/utils";
import PictureInput from "../controlled/PictureInput";
import { RRequiredUserAttributes, RUserAttributes } from "@root/db/db";
import type { UserAttributeKey } from "@berytus/types";
import { stringifyUserAttributeValue } from "./AccountView";

export interface GetUserAttributesViewProps {
    requiredAttributes: RRequiredUserAttributes;
    userAttributes: Identity['userAttributes'];
    onCancel(): void;
    onApprove(selectedUserAttributes: Array<UserAttributeKey>, overrides: Partial<RUserAttributes>): void;
    onReject(): void;
    error?: Error;
}

function Checkbox({ checked, toggleChecked }: { checked: boolean; toggleChecked?: () => void }) {
    return <input type="checkbox"
        checked={checked}
        disabled={!toggleChecked}
        onChange={() => {
            if (toggleChecked) {
                toggleChecked();
            }
        }}
    />
}

export default function GetUserAttributesView(props: GetUserAttributesViewProps) {
    const [openCode, setOpenCode] = useState<boolean>(false);
    const [check, setCheck] = useState<Partial<Record<UserAttributeKey, boolean>>>(
        {}
    );
    const [refs] = useState(() => {
        return (Object.keys(props.requiredAttributes) as Array<keyof typeof props.requiredAttributes>)
            .reduce((prev, curr) => {
                prev[curr] = createRef<HTMLInputElement & HTMLTextAreaElement>();
                return prev;
            }, {} as Record<UserAttributeKey, RefObject<HTMLInputElement & HTMLTextAreaElement>>);
    });
    const [uploadedPictureId, setUploadedPictureId] = useState<string>();

    const rows: Array<[ReactNode]> = [];
    (Object.keys(props.requiredAttributes) as Array<keyof typeof props.requiredAttributes>).forEach(name => {
        const required = props.requiredAttributes[name];
        const value = props.userAttributes && props.userAttributes[name]
            ? stringifyUserAttributeValue(props.userAttributes[name]!)
            : "";
        const isEmpty = value === undefined || value.length === 0
        if (! required && isEmpty) {
            // skip empty strings that are not required.
            return;
        }

        const checkVal = check[name];
        const isChecked = checkVal === undefined ? true : checkVal;
        // TODO: use CondensedRowWithHint instead
        rows.push([
            <div className="flex flex-row gap-2">
                <Checkbox
                    checked={isChecked}
                    toggleChecked={
                        ! props.requiredAttributes[name] ?
                            () => setCheck({ ...check, [name]: !isChecked }) :
                            undefined
                    }
                />
                { name === "picture" ? (
                    <PictureInput
                        className="flex-1"
                        onUpload={async (pictureId) => {
                            setUploadedPictureId(pictureId);
                        }}
                        onCancel={async () => {
                            setUploadedPictureId(undefined);
                        }}
                        placeholder="Upload an avatar"
                        defaultPictureId={value}
                        placeholderClassName={isEmpty && required ? "text-amber-800 animate-pulse " : undefined}
                    />
                    // <UploadImage
                    //     className={cn("flex-1")}
                    //     selectFileTextClassName={isEmpty && required ? "text-amber-700 animate-pulse" : undefined}
                    // />
                ) : (
                    <BorderlessInput
                    type={
                        name === "address"
                            ? "textarea"
                            : name === "birthdate"
                                ? "date"
                            : "text"
                    }
                    defaultValue={value}
                    ref={refs[name]}
                    className={"flex-1"}
                    inputClassName={cn(
                        isEmpty && required &&
                            'placeholder:italic placeholder:text-amber-600 placeholder-shown:animate-pulse',
                    )}
                    spellCheck={false}
                    required={required} // this is useless since we do not set dirty="" when the input has changed
                    placeholder={required && isEmpty ? "Empty" : undefined}
                    onBlur={(e) => {
                        if (e.target.value !== "") {
                            return;
                        }
                        if (required) {
                            e.target.value = value || "";
                        }
                    }}
                />
                )}
                <ValueTypeColumn
                    className="whitespace-nowrap"
                    text={userAttributesLabels[name].toLocaleLowerCase()}
                    textSize="text-sm"
                />
            </div>
        ])
        //rows.push([name, value]);
    })
    return (
        <Container>
            <TopBar onCancel={props.onCancel} />
            <CommonBodyContainer className="text-sm">
                <Breadcrumbs
                    headerIcon={faUserPlus}
                    locations={[
                        "Registration",
                        "Identity info"
                    ]}
                    noMarginBottom
                    className="mb-5"
                />
                { props.error  ? (
                    <Notice text={props.error.message} type="error" />
                ) : null}
                <p className="text-base mb-3.5 text-justify">Do you want to share your identity information?</p>
                <CondensedTable
                    className="mb-8 mx-auto w-full"
                    rows={rows}
                />
                {/* BRTTODO: Use ApproveOrReject */}
                <div className="flex flex-row gap-8 mb-8">
                    <Button
                        text="Approve"
                        className="flex-1"
                        onClick={() => {
                            const overrides: Partial<RUserAttributes> =
                                (Object.keys(refs) as Array<keyof typeof refs>)
                                    .reduce((prev, curr) => {
                                        if (refs[curr].current === null) {
                                            return prev;
                                        }
                                        const value = refs[curr].current!.value;
                                        if (
                                            props.userAttributes &&
                                            props.userAttributes[curr] &&
                                            value === props.userAttributes[curr]!.value
                                        ) {
                                            return prev;
                                        }
                                        // TODO(berytus): userAttributes should not be optional
                                        prev[curr] = {
                                            id: curr,
                                            mimeType: props.userAttributes ?
                                                props.userAttributes[curr]?.mimeType
                                                : undefined,
                                            value
                                        };
                                        return prev;
                                    }, {} as Partial<RUserAttributes>);
                            if (uploadedPictureId) {
                                overrides.picture = {
                                    id: "picture",
                                    // TODO(berytus): Check
                                    mimeType: "text/plain",
                                    value: uploadedPictureId
                                };
                            }
                            props.onApprove(
                                (Object.keys(userAttributesLabels) as Array<keyof typeof userAttributesLabels>)
                                    .filter(k => check[k] === undefined || check[k]),
                                overrides
                            );
                            scrollTopSmooth();
                        }}
                    />
                    <Button
                        color="red"
                        text="Reject"
                        className="flex-1"
                        onClick={() => props.onReject()}
                    />
                </div>
            </CommonBodyContainer>
        </Container>
    )
}