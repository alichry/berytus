import { Account } from "@root/db";
import { Container } from './Container';
import CommonBodyContainer from './CommonBodyContainer';
import TopBar from '../controlled/TopBar';
import Breadcrumbs from "./Breadcrumbs";
import { faBan, faPlay, faUser, faUserPlus } from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import ApproveOrReject from "./ApproveOrReject";
import TextWithIcon from "./TextWithIcon";
import { AccountIdCardView } from "./AccountIdCardView";

interface TransitionToAuthViewProps {
    selectedAccount: Account;
    onConfirm(): void;
    onDeny(): void;
    onCancel(): void;
}

export default function TransitionToAuthView({
    selectedAccount,
    onCancel,
    onDeny,
    onConfirm
}: TransitionToAuthViewProps) {
    return (
        <Container>
            <TopBar onCancel={() => onCancel()} />
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
                    After successfully registering your account,
                    the web application would like to initiate
                    an authenntication operation for the newly
                    registered account, would you like to proceed?
                </p>
                <AccountIdCardView
                    account={selectedAccount}
                    className="mb-4"
                />
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
                    onApprove={() => onConfirm()}
                    onReject={() => onDeny()}
                />
            </CommonBodyContainer>
        </Container>
    )
}