import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faChevronRight } from "@fortawesome/free-solid-svg-icons";
import { IconProp } from "@fortawesome/fontawesome-svg-core";
import React from "react";
import { ClassNameProp } from "../props/ClassNameProp";
import { cn } from "../utils/joinClassName";

export interface BreadcrumbsProps extends ClassNameProp {
    headerIcon: IconProp;
    locations: Array<string>;
    noMarginBottom?: boolean;

}
export default function Breadcrumbs({ locations, headerIcon, className, noMarginBottom }: BreadcrumbsProps) {
    return <div className={cn("flex flex-row gap-2 text-sm", ! noMarginBottom && 'mb-6', className)}>
        <FontAwesomeIcon
            icon={headerIcon}
            color="white"
            className="pr-0.5 self-center"
        />
        { locations.map((loc, i) =>
            <React.Fragment key={i}>
                <p>{loc}</p>
                {i !== locations.length - 1
                ? <FontAwesomeIcon
                    icon={faChevronRight}
                    //size="sm"
                    color="white"
                    className="self-center px-0.5"
                /> : null}
            </React.Fragment>)}
    </div>
}