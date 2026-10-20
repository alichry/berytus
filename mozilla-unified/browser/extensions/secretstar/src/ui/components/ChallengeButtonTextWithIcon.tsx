import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { ClassNameProp } from "../props/ClassNameProp";
import TextWithIcon from "./TextWithIcon";
import { IconProp } from "@fortawesome/fontawesome-svg-core";

export interface ChallengeButtonTextWithIconProps extends ClassNameProp {
    icon: IconProp;
    text: string;
}

export type { ChallengeButtonTextWithIconProps as ChallengeButtonTextWithIcon };

export default function ChallengeButtonTextWithIcon({ icon, text }: ChallengeButtonTextWithIconProps) {
    return (
        <TextWithIcon
            icon={
                <FontAwesomeIcon icon={icon} />
            }
            gap="mr-3"
            centered
            text={text}
            textSize="text-sm"
        />
    );
}