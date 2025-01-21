import { PropsWithChildren } from "react";
import { cn } from "../utils/joinClassName";
import { ClassNameProp } from "../props/ClassNameProp";
import { BgColor } from "@root/utils";

export interface FolderPageProps extends PropsWithChildren, ClassNameProp {
    bgColor?: BgColor;
    fullWidth?: boolean;
}

export default function FolderPage({ bgColor = "bg-gray-700", children, className, fullWidth = true }: FolderPageProps) {
    return <div className={
        cn(
            'overflow-auto',
            bgColor,
            className,
            'rounded-r-lg',
            'rounded-bl',
            fullWidth && 'w-full'
        )
    }>{children}</div>
}