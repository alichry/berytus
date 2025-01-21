import { PropsWithChildren } from "react";
import { cn } from "../utils/joinClassName";

export interface ChallengeCardProps extends PropsWithChildren {
    borderStyle?: "top" | "left" | "sides";
}

export default function ChallengeCard({
    children,
    borderStyle = "top"
}: ChallengeCardProps) {
    return (
        <div
            className={cn(
                "pt-8 pb-6 px-4 bg-gray-700 rounded-b-md",
                "border-dashed",
                borderStyle === "top" && "border-t-2",
                borderStyle === "left" && "border-l-2",
                borderStyle === "sides" && "border-x-2",
            )}
        >
            {children}
        </div>
    )
}