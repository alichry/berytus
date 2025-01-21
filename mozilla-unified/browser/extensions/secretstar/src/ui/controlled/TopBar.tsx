import TopBarView, { TopBarViewProps } from "../components/TopBarView";
import { useNavigateWithPageContextRoute } from "@root/hooks";

export interface TopBarProps extends Omit<TopBarViewProps, 'onClick'> {}

export default function TopBar(props: TopBarProps) {
    const navigate = useNavigateWithPageContextRoute();
    return (
        <TopBarView {...props} navigate={navigate} />
    )
}