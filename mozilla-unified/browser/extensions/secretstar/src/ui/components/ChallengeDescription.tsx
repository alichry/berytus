import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { ClassNameProp } from "../props/ClassNameProp";
import TextWithIcon from "./TextWithIcon";
import { faChevronRight } from "@fortawesome/free-solid-svg-icons";

const Instruction = (text: string, key: number | string) => {
    return (
        <TextWithIcon
            key={key}
            icon={
                <FontAwesomeIcon
                    icon={faChevronRight}
                    //color="white"
                    size="xs"
                    className="text-indigo-100"
                />
            }
            gap="mr-3"
            textSize="text-sm"
            text={text}
        />
    );
}

export interface ChallengeDescriptionProps extends ClassNameProp {
    instructions: Array<string>;
}

export default function ChallengeDescription({ className, instructions }: ChallengeDescriptionProps) {
    return (
        <div className={className}>
            <p className="text-sm mb-3 italic">The web application...</p>
            <div className="flex flex-col gap-2 border-color-gray-200 border-[0.5px] border-dashed p-4">
                {instructions.map((inst, i) => Instruction(inst, i))}
            </div>
        </div>
    )

}