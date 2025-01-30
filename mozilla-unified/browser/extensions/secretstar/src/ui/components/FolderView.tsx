import { Fragment, useState } from "react";
import FolderTab from "./FolderTab";
import FolderPage from "./FolderPage";
import { ReactNode } from 'react';

export interface FolderViewProps {
    selectedTab: number;
    setSelectedTab: (tabIndex: number) => void;
    tabs: Array<[title: ReactNode, content: ReactNode]>;
}

export default function FolderView({ tabs, selectedTab, setSelectedTab }: FolderViewProps) {
    return <div className="flex flex-row gap-0">
        <FolderTab
            selectedTab={selectedTab}
            tabs={tabs.map(([title], i) => <Fragment key={i}>{title}</Fragment>)}
            onSelect={(i) => setSelectedTab(i)}
        />
        <FolderPage className="w-32 min-h-[8rem] px-4 py-4">
            {tabs[selectedTab][1]}
        </FolderPage>
    </div>
}