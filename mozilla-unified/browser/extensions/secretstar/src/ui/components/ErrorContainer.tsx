import { PropsWithChildren } from "react";
import { Container } from "./Container";
import TopBar from "../controlled/TopBar";
import styles from "@styles/Loading.module.scss";
import { cn } from "../utils/joinClassName";
import { ClassNameProp } from "../props/ClassNameProp";
import Notice from "./Notice";

export interface ErrorContainerProps extends ClassNameProp {
    error: Error;
}

export default function ErrorContainer({ error, children, className }: PropsWithChildren<ErrorContainerProps>) {
    return <Container className={className}>
        <TopBar />
            <div className={cn(styles.loading, "absolute inset-0 m-auto flex flex-col gap-4 items-center justify-center")}>
                {children}
                <Notice text={error.message} type="error" />
            </div>
    </Container>
}