import { forwardRef, ReactNode, useRef } from "react";
import { InputProps } from "./Input";
import { useState } from "react";
import { cn } from "../utils/joinClassName";
import styles from "@styles/SuperInput.module.scss";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faEye, faEyeSlash } from "@fortawesome/free-solid-svg-icons";

export interface SuperInputProps extends InputProps {
    type: string; // force type to be required
    inputIcon?: ReactNode; // TODO: Replace with FA IconProp
    enableVisibilityToggle?: boolean;
    preIconContainerClassName?: string;
    afterContainerClassName?: string;
    inputClassName?: string;
    buttonAfter?: ReactNode;
    dirty?: boolean;
}

const SuperInput = forwardRef<
    HTMLInputElement,
    SuperInputProps
>(function SuperInput({ type, inputClassName, preIconContainerClassName, afterContainerClassName, className, inputIcon, enableVisibilityToggle = true, noMargin, buttonAfter, dirty, ...otherProps}: SuperInputProps, ref) {
    const [show, setShow] = useState<boolean>(false);
    return <div className={cn(styles.inputContainer, noMargin && "!my-0", className)}>
        <input
            autoComplete="off"
            {...otherProps}
            type={type !== "password" ? type : (
                show && enableVisibilityToggle ? "text" : "password"
            )}
            className={cn(styles.input, inputClassName, !!inputIcon && styles.inputPreIconShown)}
            ref={ref}
            data-dirty={dirty ? "1" : undefined}
        />
        {inputIcon ? ( // position: absolute.
            <div className={cn(styles.preIcon, preIconContainerClassName)}>
                {inputIcon}
            </div>
        ) : null}
        <div className={cn(styles.afterContainer, afterContainerClassName)}>
            { type === "password" && enableVisibilityToggle ? (
                <FontAwesomeIcon
                    icon={show ? faEyeSlash : faEye}
                    className="cursor-pointer"
                    onClick={() => setShow(!show)}
                />
            ) : null}
            {buttonAfter}
        </div>
    </div>
})

export default SuperInput;