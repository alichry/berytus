import Notice from "./Notice";

export interface ErrorNoticeProps {
    error: Error;
}

export default function ErrorNotice({ error }: ErrorNoticeProps) {
    return <Notice
        type="error"
        text={`Error: ${error.message}`}
    />
}