import { cn } from "@ui-utils/joinClassName";
import '@styles/buttons.scss';
import * as React from 'react';

export interface ButtonProps extends React.DetailedHTMLProps<React.ButtonHTMLAttributes<HTMLButtonElement>, HTMLButtonElement> {
    text: React.ReactNode;
    color?: "purple" | "red" | "blue" | "teal";
    borderStyle?: "default" | "inputButtonAfter";
    size?: "xs" | "default";
}

export default function Button({ text, color = "purple", className, borderStyle = "default", size = "default", ...otherProps }: ButtonProps) {
    return <button
        {...otherProps}
        className={cn(
            `${color}Button`,
            `${borderStyle}BorderStyle`,
            size !== "default" && `${size}Size`,
            className
        )}
    >
        {text}
    </button>
}