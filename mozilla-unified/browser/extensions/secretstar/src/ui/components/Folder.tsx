import { useState } from "react";
import FolderTab from "./FolderTab";
import FolderPage from "./FolderPage";
import { ReactNode } from 'react';
import FolderView from "./FolderView";

export interface FolderProps {
    tabs: Array<[title: ReactNode, content: ReactNode]>;
}

export default function Folder({ tabs }: FolderProps) {
    const [selectedTab, setSelectedTab] = useState<number>(0);
    return <FolderView
        tabs={tabs}
        setSelectedTab={setSelectedTab}
        selectedTab={selectedTab}
    />
}