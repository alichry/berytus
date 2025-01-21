import { DetailedHTMLProps, ImgHTMLAttributes } from "react";
import spinnerGif from '@styles/images/authRealmSpinner.gif';

interface SpinnerProps extends Omit<DetailedHTMLProps<ImgHTMLAttributes<HTMLImageElement>, HTMLImageElement>, 'src' | 'height'> {}

export default function Spinner(props: SpinnerProps) {
    return (
        <img
            {...props}
            style={{
                width: props.width || 48,
                height: props.width
            }}
            src={spinnerGif}
        />
    );
}