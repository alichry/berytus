import { ReactNode } from "react";
import { cn } from "../utils/joinClassName";
import styles from '@styles/KeyValue.module.scss';
import { ClassNameProp } from "../props/ClassNameProp";

export interface KeyValueProps extends ClassNameProp {
    rows: Array<readonly [key: ReactNode, value: ReactNode]>;
    dir?: "row" | "col";
    gap?: `gap-${number}`;
    withBorder?: boolean
    keyClassName?: string;
    valueClassName?: string;
    pairGap?: `gap-${number}`; // gap between each key-value pair
    pairDir?: "row" | "col";
}
//1px solid #dfdfdf;
export default function KeyValue({ gap, pairGap, rows, dir = "row", pairDir = "col", withBorder = false, className, valueClassName, keyClassName }: KeyValueProps) {
    if (gap === undefined) {
        gap = dir === 'row' ? 'gap-6' : 'gap-4';
    }
    if (pairGap === undefined) {
        pairGap = dir === 'row' ? 'gap-4' : 'gap-2';
    }

    return (
        <div className={cn(`flex flex-wrap flex-${pairDir}`, `${pairGap}`, className)}>
            {rows.map((row, i) => (
                <div className={`flex flex-${dir} ${gap} ${(row[0] || row[1]) && withBorder ? styles.rowWithBorder : ""}`} key={i}>
                    <div className={keyClassName}>{row[0]}</div>
                    <div className={valueClassName}>{row[1]}</div>
                </div>
            ))}
        </div>
    )
}