import { PropsWithChildren } from "react";
import { Container } from "./Container";
import TopBar from "../controlled/TopBar";
import Spinner from "./Spinner";
import styles from "@styles/Loading.module.scss";
import { cn } from "../utils/joinClassName";
import { ClassNameProp } from "../props/ClassNameProp";

export interface LoadingProps extends ClassNameProp {
    width?: number
}

export default function Loading({ width = 48, children, className }: PropsWithChildren<LoadingProps>) {
    return <Container className={className}>
        <TopBar />
            {/* <div className={"flex flex-col gap-4 items-center justify-center"}>
                {children}
                <Spinner width={width} />
            </div> */}
            <div className={cn(styles.loading, "absolute inset-0 m-auto flex flex-col gap-4 items-center justify-center")}>
                {children}
                <Spinner width={width} />
            </div>
    </Container>
}