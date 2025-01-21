import { ClassNameProp } from "@props/ClassNameProp";
import styles from '@styles/TopBar.module.scss';
import { Logo } from "./Logo";
import Button from "./Button";
import { cn } from "@root/ui/utils/joinClassName";
import type { useNavigate } from "react-router-dom";
import { url } from "@root/workers/paths";
import { PAGECONTEXT_WINDOW } from "@root/pagecontext";

export interface TopBarViewProps extends ClassNameProp {
    onCancel?: () => void;
    navigate?: ReturnType<typeof useNavigate>;
}

export default function TopBarView({ className, onCancel, navigate }: TopBarViewProps) {
    return (
        <div
            onClick={(e) => {
                if (e.ctrlKey || e.metaKey) {
                    window.open(url('/', PAGECONTEXT_WINDOW), '_blank');
                    return;
                }
                navigate && navigate('/')
            }}
            className={cn(styles.container, "cursor-pointer", className) }
        >
            <Logo className="flex-1" />
            { onCancel ? (
                <Button
                    color="red"
                    text="Cancel"
                    className="self-center mr-4"
                    onClick={() => onCancel()}
                />
            ) : null}
        </div>
    )
}