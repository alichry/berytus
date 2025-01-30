import { PropsWithChildren } from "react";
import { ClassNameProp } from "../props/ClassNameProp";
import { cn } from "../utils/joinClassName";

export default function CommonBodyContainer({ children, className }: PropsWithChildren<ClassNameProp>) {
    return (
        <div className={cn("mx-8", className)}>{children}</div>
    )
}