import { PropsWithChildren } from "react";
import { ClassNameProp } from "../props/ClassNameProp";
import { cn } from "../utils/joinClassName";

export interface SmallMessageProps extends ClassNameProp,
    PropsWithChildren {}

export default function SmallMessage({
    children: text,
    className
}: SmallMessageProps) {
    return <p className={cn("text-sm", className)}>{text}</p>
}