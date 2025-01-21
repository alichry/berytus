import { ClassNameProp } from '@props/ClassNameProp';
import styles from '@styles/IntentActionSplit.module.scss';
import { cn } from '@root/ui/utils/joinClassName';
import Button from './Button';
import { ELoginUserIntent } from '@berytus/enums';

export interface IntentActionSplitProps extends ClassNameProp {
    onSubmit: (intent: (typeof ELoginUserIntent)[keyof typeof ELoginUserIntent]) => void;
}

export default function IntentActionSplit(props: IntentActionSplitProps) {
    return (
        <div className={cn(styles.container, props.className)}>
            <div className={styles.leftContainer}>
                <p className="text-xl text-center">Your saved accounts</p>
                <div className={styles.savedAccountsContainer}>
                    <p className={styles.savedAccountRow}>john@doe.com</p>
                    <p className={styles.savedAccountRow}>jane@doe.com</p>
                    <p className={styles.savedAccountRow}>john@doe.com</p>
                    <p className={styles.savedAccountRow}>jane@doe.com</p>
                    <p className={styles.savedAccountRow}>jane@doe.com</p>
                </div>
            </div>
            <div className={styles.rightContainer}>
                <p className="text-xl text-center">Action</p>
                <Button
                    text="Sign up"
                    className={styles.button80}
                    onClick={() => props.onSubmit(ELoginUserIntent.Register)}
                />
                <p className="text14 text-center">Or</p>
                <Button
                    text="Login"
                    className={styles.button80}
                    onClick={() => props.onSubmit(ELoginUserIntent.Authenticate)}
                />
            </div>
        </div>
    )
}