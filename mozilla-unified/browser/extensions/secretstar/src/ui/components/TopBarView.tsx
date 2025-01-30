import { ClassNameProp } from "@props/ClassNameProp";
import styles from '@styles/TopBar.module.scss';
import { Logo } from "./Logo";
import Button from "./Button";
import { cn } from "@root/ui/utils/joinClassName";
import type { useNavigate } from "react-router-dom";
import { url } from "@root/workers/paths";
import { PAGECONTEXT_WINDOW } from "@root/pagecontext";
import { PendingRequest } from "@root/db";

export interface TopBarViewProps extends ClassNameProp {
    onCancel?: () => void;
    navigate?: ReturnType<typeof useNavigate>;
    pendingRequest?: PendingRequest;
}

export default function TopBarView({ className, onCancel, navigate, pendingRequest }: TopBarViewProps) {
    return (
        <>
            {pendingRequest ? (
                <div className="bg-orange-800 text-sm text-white px-2 py-0.5 text-center">
                    Click <a href="#" className="underline clickable">here</a> to resolve pending request.
                </div>
            ): null}
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
        </>
    )
}