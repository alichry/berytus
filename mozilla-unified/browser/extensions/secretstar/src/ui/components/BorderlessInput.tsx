import { forwardRef } from "react";
import SuperInput, { SuperInputProps } from "./SuperInput";
import styles from "@styles/BorderlessInput.module.scss";
import { cn } from "../utils/joinClassName";
import TextArea, { TextAreaProps } from "./TextArea";

export type BorderlessInputProps = SuperInputProps & TextAreaProps & {
    type: SuperInputProps['type'] | "textarea";
    textAreaColor?: "light" | "gray" | "lightgray" | "unset"; // consumers seems to be faced with the challenge to change the text area color due to how the css structure is. .textArea has color: #X !important that cannot be overridden
}

const BorderlessInput = forwardRef<
    HTMLInputElement & HTMLTextAreaElement,
    BorderlessInputProps
>(function BorderlessInput({ type, textAreaColor = "gray", className, inputClassName, afterContainerClassName, preIconContainerClassName, required, placeholder, ...props }: BorderlessInputProps, ref) {
    if (type === "textarea") {
        return <TextArea
            ref={ref}
            className={cn(
                styles.textArea,
                className,
                inputClassName,
                textAreaColor && textAreaColor !== "unset" &&  styles[`textArea_${textAreaColor}`],
            )}
            placeholder={required && placeholder === undefined ? "Required" : placeholder}
            required={required}
            {...props}
        />
    }
    return <SuperInput
        ref={ref}
        type={type}
        className={cn(styles.inputContainer, className)}
        inputClassName={cn(styles.input, inputClassName)}
        preIconContainerClassName={cn(styles.preIconContainer, preIconContainerClassName)}
        afterContainerClassName={cn(styles.afterContainer, afterContainerClassName)}
        placeholder={required && placeholder === undefined ? "Required" : placeholder}
        required={required}
        {...props}
    />
});

export default BorderlessInput;
