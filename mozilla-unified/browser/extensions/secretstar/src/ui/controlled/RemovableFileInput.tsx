import { useRef, useState } from "react";
import { FileInputText, FileInputTextProps } from "./FIleInputWithText";
import FileInput, { FileInputProps, fileInputButtonClassName } from "../components/FileInput";
import { cn } from "../utils/joinClassName";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faXmark } from "@fortawesome/free-solid-svg-icons";

export interface RemovableFileInputProps
    extends Omit<FileInputProps, 'onChange'>, Omit<FileInputTextProps, 'filename'> {
    forceShowDelete?: boolean;
    onUpload(files: FileList): void;
    onCancel?: () => void;
}

function RemovableFileInput({ forceShowDelete, onUpload, before, after, onCancel, placeholder, placeholderClassName, filenameClassName,  ...props }: RemovableFileInputProps) {
    const inputRef = useRef<HTMLInputElement>(null);
    const [filename, setFilename] = useState<string>();
    return <FileInput
        ref={inputRef}
        {...props}
        onChange={(e) => {
            if (e.target.files && e.target.files.length > 0) {
                setFilename(e.target.files[0].name);
                if (onUpload) {
                    onUpload(e.target.files);
                }
                return;
            }
        }}
        before={<>
            {before}
            {(filename !== undefined || forceShowDelete) ? (
                <div
                    onClick={() => {
                        if (! inputRef.current) {
                            return;
                        }
                        inputRef.current.files = null;
                        setFilename(undefined);
                        if (onCancel) {
                            onCancel();
                        }
                    }}
                    className={cn(fileInputButtonClassName({ bgColor: "red" }))}
                >
                    <FontAwesomeIcon icon={faXmark} size="lg" />
                </div>
            ) : null}
        </>}
        after={
            <>
                <FileInputText
                    filename={filename}
                    placeholder={placeholder}
                    placeholderClassName={placeholderClassName}
                    filenameClassName={filenameClassName}
                />
                {after}
            </>
        }
    />
}
export default RemovableFileInput;