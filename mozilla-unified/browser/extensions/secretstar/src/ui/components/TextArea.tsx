import { TextareaHTMLAttributes, forwardRef } from "react";
import styles from "@styles/TextArea.module.scss";
import { cn } from "../utils/joinClassName";

export interface TextAreaProps extends TextareaHTMLAttributes<HTMLTextAreaElement> {
    noMargin?: boolean;
    dirty?: boolean;
}

const TextArea = forwardRef<
    HTMLTextAreaElement,
    TextAreaProps
>(function TextArea({ className, noMargin, dirty, ...otherProps }: TextAreaProps, ref) {
    return (
        <textarea
            className={cn(
                styles.textArea, // normal styling
                noMargin && "my-0",
                className
            )}
            rows={5}
            cols={100} // arbitrary long value to fill the width
            {...otherProps}
            ref={ref}
            data-dirty={dirty ? "1" : undefined}
        />
    )
})

export default TextArea;