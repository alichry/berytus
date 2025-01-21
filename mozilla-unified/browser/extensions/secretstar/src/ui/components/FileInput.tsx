import { ReactNode, forwardRef } from "react";
import { InputProps } from "./Input";
import Button, { ButtonProps } from "./Button";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faFileUpload } from "@fortawesome/free-solid-svg-icons";
import { cn } from "../utils/joinClassName";

export interface FileInputProps extends Omit<InputProps, 'type' | 'value' | 'defaultValue' > {
    color?: "purple" | "blue";
    rowGap?: string;
    before?: ReactNode;
    after?: ReactNode;
    /**
     * Useful override to prevent the upload file dialog
     * in some circumstances, e.g. when the application
     * is in a popup.
     */
    shouldPreventDefault?: () => boolean;
    uploadButtonClassName?: string;
    uploadButtonTitle?: string;
    prompt?: () => void;
}

interface FileInputButtonClassNameOptions {
    bgColor?: "violet" | "red" | "blue";
    color?: "white";
}

export function fileInputButtonClassName({ bgColor, color }: FileInputButtonClassNameOptions = {}) {
    const cBgColor = bgColor === 'violet'
        ? "bg-violet-600"
        : bgColor === 'red'
        ? 'bg-red-700'
        : 'bg-blue-700';
    const cColor = "text-white";
    return `cursor-pointer ${cBgColor} ${cColor} rounded py-2 px-3`;
}

const FileInput = forwardRef<
    HTMLInputElement,
    FileInputProps
>(function FileInput({ shouldPreventDefault, uploadButtonTitle, className, before, after, rowGap = "gap-2", color = "purple", uploadButtonClassName, ...inputProps }, ref) {
    return (
        <div className={cn(`text-sm flex flex-row ${rowGap} items-center`, className)}>
            {before}
            <label
                title={uploadButtonTitle}
                className={cn(fileInputButtonClassName(), uploadButtonClassName)}
                onClick={(e) => {
                    if (shouldPreventDefault && shouldPreventDefault()) {
                        e.preventDefault();
                    }
                }}
            >
                <input
                    ref={ref}
                    type="file"
                    {...inputProps}
                    className="hidden"
                />
                <FontAwesomeIcon icon={faFileUpload} size="lg" />
            </label>
            {after}
        </div>
    )
})

export default FileInput;