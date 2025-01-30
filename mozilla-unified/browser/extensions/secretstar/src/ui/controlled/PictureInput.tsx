import { useState } from "react";
import { FileInputTextProps } from "./FIleInputWithText";
import RemovableFileInput from "./RemovableFileInput";
import { db } from "@root/db";
import { usePicture } from "@root/hooks";
import { cn } from "../utils/joinClassName";
import WebExtSafeFileInput from "@controlled/WebExtSafeFileInput";
import { ClassNameProp } from "../props/ClassNameProp";

export interface PictureInputProps extends Omit<FileInputTextProps, 'filename'>, ClassNameProp {
    onUpload: (pictureId: string) => void;
    onCancel: () => void;
    defaultPictureId?: string;
}

export default function PictureInput({ className, placeholder, placeholderClassName, filenameClassName, onUpload, onCancel, defaultPictureId, ...props }: PictureInputProps) {
    const [pictureId, setPictureId] = useState<string | undefined>(defaultPictureId);
    const picture = usePicture(pictureId);

    return (
        <div className={cn("p-2.5 border-[0.5px] border-gray-100 rounded",  className)}>
            <WebExtSafeFileInput
                className={"text-xs"}
                // In RemovableFileInput placeholder will be shown when there
                // is no picture. When we want to display a default picture,
                // it is not yet loaded in the input file. We can leverage the
                // placeholder display to render the existing filename and unset
                // its italic style. If the input file has a file selected,
                // `placeholder` will be ignored, so it is safe to override it.
                placeholder={picture ? picture.filename : placeholder}
                placeholderClassName={cn(picture && "not-italic text-inherit", placeholderClassName)}
                forceShowDelete={!!picture}
                filenameClassName={cn(filenameClassName, )}
                onUpload={async (files) => {
                    const newPictureId = await db.savePicture(files[0]);
                    setPictureId(newPictureId.toString());
                    onUpload(newPictureId.toString());
                }}
                onCancel={async () => {
                    if (pictureId) {
                        await db.picture.delete(pictureId);
                    }
                    setPictureId(undefined);
                    onCancel();
                }}
            />
            { picture ? (
                <img
                    className="mt-4 mx-auto rounded-[50%]"
                    src={db.pictureToDataUrl(picture)}
                    width={128}
                    height={128}
                />
            ) : null}

        </div>
    )
}