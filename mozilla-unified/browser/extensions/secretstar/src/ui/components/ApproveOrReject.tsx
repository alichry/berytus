import { ReactNode } from "react";
import { ClassNameProp } from "../props/ClassNameProp";
import { cn } from "../utils/joinClassName";
import Button, { ButtonProps } from "./Button";

export interface ApproveOrRejectProps extends ClassNameProp {
    approveText?: ReactNode;
    approveProps?: Omit<ButtonProps, 'onClick' | 'text'>;
    rejectText?: ReactNode;
    rejectProps?: Omit<ButtonProps, 'onClick' | 'text'>;
    onApprove?(): void;
    onReject?(): void;
    middleClassName?: string;
    order?: "rejectFirst" | "approveFirst";
}

export default function ApproveOrReject({
    className,
    onReject,
    onApprove,
    approveProps,
    rejectProps,
    approveText,
    rejectText,
    middleClassName,
    order = "approveFirst"
}: ApproveOrRejectProps) {
    if (
        (rejectText === undefined && onReject !== undefined) ||
        (rejectText !== undefined && onReject === undefined)
    ) {
        throw new Error('ApproveOrReject: invalid rejectText/onReject');
    }
    if (
        (approveText === undefined && onApprove !== undefined) ||
        (approveText !== undefined && onApprove === undefined)
    ) {
        throw new Error('ApproveOrReject: invalid rejectText/onReject');
    }

    const approve = approveText && onApprove ? (
        <Button
            text={approveText}
            color="blue"
            onClick={() => onApprove()}
            {...approveProps}
        />
    ) : null;
    const reject = rejectText && onReject ? (
        <Button
            color="red"
            text={rejectText}
            onClick={() => onReject()}
            {...rejectProps}
        />
    ) : null;
    return (
        <div className={cn("flex flex-row", className)}>
            {order === "approveFirst" ? <>
                {approve}
                <div className={cn("flex-1", middleClassName)}></div>
                {reject}
            </> : <>
                {reject}
                <div className={cn("flex-1", middleClassName)}></div>
                {approve}
            </>}
        </div>
    )
}