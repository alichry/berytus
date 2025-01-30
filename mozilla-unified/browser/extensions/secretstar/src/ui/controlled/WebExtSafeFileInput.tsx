import { useParams } from "react-router-dom";
import FileInput, { FileInputProps } from "../components/FileInput";
import { isPopup } from "@root/pagecontext";
import RemovableFileInput, { RemovableFileInputProps } from "./RemovableFileInput";
import { cn } from "../utils/joinClassName";

export interface WebExtSafeFileInput extends Omit<RemovableFileInputProps, 'shouldPreventDefault' | 'uploadButtonTitle'> {}


export default function WebExtSafeFileInput({ uploadButtonClassName, ...props }: WebExtSafeFileInput) {
    const { pageContext } = useParams();
    return <RemovableFileInput
        {...props}
        uploadButtonClassName={cn(
            uploadButtonClassName,
            pageContext && isPopup(pageContext) ?
            "bg-gray-200 text-gray-400 !cursor-not-allowed" : undefined
        )}
        uploadButtonTitle="File uploads are disabled in popup mode. This is due to a 7-year Mozilla bug that closes the popup during file upload."
        shouldPreventDefault={() => {
            if (! pageContext) {
                return false;
            }
            return isPopup(pageContext);
        }}
    />
}