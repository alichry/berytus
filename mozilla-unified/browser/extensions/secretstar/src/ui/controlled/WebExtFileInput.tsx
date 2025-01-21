/**
 * WIP. Do not use.
 * Unfortunately, when the native modal UI
 * to select a file to upload, it closes
 * the page action popup. This is a WIP on a potential
 * workaround that communicates with the background script
 * to open the native UI there and transfer
 * the data.
 * Bug: https://bugzilla.mozilla.org/show_bug.cgi?id=1292701
 */
import { useEffect, useRef, useState } from "react";
import { fileInputButtonClassName } from "../components/FileInput";
import { cn } from "../utils/joinClassName";
import { RemovableFileInputProps } from "./RemovableFileInput";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faXmark } from "@fortawesome/free-solid-svg-icons";

export interface WebExtFileInputProps extends RemovableFileInputProps {}

export default function WebExtFileInput({ onUpload, onCancel, rowGap = "gap-2", className, before, forceShowDelete, ...props }: WebExtFileInputProps) {
    const [filename, setFilename] = useState<string>();
    const { file, request } = useWebExtUpload();
    const previousFile = useRef<WebExtUploadedFile | null>(null);

    useEffect(() => {
        if (previousFile && onCancel) {
            onCancel();
        }
        if (file) {
            //onUpload(file);
            previousFile.current = file;
        } else {
            previousFile.current = null;
        }
    }, [file]);

    return <div className={cn(`text-sm flex flex-row ${rowGap} items-center`, className)}>
        {before}
            {(filename !== undefined || forceShowDelete) ? (
                <div
                    onClick={() => {
                        previousFile.current = null;
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
        <div className={fileInputButtonClassName()} />
    </div>;
    // return <FileInput
    //     ref={inputRef}
    //     {...props}
    //     onChange={(e) => {
    //         if (e.target.files && e.target.files.length > 0) {
    //             setFilename(e.target.files[0].name);
    //             if (onUpload) {
    //                 onUpload(e.target.files);
    //             }
    //             return;
    //         }
    //     }}
    //     after={
    //         <>
    //             <FileInputText
    //                 filename={filename}
    //                 placeholder={placeholder}
    //                 placeholderClassName={placeholderClassName}
    //                 filenameClassName={filenameClassName}
    //             />
    //             {after}
    //         </>
    //     }
    // />
}

interface WebExtUploadFileHook {
    file?: WebExtUploadedFile;
    request: () => void;
    reset: () => void;
}

export interface WebExtUploadedFile {
    base64Data: string; type: string; filename: string
}


export function useWebExtUpload(): WebExtUploadFileHook {
    // const [file, setFile]
    return {
        request() {},
        reset() {}
    }
}
