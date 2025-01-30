import { Container } from "./Container";
import Button from "./Button";
import TopBar from "../controlled/TopBar";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faShuffle, faUserPlus } from "@fortawesome/free-solid-svg-icons";
import { FieldValueRejection } from "@root/db/db";
import CommonBodyContainer from "./CommonBodyContainer";
import Breadcrumbs from "./Breadcrumbs";
import FieldOptionsKeyValue from "./FieldOptionsKeyValue";
import TextArea from "./TextArea";
import FieldInput from "./FieldInput";
import Notice from "./Notice";
import { cn } from "../utils/joinClassName";
import { useRef } from "react";
import type { DocumentMetadata, FieldInfo } from "@berytus/types";
import { EBerytusFieldType } from "@berytus/enums";

export interface CreateFieldViewProps<T> {
    page: DocumentMetadata;
    field: FieldInfo;
    onCancel(): void;
    onSubmit(): void;
    value: T;
    setValue(value: string): void;
    isEmpty: boolean;
    generateAndSetValue? (): void;
    isGenerating?: boolean;
    toString(): string;
    error?: Error;
    previouslyRejected?: FieldValueRejection
}

export default function CreateFieldView<T>({ generateAndSetValue, ...props }: CreateFieldViewProps<T>) {
    const dictated = props.previouslyRejected?.webAppDictatedValue !== undefined;
    const inputRef = useRef<HTMLInputElement & HTMLTextAreaElement>(null);
    return (
        <Container>
            <TopBar onCancel={props.onCancel} />
            <CommonBodyContainer>
                <Breadcrumbs
                    headerIcon={faUserPlus}
                    locations={[
                        "Registration",
                        "Create field"
                    ]}
                    noMarginBottom
                    className="mb-5"
                />
                { props.previouslyRejected  ? (
                    <Notice text={`The Web Application has rejected the last value you have entered! Reason: ${props.previouslyRejected.code}.`} type="error" />
                ) : null}
                <FieldOptionsKeyValue
                    hostname={props.page.uri.hostname}
                    field={props.field}
                    className="mb-2"
                />
                {props.field.type === EBerytusFieldType.Key ?
                    <>
                        <Button
                            text={
                                props.isGenerating ? "..." :
                                <FontAwesomeIcon
                                    icon={faShuffle}
                                />
                            }
                            disabled={props.isGenerating}
                            className="float-right mb-2"
                            color="blue"
                            onClick={() => { generateAndSetValue!() }}
                        />
                        <TextArea
                            ref={inputRef}
                            value={props.toString()}
                            onChange={(e) => props.setValue(e.target.value)}
                            className="mb-2"
                            disabled={props.isGenerating}
                            // BRTTODO: Pasting does not work since we validate on each input character change, and it throws an error before the whole PEM str is provided.
                            placeholder={`Paste a private key or click on the blue button to generate one. The web application will receive the public key only.`}
                        />
                    </> :
                    <div className="mt-6 mb-2 flex flex-col">
                        <div className="flex flex-row gap-1">
                            <FieldInput
                                ref={inputRef}
                                placeholder={''}
                                onChange={e => {
                                    props.setValue(e.target.value);
                                }}
                                value={props.toString()}
                                field={props.field}
                                className={cn("w-full")}
                                noMargin={true}
                                disabled={dictated}
                            />
                            {   generateAndSetValue ?
                                    <FontAwesomeIcon
                                        icon={faShuffle}
                                        color="white"
                                        className="pl-2 cursor-pointer self-center"
                                        onClick={() => generateAndSetValue()}
                                    /> : null
                            }
                        </div>
                        {dictated
                        ? <p className="pt-1 text-left italic text-sm">
                            The web app suggested the value above, click save to confirm.
                        </p>
                        : null}
                    </div>
                }
                <div className="pb-2">
                    { props.error ? (
                        <p className="text-sm text-red-400 mb-2">Error: {props.error.message}</p>
                    ) : null}
                </div>

                <Button
                    text="Save"
                    className="w-full mb-4"
                    disabled={props.isEmpty}
                    onClick={() => {
                        inputRef.current?.setAttribute('dirty', '');
                        if (! inputRef.current?.validity.valid) {
                            /* Report validity does not work in popup mode */
                            inputRef.current?.reportValidity();
                            return;
                        }
                        props.onSubmit();
                    }}
                />
            </CommonBodyContainer>
        </Container>
    )
}