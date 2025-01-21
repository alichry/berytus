import { faKey } from "@fortawesome/free-solid-svg-icons";
import TopBar from "../controlled/TopBar";
import Breadcrumbs from "./Breadcrumbs";
import { Container } from "./Container";
import ChallengeCard from "./ChallengeCard";
import ChallengePicture from "./ChallengePicture";
import SmallMessage from "./SmallMessage";
import { PropsWithChildren } from "react";
import { EBerytusChallengeType } from "@berytus/enums";

export interface RespondToMessageViewProps extends PropsWithChildren {
    challengeType: typeof EBerytusChallengeType[keyof typeof EBerytusChallengeType];
    onCancel?(): void;
}

export default function RespondToMessageView({ children, onCancel, challengeType }: RespondToMessageViewProps) {
    return (
        <Container>
            <TopBar onCancel={onCancel} />
            <div className="mx-8">
                <Breadcrumbs
                    headerIcon={faKey}
                    locations={[
                        "Authentication",
                        "Challenge Message"
                    ]}
                />
            </div>
            <div className="mx-6">
                <SmallMessage className="mb-5">
                    We have received a new challenge message!
                </SmallMessage>
                <ChallengeCard borderStyle="sides">
                    <ChallengePicture
                        className="mb-4"
                        challengeType={challengeType}
                    />
                    {children}
                </ChallengeCard>
            </div>
        </Container>
    )
}