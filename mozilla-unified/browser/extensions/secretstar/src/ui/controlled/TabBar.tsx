import { useState } from "react";
import TabBarView, { TabBarViewProps } from "../components/TabBarView";

export interface TabBarProps extends Omit<TabBarViewProps, 'selectedTab' | 'setSelectedTab'> {}

export default function TabBar(props: TabBarProps) {
    const [selected, setSelected] = useState<number>(0);
    return (
        <TabBarView
            {...props}
            selectedTab={selected}
            setSelectedTab={(i) => setSelected(i)}
        />
    )
}