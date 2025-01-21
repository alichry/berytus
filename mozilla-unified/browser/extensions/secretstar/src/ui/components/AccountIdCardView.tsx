import { faUser } from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { Account } from "@root/db";
import { ClassNameProp } from "../props/ClassNameProp";
import { cn } from "../utils/joinClassName";
import { EBerytusFieldType } from "@berytus/enums";

interface AccountIdCardViewProps extends ClassNameProp {
    account: Account;
    overrideColor?: `bg-${string}`;
    onClick?: () => void;
}

export function AccountIdCardView({ account, className, onClick, overrideColor }: AccountIdCardViewProps) {
    return (
        <div
            className={
                cn(
                    overrideColor ? overrideColor : "bg-blue-900",
                    "pt-4 pb-6 px-6 rounded",
                    className,
                    onClick ? 'hover:bg-[#260078] cursor-pointer' : ''
                )
            }
            onClick={() => {
                if (! onClick) {
                    return;
                }
                onClick();
            }}
        >
            <div className="flex flex-row gap-4">
                <FontAwesomeIcon
                    icon={faUser}
                    color="white"
                    className="pr-0.5 pt-1.5"
                />
                <div className="flex flex-col gap-1">
                    {account.fields.length === 0 ? (
                        <p className="italic">Empty Login Handle</p>
                    ) : null}
                    {account.fields.filter(
                        f => f.type === EBerytusFieldType.Identity ||
                            f.type === EBerytusFieldType.ForeignIdentity
                    ).map((field, i) => (
                        <p className={i !== 0 ? "text-sm" : ""}>{field.value}</p>
                    ))
                    }
                </div>
            </div>
        </div>
    );
}