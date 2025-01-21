import { faChess, faKey, faPersonRunning } from "@fortawesome/free-solid-svg-icons";
import TopBar from "../controlled/TopBar";
import Breadcrumbs from "./Breadcrumbs";
import { Container } from "./Container";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import TextWithIcon from "./TextWithIcon";
import ApproveOrReject from "./ApproveOrReject";
import ChallengePicture from "./ChallengePicture";
import ChallengeCard from "./ChallengeCard";
import ChallengeDescription from "./ChallengeDescription";
import ChallengeParameters from "./ChallengeParameters";
import SmallMessage from "./SmallMessage";
import type { BerytusChallengeInfoUnion } from "@berytus/types";
import { EBerytusChallengeType } from "@berytus/enums";

export interface ApproveChallengeViewProps {
    challenge: BerytusChallengeInfoUnion;
    onApprove(): void;
    onReject(): void;
}

export default function ApproveChallengeView({ challenge, onApprove, onReject }: ApproveChallengeViewProps) {
    return (
        <Container>
            <TopBar />
            <div className="mx-8">
                <Breadcrumbs
                    headerIcon={faKey}
                    locations={[
                        "Authentication",
                        "Challenge Approval"
                    ]}
                    className="!mb-4"
                />
            </div>
            <div className="ml-6 mr-8">
                {/* <Notice
                    text="The web application would like to start a new authentication challenge."
                    /> */}
                <SmallMessage className="mb-4">
                    The web application would like to start a new authentication challenge.
                </SmallMessage>
                <ChallengeCard>
                    <ChallengePicture
                        challengeType={challenge.type}
                        className="mb-5"
                    />
                    { challenge.type === EBerytusChallengeType.Identification ? (
                        <ChallengeDescription
                            className="mb-5"
                            instructions={[
                                "will retrieve identity fields.",
                                "might not reveal field validity."
                            ]}
                        />
                    ) : null}
                    <ChallengeParameters
                        className="mb-5"
                        //parameters={{ "alg": "RSA-OEP", "bb": "OO" }}
                        parameters={(challenge.parameters || {}) as Record<string, unknown>}
                    />
                    <ApproveOrReject
                        approveText={
                            <TextWithIcon
                                icon={
                                    <FontAwesomeIcon icon={faChess} />
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
                                        icon={faPersonRunning}
                                        flip="horizontal"
                                    />
                                }
                                gap="mr-3"
                                centered
                                text="Reject"
                                textSize="text-sm"
                            />
                        }
                        order="rejectFirst"
                        onApprove={() => onApprove()}
                        onReject={() => onReject()}
                    />
                </ChallengeCard>
            </div>
        </Container>
    );
}