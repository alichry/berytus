import styles from "@styles/Switch.module.scss";
import { InputHTMLAttributes, forwardRef } from "react";
import { cn } from "../utils/joinClassName";

export interface SwitchProps extends Omit<InputHTMLAttributes<HTMLInputElement>, 'type'> {
    text: string;
}

const Switch = forwardRef<HTMLInputElement, SwitchProps>(function Switch({ text, className, ...otherProps }: SwitchProps, ref) {
    return (
        <div className={cn("flex flex-row gap-2 mb-4", className)}>
            <label className={styles.switchContainer}>
                <input ref={ref} type="checkbox" className={styles.checkbox} {...otherProps} />
                <span className={styles.switch} />
            </label>
            <p className="my-auto text-sm">{text}</p>
        </div>

    );
});

export default Switch;