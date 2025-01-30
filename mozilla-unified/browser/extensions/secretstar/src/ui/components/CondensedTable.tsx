import { ReactNode } from "react";
import styles from '@styles/CondensedTable.module.scss';
import { ClassNameProp } from "@props/ClassNameProp";
import { cn } from "@root/ui/utils/joinClassName";

export interface CondensedTableProps extends ClassNameProp {
    rows: Array<Array<ReactNode>>;
    cols?: Array<ReactNode>;
    rowClassName?: string;
    evenRowClassName?: string;
    rowColClassName?: string;
    evenRowColClassName?: string;
}

export default function CondensedTable({ rows, cols, className, rowClassName, evenRowClassName, rowColClassName, evenRowColClassName }: CondensedTableProps) {
    return <table className={cn(styles.tableContainer, className)}>
        { cols ?
        <thead>
            <tr>
                {cols.map((c, j) =>
                    <th
                        key={j}
                    >{c}</th>
                )}
            </tr>
        </thead> : null}
        <tbody>
            {rows.map((row, i) =>
                <tr key={i}
                    className={cn(rowClassName, (i + 1) % 2 === 0 && evenRowClassName)}
                >
                    {row.map((c, j) =>
                        <td
                            key={j}
                            className={cn("align-top", rowColClassName, (i + 1) % 2 === 0 && evenRowColClassName)}
                        >
                            {c}
                        </td>
                    )}
                </tr>
            )}
        </tbody>
    </table>
}

interface ValueColumnProps extends ClassNameProp {
    text: ReactNode;
    textSize?: `text-${string}`;
    onClick?: () => void;
}

export function ValueColumn({ text, className, textSize = 'text-base', onClick }: ValueColumnProps) {
    return (
        <div
            className={cn("text-left", textSize, className)}
            onClick={onClick ? () => onClick() : undefined}
        >
            {text}
        </div>
    )
}

export function ValueTypeColumn({ text, textSize = 'text-base', className, onClick }: ValueColumnProps) {
    return (
        <div
            className={cn("text-right", textSize, "text-gray-600", className)}
            onClick={onClick ? () => onClick() : undefined}
        >
            {text}
        </div>
    );
}