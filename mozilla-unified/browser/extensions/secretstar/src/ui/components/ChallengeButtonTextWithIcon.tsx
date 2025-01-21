import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { ClassNameProp } from "../props/ClassNameProp";
import TextWithIcon from "./TextWithIcon";
import { IconProp } from "@fortawesome/fontawesome-svg-core";

export interface ChallengeButtonTextWithIcon extends ClassNameProp {
    icon: IconProp;
    text: string;
}

export default function ChallengeButtonTextWithIcon({ icon, text }: ChallengeButtonTextWithIcon) {
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