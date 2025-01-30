import { ClassNameProp } from '@props/ClassNameProp';
import logoPng from '@styles/images/logo48.png';
import styles from '@styles/Logo.module.scss';
import { cn } from '@root/ui/utils/joinClassName';

export interface LogoProps extends ClassNameProp {
    onClick?: () => void;
};

export function Logo(props: LogoProps) {
    return <div
        onClick={props.onClick}
        className={cn(styles.logoContainer, props.className)}
    >
        <img className={styles.logo} src={logoPng} />
        <p className={styles.logoText}>Secret*</p>
    </div>
}