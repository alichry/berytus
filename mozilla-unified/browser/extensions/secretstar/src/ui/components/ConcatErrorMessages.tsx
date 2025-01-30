import { concatErrorMessages } from "@root/utils";
import Notice from "./Notice";

export interface ConcatErrorMessagesProps {
    errors: Array<Error | undefined>;
}

export default function ConcatErrorMessages({ errors }: ConcatErrorMessagesProps) {
    const message = concatErrorMessages(...errors);
    return <Notice
        type="error"
        text={
            message.length === 0
            ? "An error has occurred however we are not able to display it! " +
                "None of the passed errors are set! "
            : `Error: ${message}`
        }
    />

}