import { forwardRef, useState } from "react";
import FileInput, { FileInputProps } from "../components/FileInput";
import { cn } from "@ui-utils/joinClassName";

export interface FileInputTextProps {
    filename?: string;
    placeholder?: string;
    filenameClassName?: string;
    placeholderClassName?: string
}
export function FileInputText({ filename, filenameClassName, placeholderClassName, placeholder = "Please select a file." }: FileInputTextProps) {
    return filename ? (
        <p
            className={cn(filenameClassName)}
        >
            {filename}
        </p>
    ) : (
        <p
            className={cn("text-gray-300 italic select-none", placeholderClassName)}
        >
            {placeholder}
        </p>
    );
}

export interface FileInputWithTextProps extends
    Omit<FileInputProps, 'multiple'>,
    Omit<FileInputTextProps, 'filename'> {}

const FileInputWithText = forwardRef<
    HTMLInputElement,
    FileInputWithTextProps
>
(function FileInputWithText({ filenameClassName, placeholder, placeholderClassName, ...props }, ref) {
    const [filename, setFilename] = useState<string>();
    return <FileInput
        ref={ref}
        {...props}
        multiple={false}
        onChange={(e) => {
            if (! e.target.files) {
                setFilename(undefined);
                return;
            }
            setFilename(e.target.files[0].name);
            if (props.onChange) {
                props.onChange(e);
            }
        }}
        after={
            <>
                <FileInputText
                    filename={filename}
                    placeholder={placeholder}
                    filenameClassName={filenameClassName}
                    placeholderClassName={placeholderClassName}
                />
                {props.after}
            </>
        }
    />
})

export default FileInputWithText;