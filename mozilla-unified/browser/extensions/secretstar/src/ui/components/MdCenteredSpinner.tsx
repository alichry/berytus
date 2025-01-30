import { ClassNameProp } from "../props/ClassNameProp";
import { cn } from "../utils/joinClassName";
import Spinner from "./Spinner";

export interface MdCenterSpinnerProps extends ClassNameProp {}

export default function MdCenteredSpinner({ className }: ClassNameProp) {
    return <Spinner width="2rem" className={cn("mx-auto", className)} />;
}