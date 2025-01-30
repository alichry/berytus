import { ClassNameProp } from "../props/ClassNameProp";
import LabeledInput from "./LabeledInput";

export interface LabeledInputItem {
    id: string;
    defaultValue?: string;
    /* defaults to true */
    required?: boolean;
    label: string;
    type: string;
}
export interface LabeledInputListProps extends ClassNameProp {
    items: Array<LabeledInputItem>;
    onChange(itemId: string, newValue: string): void;
}

export default function LabeledInputList({
    items,
    className,
    onChange
}: LabeledInputListProps) {
    return (
        <div className={className}>
            {items.map((item, i) => (
                <LabeledInput
                    key={item.id}
                    label={item.label}
                    type={item.type}
                    defaultValue={item.defaultValue}
                    onChange={(e) => onChange(item.id, e.target.value)}
                    className={i !== items.length - 1 ? "mb-2" : undefined}
                    required={item.required === undefined ? true : item.required}
                />
            ))}
        </div>
    )
}