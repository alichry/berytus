import { Container } from "./Container";
import TopBar from "../controlled/TopBar";
import { ClassNameProp } from "../props/ClassNameProp";
import Notice from "./Notice";

export interface ErrorPageProps extends ClassNameProp {
    text: string;
}

export default function ErrorPage({ text }: ErrorPageProps) {
    return <Container>
        <TopBar />
            <Notice type="error" text={text} />
    </Container>
}