import { IconProp } from "@fortawesome/fontawesome-svg-core";
import { ClassNameProp } from "../props/ClassNameProp";
import { cn } from "../utils/joinClassName";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faCircleInfo, faCircleXmark } from "@fortawesome/free-solid-svg-icons";

export interface NoticeProps extends ClassNameProp {
    type?: "error" | "info";
    text: string;
    color?: "red" | "green";
}

const iconColors = {
    "info": "text-teal-300",
    "error": "text-red-400"
}
const icons = {
    "info": faCircleInfo,
    "error" : faCircleXmark
}

export default function Notice({ text, className, type = "info" }: NoticeProps) {
    return (
        <div className={cn("text-sm flex px-3 py-3 rounded bg-gray-600 mb-4 flex-row gap-3 items-center font-bold", className)}>
            <FontAwesomeIcon
                icon={icons[type]}
                size="lg"
                color="white"
                className={cn(iconColors[type])}
            />
            <p>{text}</p>
        </div>
    )
}