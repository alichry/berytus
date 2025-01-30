import { ReactNode, PropsWithChildren } from "react";
import { cn } from "../utils/joinClassName";
import { ClassNameProp } from "../props/ClassNameProp";

export interface TabBarViewProps {
    tabs: Array<[ReactNode, ReactNode]>;
    contentClassName?: string;
    selectedTab: number;
    setSelectedTab(index: number): void;
}
interface TabViewProps extends PropsWithChildren {
    selected?: boolean;
    first?: boolean;
    onClick?: () => void;
}
function TabView({ children, selected, onClick, first }: TabViewProps) {
    return (
        <div className={cn(
            "text-white text-sm rounded-tr-2xl px-6 cursor-pointer py-2.5 text-base cursor-pointer",
            !first && "rounded-tl-lg",
            selected ? "bg-indigo-950" : "bg-indigo-900"
        )}
            onClick={onClick}
        >
            {children}
        </div>
    )
}

interface TabContentProps extends ClassNameProp, PropsWithChildren { }
function TabContent({ children, className }: TabContentProps) {
    return (
        <div className={cn("bg-indigo-950 py-6 px-4 text-white", className)}>
            {children}
        </div>
    )
}

export default function TabBarView({ tabs, selectedTab, setSelectedTab, contentClassName }: TabBarViewProps) {
    return (
        <>
            <div className="flex flex-row border-b border-b-gray-400 gap-0.5">
                {tabs.map((tab, i) => (
                    <TabView
                        key={i}
                        selected={selectedTab === i}
                        onClick={() => setSelectedTab(i)}
                        first={i === 0}
                    >
                        {tab[0]}
                    </TabView>
                ))}
            </div>
            <TabContent className={contentClassName}>
                {tabs[selectedTab][1]}
            </TabContent>
        </>

    )
}