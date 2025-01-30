import styles from '@styles/IntentContextView.module.scss';
import { useState } from "react";
import { cn } from "@root/ui/utils/joinClassName";
import KeyValue from "./KeyValue";
import { ClassNameProp } from "../props/ClassNameProp";
import type { UriParams } from "@berytus/types";

export interface IntentContextViewProps extends ClassNameProp {
    uri: UriParams;
}

export default function IntentContextView(props: IntentContextViewProps) {
    return <div className={cn(styles.container, props.className)}>
        <p className="text-center text-base mt-2 mb-6">Onboarding Session</p>
        <KeyValue
            className="text-sm"
            valueClassName="font-value"
            rows={[
                [<p>Domain</p>, <p>{props.uri.hostname}</p>]
            ]}
        />
    </div>
}