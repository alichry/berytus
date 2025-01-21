import { ClassNameProp } from "../props/ClassNameProp";
import CondensedTable from "./CondensedTable";
import CondensedRowWithHint  from "./CondensedRowWithHint";
import CenteredTextWithIcon from "./CenteredTextWIthIcon";
import { faGear } from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";

const RowWithHint = ({ value, hint }: { value: string, hint: string }) => {
    return [<CondensedRowWithHint
            item={<p>{value}</p>}
            hint={hint}
            // white since we have a custom colored table
            hintClassName="!text-slate-300"
    />];
}


export interface ChallengeParametersWithProps extends ClassNameProp {
    parameters?: Record<string, unknown>;
}

export default function ChallengeParameters({ parameters, className }: ChallengeParametersWithProps) {

    if (! parameters || Object.keys(parameters).length === 0) {
        return null;
    }

    return (
        <div className={className}>
            <CenteredTextWithIcon
                icon={<FontAwesomeIcon icon={faGear} color="white" />}
                text="Challenge parameters"
                className="mb-3"
            />
            <CondensedTable
                className="w-full !bg-violet-700"
                rowColClassName="!text-white-600" // todo: text-default
                evenRowColClassName="!bg-violet-600"
                rows={
                    Object.keys(parameters).map((key) => (
                        RowWithHint({
                            value: String(parameters[key]),
                            hint: key
                        })
                    ))
                }
            />
        </div>
    );
}