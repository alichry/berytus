import { Container } from "@root/ui/components/Container";
import TopBar from "../TopBar";
import Notice from "@root/ui/components/Notice";

export interface UnknownChallengeMessageProps {
    challengeType: string;
    messageName: string;
}

export default function UnknownChallengeMessage({ challengeType, messageName }: UnknownChallengeMessageProps) {
    return (
        <Container>
            <TopBar />
            <Notice
                className="mt-2"
                type="error"
                text={`Unknwon challenge message. Type: ${challengeType} Name: ${messageName}`}
            />
        </Container>
    )
}