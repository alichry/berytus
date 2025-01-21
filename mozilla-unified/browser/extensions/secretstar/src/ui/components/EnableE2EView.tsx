import { faUserPlus, faUser, faPlay, faBan } from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import TopBar from "../controlled/TopBar";
import ApproveOrReject from "./ApproveOrReject";
import Breadcrumbs from "./Breadcrumbs";
import CommonBodyContainer from "./CommonBodyContainer";
import { Container } from "./Container";
import TextWithIcon from "./TextWithIcon";

export interface EnableE2EViewProps {
    webAppEd25519Key: string;
    webAppX25519Key: string;
    webAppSignatureValid?: boolean;
    webAppSignatureValidationError?: Error;
    webAppSignatureValidationLoading?: boolean;
    scmSignatureError?: Error;
    scmSignatureLoading?: boolean;
    scmSignatureComputed?: boolean;
    onVerifyWebAppSignature(): Promise<void>;
    onComputeScmSignature(): Promise<void>;
    onProceed(): void;
    onCancel(): void;
}

export default function EnableE2EView() {
    return (
        <Container>
            <TopBar onCancel={() => { }} />
            <CommonBodyContainer>
                <Breadcrumbs
                    headerIcon={faUserPlus}
                    locations={[
                        "Registration",
                        "Transition to Auth"
                    ]}
                    noMarginBottom
                    className="mb-5"
                />
                <p className="text-base mb-3.5 text-justify">
                    The Web Application would like to enable
                    end-to-end encryption. There are two steps
                    required to achieve end-to-end encryption.
                    First, we have to verify that the key exchange
                    parameters signature is valid. Second, we have
                    to produce our own key exchange parameters signature.
                </p>
                <div className="bg-blue-900 pt-4 pb-6 px-6 rounded mb-4">
                    <div className="flex flex-row gap-4">
                        <FontAwesomeIcon
                            icon={faUser}
                            color="white"
                            className="pr-0.5 pt-1.5"
                        />
                        <div className="flex flex-col gap-1">
                            {/* {selectedAccount.fields.filter(
                            f => f.options.fieldType === EFieldType.Identity ||
                            f.options.fieldType === EFieldType.ForeignIdentity
                        ).map((field, i) => (
                            <p className={i !== 0 ? "text-sm" : ""}>{field.value}</p>
                        ))
                    } */}
                        </div>
                    </div>
                </div>
                <ApproveOrReject
                    middleClassName="h-0.5 mx-0.5 bg-gray-600 self-center"
                    approveText={
                        <TextWithIcon
                            icon={
                                <FontAwesomeIcon icon={faPlay} />
                            }
                            gap="mr-3"
                            centered
                            text="Start"
                            textSize="text-sm"
                        />
                    }
                    rejectText={
                        <TextWithIcon
                            icon={
                                <FontAwesomeIcon
                                    icon={faBan}
                                    flip="horizontal"
                                />
                            }
                            gap="mr-3"
                            centered
                            text="Reject"
                            textSize="text-sm"
                        />
                    }
                    approveProps={{ color: "purple" }}
                    order="rejectFirst"
                    onApprove={() => { }}
                    onReject={() => { }}
                />
            </CommonBodyContainer>
        </Container>
    );
}