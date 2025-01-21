import TextWithIcon, { TextWithIconProps } from "./TextWithIcon";

export interface CenteredTextWithIconProps extends Omit<TextWithIconProps, 'centered'> {}

export default function CenteredTextWithIcon(props: CenteredTextWithIconProps) {
    return <TextWithIcon {...props} centered={true} />
}