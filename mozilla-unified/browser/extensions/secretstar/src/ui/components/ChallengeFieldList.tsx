import { Field } from "@root/db";
import { ClassNameProp } from "../props/ClassNameProp";
import ChallengeField from "./ChallengeField";

export interface ChallengeFieldListProps extends ClassNameProp {
    fields: Array<Field>;
    onChange(fieldId: string, newValue: string): void;
}

export default function ChallengeFieldList({ fields, className, onChange }: ChallengeFieldListProps) {
    return (
        <div className={className}>
            {fields.map((field, i) => (
                <ChallengeField
                    key={field.id}
                    field={field}
                    defaultValue={field.value}
                    onChange={(e) => onChange(field.id, e.target.value)}
                    className={i !== fields.length - 1 ? "mb-2" : undefined}
                />
            ))}
        </div>
    )
}