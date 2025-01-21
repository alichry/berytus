import { useState } from "react";
import { Container } from "./Container";
import Button from "./Button";
import TopBar from "../controlled/TopBar";
import { faUserPlus } from "@fortawesome/free-solid-svg-icons";
import { formatBase64AsPem } from "@root/key-utils";
import CommonBodyContainer from "./CommonBodyContainer";
import Breadcrumbs from "./Breadcrumbs";
import FieldOptionsKeyValue from "./FieldOptionsKeyValue";
import TextArea from "./TextArea";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faKey } from "@fortawesome/free-solid-svg-icons";
import FieldInput from "./FieldInput";
import Notice from "./Notice";
import type { FieldInfo, UriParams } from "@berytus/types";
import { EBerytusFieldType } from "@berytus/enums";

export interface PutFieldViewProps {
    uri: UriParams;
    field: FieldInfo;
    value: string;
    onCancel(): void;
    onSubmit(): void;
    error?: Error;
}

export default function PutFieldView(props: PutFieldViewProps) {
    const [value, setValue] = useState<string | null>(null);
    return (
        <Container>
            <TopBar onCancel={props.onCancel} />
            <CommonBodyContainer>
                <Breadcrumbs
                    headerIcon={faUserPlus}
                    locations={[
                        "Registration",
                        "Save field"
                    ]}
                    noMarginBottom
                    className="mb-5"
                />
                { props.error  ? (
                    <Notice text={`Error: ${props.error.message}`} type="error" />
                ) : null}
                <FieldOptionsKeyValue
                    hostname={props.uri.hostname}
                    field={props.field}
                    className="mb-1 pb-4 border-b-[0.5px] border-[#dfdfdf]"
                />

                {props.field.type === EBerytusFieldType.Key ?
                    <div>
                        <div className="flex flex-row items-center text-sm">
                            <div className=" pr-4 h-full">
                                <FontAwesomeIcon
                                    icon={faKey}
                                    color="white"
                                />
                            </div>
                            <TextArea
                                defaultValue={formatBase64AsPem(props.value, false)}
                                disabled
                            />
                        </div>

                    </div> :
                    <div className="flex flex-col">
                        <div className="flex flex-row gap-1">
                            <FieldInput
                                disabled
                                className="w-full"
                                value={props.value}
                                field={props.field}
                            />
                        </div>
                    </div>
                }
                <Button
                    text="Save"
                    className="w-full mb-4"
                    onClick={() => {
                        props.onSubmit();
                    }}
                />
            </CommonBodyContainer>
        </Container>
    )
}