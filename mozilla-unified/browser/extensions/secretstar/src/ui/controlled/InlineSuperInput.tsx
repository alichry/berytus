import { faCheck } from "@fortawesome/free-solid-svg-icons";
import Button, { ButtonProps } from "../components/Button";
import SuperInput, { SuperInputProps } from "../components/SuperInput";
import { IconProp } from "@fortawesome/fontawesome-svg-core";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";

export interface InlineSuperInput extends Omit<SuperInputProps, 'ref' | 'buttonAfter'> {
    buttonAfterColor?: ButtonProps['color'];
    buttonAfterIcon?: IconProp;
    buttonAfterIconColor?: string;
    hideButton?: boolean;
}

export default function InlineSuperInput({
    buttonAfterColor: afterButtonColor = "teal",
    buttonAfterIcon: afterButtonIcon = faCheck,
    buttonAfterIconColor: afterButtonIconColor = "white",
    hideButton,
    ...otherProps
}: InlineSuperInput) {
    return (
        <SuperInput
            {...otherProps}
            //inputClassName={styles.input}
            buttonAfter={
                hideButton
                ? <Button
                    color={afterButtonColor}
                    borderStyle="inputButtonAfter"
                    //className={styles.button}
                    text={
                        <FontAwesomeIcon
                            icon={afterButtonIcon}
                            color={afterButtonIconColor}
                        />
                    }
                />
                : null
            }
        />
    )
}