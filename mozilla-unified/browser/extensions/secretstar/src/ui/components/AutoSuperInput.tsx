import { faCheck, faXmark } from "@fortawesome/free-solid-svg-icons";
import SuperInput, { SuperInputProps } from "../components/SuperInput";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import styles from "@styles/AutoSuperInput.module.scss";

export interface AutoSuperInputProps extends Omit<SuperInputProps, 'buttonAfter'> {}

export default function AutoSuperInput({
    ...otherProps
}: AutoSuperInputProps) {
    return (
        <SuperInput
            {...otherProps}
            inputClassName={styles.input}
            afterContainerClassName={styles.iconContainer}
            buttonAfter={
                <>
                    <FontAwesomeIcon
                        className={styles.iconSubmitted}
                        icon={faCheck}
                    />
                    <FontAwesomeIcon
                        className={styles.iconError}
                        icon={faXmark}
                    />
                </>
            }
        />
    )
}