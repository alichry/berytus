import { useRef, useState } from "react";
import Button, { ButtonProps } from "../components/Button";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faFileUpload } from "@fortawesome/free-solid-svg-icons";
import { ClassNameProp } from "../props/ClassNameProp";
import { cn } from "../utils/joinClassName";

export interface UploadImageProps extends ClassNameProp {
    selectFileTextClassName?: string;
}

export default function UploadImage({ className, selectFileTextClassName }: UploadImageProps) {
    const imageRef = useRef<HTMLInputElement>(null);
    const [filename, setFilename] = useState<string>();
    return (
        <div className={cn("text-xs flex flex-row gap-1.5 items-center", className)}>
            <input
                ref={imageRef}
                type="file"
                accept="image/*"
                onChange={(e) => {
                    if (! e.target.files) {
                        setFilename(undefined);
                        return;
                    }
                    setFilename(e.target.files[0].name);
                }}
                className="hidden"
            />
            <Button
                className="h-fit"
                text={
                    <FontAwesomeIcon icon={faFileUpload} />
                }
                onClick={() => imageRef.current?.click()}
                size="xs"
            />
            { filename ? (
                <p
                    className="text-gray-600"
                >
                    {filename}
                </p>
            ) : (
                <p
                    className={cn("text-gray-600 italic select-none", selectFileTextClassName)}
                >
                    Please select a file.
                </p>
            )}
        </div>
    )
}