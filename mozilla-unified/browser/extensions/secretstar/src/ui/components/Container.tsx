import { ClassNameProp } from '@props/ClassNameProp';
import styles from '@styles/Container.module.scss';
import { PropsWithChildren } from 'react';
import { cn } from '@root/ui/utils/joinClassName';

export interface ContainerProps extends PropsWithChildren<ClassNameProp> {
    className?: string;
    width?: string;
    height?: string;
}

export function Container(props: ContainerProps) {
    return <div className={cn(styles.container, props.className)} style={{ height: props.height, width: props.width }}>
        {props.children}
    </div>
}