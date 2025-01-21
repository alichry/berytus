import { FontAwesomeIcon } from "@fortawesome/react-fontawesome"
import { cn } from "../utils/joinClassName"
import { ClassNameProp } from "../props/ClassNameProp"
import { IconProp } from "@fortawesome/fontawesome-svg-core"

export interface IconAndTitleProps extends ClassNameProp {
    icon: IconProp;
    title: string;
    centered?: boolean
}
export default function IconAndTitle({ centered = true, className, icon, title }: IconAndTitleProps) {
    return (
        <div className={className}>
            <FontAwesomeIcon icon={icon}
                className={cn(
                    "block mb-3 text-default",
                    centered && "mx-auto"
                )}
                size="3x"
            />
            <p
                className={cn(
                    "font-medium !text-default",
                    centered && "text-center"
                )}
            >
                {title}
            </p>
        </div>
    )
}