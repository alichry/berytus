import { Challenge, ChallengeMessage, Session, Settings } from "@root/db";

export interface BaseMessageProps {
    session: Session;
    challenge: Challenge;
    message: ChallengeMessage;
    settings: Settings;
}