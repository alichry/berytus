import { InputHTMLAttributes, PropsWithRef, forwardRef } from "react";
import styles from '@styles/Input.module.scss';
import { cn } from "../utils/joinClassName";

export interface InputProps extends InputHTMLAttributes<HTMLInputElement> {
    noMargin?: boolean;
}

const Input = forwardRef<
    HTMLInputElement,
    InputProps
>(function Input(props: InputProps, ref) {
    return <input
        ref={ref}
        {...props}
        className={cn(styles.input, props.noMargin && "my-0", props.className)}
    />
})

export default Input;