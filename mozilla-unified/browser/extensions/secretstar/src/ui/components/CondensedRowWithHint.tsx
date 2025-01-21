import { ReactNode } from "react";
import { cn } from "../utils/joinClassName";

export interface CondensedRowWithHintProps {
    item: ReactNode;
    hint: string;
    hintClassName?: string;
}
export default function CondensedRowWithHint({ item, hint, hintClassName }: CondensedRowWithHintProps) {
    return (
        <div className="flex flex-row gap-2 text-sm">
            <div className="flex-1">
                {item}
            </div>
            <p
                className={cn(`text-right text-gray-600 whitespace-nowrap`, hintClassName)}
            >
                {hint}
            </p>
        </div>
    )
}
