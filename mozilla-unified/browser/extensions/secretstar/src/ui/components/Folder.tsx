import { useState } from "react";
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