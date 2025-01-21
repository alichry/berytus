import { fieldOptionValueToString } from "@root/db/db";
import { cn } from "../utils/joinClassName";
import KeyValue, { KeyValueProps } from "./KeyValue";
import { fieldOptionsLabels } from "../utils/fieldOptionsLabels";
import type { FieldInfo } from "@berytus/types";

export interface FieldOptionsKeyValueProps extends Omit<KeyValueProps, 'rows'> {
    field: FieldInfo;
    hostname?: string;
}

export default function FieldOptionsKeyValue({ field, className, valueClassName, hostname, ...otherProps }: FieldOptionsKeyValueProps) {
    return (
        <KeyValue
            className={(cn("text-sm", className))}
            valueClassName={cn("font-value", valueClassName)}
            {...otherProps}
            rows={[
                ...(hostname ? [[
                    <p>Domain</p>,
                    <p>{hostname}</p>
                ] as const] : []),
                [
                    <p>ID</p>,
                    <p>{field.id}</p>
                ] as const,
                [
                    <p>Type</p>,
                    <p>{field.type}</p>
                ] as const,
                ...(Object.keys(field.options) as Array<keyof typeof field.options>)
                    .map((key) => {
                        const value = fieldOptionValueToString(key, field.options[key]);
                        return [
                            <p>{fieldOptionsLabels[key]}</p>,
                            typeof value === "string" && value === ""
                                ? <p className="italic font-value">Empty</p>
                                : <p>{String(value)}</p>
                        ] as const;
                })
            ]}
        />
    )
}