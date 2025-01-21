import styles from '@styles/CondensedTable.module.scss';
import { cn } from "@root/ui/utils/joinClassName";
import { CondensedTableProps } from "./CondensedTable";

export interface TableProps extends CondensedTableProps {
    scrollable?: {
        fullWidth?: boolean;
        maxWidth?: string;
        fullHeight?: boolean;
        maxHeight?: string;
    }
    dataRowClassName?: string;
    dataRowColClassName?: string;
}

export default function Table({ rows, cols, className, scrollable, dataRowClassName, dataRowColClassName }: TableProps) {

    const table = <table className={cn(styles.tableContainer, styles.noBorderRadius, scrollable?.fullWidth && 'w-full', scrollable?.fullHeight && 'h-full', className)}>
        { cols ?
        <thead>
            <tr>
                {cols.map((c, j) =>
                    <th key={j} className={cn(j === cols.length - 1 && '!pr-6')}>{c}</th>
                )}
            </tr>
        </thead> : null}
        <tbody>
            {rows.map((row, i) => <tr key={i} className={dataRowClassName}>{row.map((c, j) =>
                <td key={j} className={cn("align-top", dataRowColClassName, j === row.length - 1
                 && '!pr-6')}>{c}</td>
            )}</tr>)}
        </tbody>
    </table>;
    if (! scrollable) {
        return table;
    }

    return (
        <div
            className={cn(
                scrollable.fullWidth && 'w-full',
                scrollable.maxWidth,
                scrollable.fullHeight &&  'h-full',
                scrollable.maxHeight,
                "overflow-auto inline-block"
            )}
        >
            {table}
        </div>
    );
}