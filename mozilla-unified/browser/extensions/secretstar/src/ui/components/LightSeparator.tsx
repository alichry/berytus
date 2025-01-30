import { ClassNameProp } from "../props/ClassNameProp";
import { cn } from "../utils/joinClassName";

export interface LightSeparatorProps extends ClassNameProp {}

export default function LightSeparator({ className }: LightSeparatorProps) {
    return <div
        className={cn(
            "mt-2 mb-5 border-b-[0.5px] border-gray-200",
            className
        )}/>
}