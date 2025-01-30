import { ReactNode } from "react";
import { cn } from "../utils/joinClassName";
import { BgColor } from "@root/utils";

export interface FolderTabProps {
    selectedTab: number;
    tabs: Array<ReactNode>;
    onSelect(index: number): void;
    defaultColor?: BgColor;
    selectedColor?: BgColor;
    borderRightColor?: `border-${string}`;
    tabWidth?: string;
    tabHeight?: string;
}

export default function FolderTab({
    selectedTab,
    tabs,
    onSelect,
    defaultColor = "bg-gray-700",
    selectedColor = "bg-violet-600",
    tabHeight = "h-10",
    tabWidth = "min-w-[3.5rem]", // or try, w-20
    borderRightColor = "border-slate-400",
}: FolderTabProps) {
    return (
        <div className={cn("flex flex-col gap-0.5 text-xs")}>
            {tabs.map((tab, i) => (
                <div
                    key={i}
                    className={cn(
                        "cursor-pointer",
                        tabWidth,
                        i === 0 ? "rounded-bl-3xl" : "rounded-l-xl",
                        tabHeight,
                        i !== selectedTab ?
                            `${defaultColor} border-r-2 ${borderRightColor}` :
                            `${selectedColor}`
                    )}
                    onClick={() => onSelect(i)}
                >
                    <div className="flex justify-center h-full items-center">
                        {tab}
                    </div>
                </div>
            ))}
        </div>
    );
}