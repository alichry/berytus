import { forwardRef, ReactNode } from "react";
import { ClassNameProp } from "../props/ClassNameProp";
import BorderlessInput, { BorderlessInputProps } from "./BorderlessInput";
import { cn } from "../utils/joinClassName";

export interface LabeledInputProps
    extends ClassNameProp, Omit<BorderlessInputProps,
        'className'
    > {
    inputContainerClassName?: string;
    label: ReactNode;
}

const LabeledInput = forwardRef<
    HTMLInputElement & HTMLTextAreaElement,
    LabeledInputProps
>(function LabeledInput({ className, type, inputContainerClassName, inputClassName, afterContainerClassName, label, ...inputProps }, ref) {
    return (
        <div className={cn(
            "bg-gray-900",
            type === "textarea" ? "rounded-lg" : "rounded-2xl",
            className
        )}
        >
            { type === "textarea" ? (
                <div className="pl-3 pt-2 font-code text-teal-300 text-xs pb-2 border-0 border-gray-600 border-b">
                    {label}
                </div>
            ) : null}
            <div className={cn(
                "flex flex-row",
                type !== "textarea" ? "px-3" : "pl-1.5"
            )}>
                <BorderlessInput
                    ref={ref}
                    type={type}
                    {...inputProps}
                    className={cn("py-3", inputContainerClassName)}
                    afterContainerClassName={cn(
                        type === "password"
                        && "!pr-[0.25rem] !pl-[0.5rem] !text-default", // icon is not white for some reason
                        afterContainerClassName
                    )}
                    //"!text-default/[0.88] ",
                    //"!text-violet-200 ",
                    inputClassName={cn(
                        "!text-sm",
                        inputProps.disabled ? "!text-gray-300" : "!text-gray-50",
                        inputClassName
                    )}
                    textAreaColor={
                        inputProps.textAreaColor ||
                        type === "textarea"
                        ? inputProps.disabled ? "lightgray" : 'light'
                        : undefined
                    }
                    dirty
                />
                {type !== "textarea" ? (
                    <div className={cn(
                        "py-3",
                        "whitespace-nowrap flex-1 font-code text-teal-300 text-xs",
                        "my-auto",
                        "ml-2 pl-2 border-0 border-gray-600 border-l"
                    )}>{label}</div>
                ) : null}

            </div>

        </div>
    )
});

export default LabeledInput;