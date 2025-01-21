import { cn } from "@ui-utils/joinClassName";
import '@styles/select.scss';
import { forwardRef } from 'react';

export interface SelectProps extends React.DetailedHTMLProps<React.SelectHTMLAttributes<HTMLSelectElement>, HTMLSelectElement> {
    color?: "purple"
}

const Select = forwardRef<
    HTMLSelectElement,
    SelectProps
>(function Select({ children, color = "purple", className, ...otherProps }, ref) {
    return <select
        ref={ref}
        {...otherProps}
        className={cn(`${color}Select`, className)}
    >
        {children}
    </select>
});
export default Select;