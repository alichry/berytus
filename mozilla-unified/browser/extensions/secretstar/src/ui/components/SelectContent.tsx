import { ReactNode } from "react";
import { faArrowRight, faArrowLeft } from "@fortawesome/free-solid-svg-icons"
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { cn } from "@ui-utils/joinClassName";
import Select from "./Select";
import { ClassNameProp } from "../props/ClassNameProp";

type SelectableItem = { selectText: string; content: ReactNode };
type Item = ReactNode;

export interface SelectContentProps {
    position: number;
    setPosition(index: number): void;
    items: Array<Item> | Array<SelectableItem>;
    arrowLocation?: "top" | "bottom";
    barClassName?: string;
    enableProgressBar?: boolean;
    submitButton?: ReactNode;
    overrideDisableLeft?: boolean;
    overrideDisableRight?: boolean;
    validateBeforeNext?: () => boolean;
}

interface ArrowProps extends ClassNameProp {
    onClick(): void;
    dir: "left" | "right";
    enabled: boolean
}

function Arrow({ onClick, enabled, dir }: ArrowProps) {
    return (
        <FontAwesomeIcon
            icon={dir === "left" ? faArrowLeft : faArrowRight}
            className={cn(
                "text-lg",
                enabled ? 'text-blue-100 cursor-pointer hover:text-blue-200  active:translate-y-0.5' : 'text-blue-100/25',
            )}
            onClick={() => enabled && onClick()}
        />
    );
}

export default function SelectContent({ position, setPosition, items, arrowLocation = "top", barClassName, enableProgressBar, submitButton, overrideDisableLeft, overrideDisableRight, validateBeforeNext }: SelectContentProps) {
    const enableSelect: boolean = items.length > 0 ?
        typeof items[0] === "object" && items[0] !== null && "selectText" in items[0] :
        false;
    if (arrowLocation === "bottom" && enableSelect) {
        throw new Error('enableSelect must not be specific with arrowLocation = bottom');
    }
    const content: ReactNode = items.length > 0
        ? enableSelect ? (items[position] as SelectableItem).content : items[position] as ReactNode
        : null;
    const left = <Arrow
        dir="left"
        onClick={() => setPosition(position - 1)}
        enabled={!overrideDisableLeft && position > 0}
    />;
    const right = <Arrow
        dir="right"
        onClick={() => {
            if (validateBeforeNext && ! validateBeforeNext()) {
                return;
            } 
            setPosition(position + 1);
        }}
        enabled={!overrideDisableRight && position < items.length - 1}
    />;
    const select = enableSelect ? (
        <Select
            onChange={(e) => setPosition(Number(e.target.value))}
            value={position}
        >
            {(Object.values(items))
                .map((item: SelectableItem, i) => (
                    <option key={i} value={i}>
                        {item.selectText}
                    </option>
                ))}
        </Select>
    ) : null;
    const bar = (
        <div className={cn("flex flex-row gap-4 items-center mb-4", barClassName)}>
            {select}
            { arrowLocation === "bottom" ? left : null}
            <div className="flex-1" />
            { arrowLocation === "bottom" ? null : left}
            { ! submitButton ? right : (
                position === items.length - 1 ? submitButton : right
            )}
        </div>
    );
    if (arrowLocation === "top") {
        return <>
            {bar}
            {content}
        </>
    }
    return <>
        {content}
        {bar}
        {enableProgressBar && (! submitButton || position !== items.length - 1) ? (
            <div className="bg-fuchsia-400 h-2.5 rounded-3xl">
                <div
                    className="bg-fuchsia-800 h-full rounded-3xl"
                    style={{ width: `${Math.floor((position + 1) * 100 / items.length)}%`}}
                >
                </div>
            </div>
        ) : null}
    </>
}
