import { ReactNode } from "react";
import { ClassNameProp } from "../props/ClassNameProp";
import { cn } from "../utils/joinClassName";

export interface TextWithIconProps extends ClassNameProp {
    icon: ReactNode;
    text: string;
    textSize?: string;
    centered?: boolean;
    gap?: `mr-${number}`
}

export default function TextWithIcon({ icon,
    gap = "mr-4",
    text,
    textSize = "text-base",
    centered = false,
    className
}: TextWithIconProps) {
    return (
        <div className={cn("flex flex-row", centered && "justify-center items-center", className)}>
            <div className={`${gap} ${textSize}`}>
                {icon}
            </div>
            <p className={`block ${textSize}`}>{text}</p>
        </div>
    );
}