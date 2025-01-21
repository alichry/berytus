import { useValueOverrides } from "@root/hooks";
import LabeledInputListForm, { LabeledInputListFormProps } from "./LabeledInputListForm";
import { LabeledInputItem } from "@root/ui/components/LabeledInputList";
import { useEffect, useState } from "react";
import MdCenteredSpinner from "@root/ui/components/MdCenteredSpinner";
import ChallengeButtonTextWithIcon from "@root/ui/components/ChallengeButtonTextWithIcon";
import { faClockRotateLeft } from "@fortawesome/free-solid-svg-icons";

export interface EditableLabeledInputListFormProps
    extends Omit<LabeledInputListFormProps, 'onChange' | 'onApprove' | 'items'> {
    items: Array<Omit<LabeledInputItem, 'defaultValue'> & { defaultValue: string }>
    onApprove(desiredValues: Record<string, string>): void;
    noReset?: boolean;
}

export default function EditableLabeledInputListForm({ onApprove, noReset = false, items, ...otherProps }: EditableLabeledInputListFormProps) {
    const { override, getDesiredValues } = useValueOverrides();
    const [showReset, setShowReset] = useState<boolean>();
    const [reset, setReset] = useState<boolean>();
    useEffect(() => {
        const int = setTimeout(() => {
            setReset(undefined);
            //setShowReset(false);
        }, 250);
        return () => clearTimeout(int);
    }, [reset]);
    if (reset) {
        return <MdCenteredSpinner />;
    }
    return <LabeledInputListForm
        items={items}
        onChange={(itemId, newValue) => {
            override(itemId, newValue);
            if (showReset !== true) {
                setShowReset(true);
            }
        }}
        onReject={! noReset && showReset ? () => {
            setReset(true);
        } : undefined}
        rejectText={
            ! noReset && showReset ? <ChallengeButtonTextWithIcon
                text="Reset"
                icon={faClockRotateLeft}
            /> : undefined
        }
        rejectProps={{
            color: "teal"
        }}
        onApprove={() => {
            if (! items) {
                throw new Error('onApprove called with items being undefined!');
            }
            const defaultValues: Record<string, string> = {};
            items.forEach(t => {
                defaultValues[t.id] = t.defaultValue;
            });
            onApprove(getDesiredValues(defaultValues));
        }}
        {...otherProps}
    />
}