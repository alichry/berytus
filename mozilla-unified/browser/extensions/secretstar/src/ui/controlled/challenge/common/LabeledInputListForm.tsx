import ApproveOrReject, { ApproveOrRejectProps } from "@root/ui/components/ApproveOrReject";
import ChallengeButtonTextWithIcon from "@root/ui/components/ChallengeButtonTextWithIcon";
import { faPaperPlane } from "@fortawesome/free-solid-svg-icons";
import MdCenteredSpinner from "@root/ui/components/MdCenteredSpinner";
import LabeledInputList, { LabeledInputListProps } from "@root/ui/components/LabeledInputList";
import { atLeastOneErrorSet, concatErrorMessages } from "@root/utils";
import Notice from "@root/ui/components/Notice";
import { ReactNode } from "react";

export interface LabeledInputListFormProps
    extends LabeledInputListProps, Omit<ApproveOrRejectProps, 'onApprove'> {
    loading?: boolean;
    errors?: Array<Error | undefined>;
    header?: ReactNode;
    onApprove(): void;
}

export default function LabeledInputListForm({
    loading,
    items = [],
    errors = [],
    onChange,
    className,
    onApprove,
    header,
    ...approveOrRejectProps
}: LabeledInputListFormProps) {
    if (atLeastOneErrorSet(errors)) {
    return <Notice
            className={className}
            text={`Error: ${concatErrorMessages(...errors)}`}
            type="error"
        />;
    }
    if (loading) {
        return <MdCenteredSpinner />;
    }
    return (
        <div className={className}>
            {header}
            <LabeledInputList
                className="mb-5"
                items={items}
                onChange={onChange}
            />
            <ApproveOrReject
                approveText={
                    <ChallengeButtonTextWithIcon
                        text="Send"
                        icon={faPaperPlane}
                    />
                }
                order="rejectFirst"
                onApprove={onApprove}
                {...approveOrRejectProps}
            />
        </div>
    )
}