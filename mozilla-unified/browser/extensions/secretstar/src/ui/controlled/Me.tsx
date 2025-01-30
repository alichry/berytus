import { useState, MutableRefObject, useEffect } from "react";
import MeView from "../components/MeView";
import { Identity, PRIMARY_IDENTITY_ID, db } from "@root/db";
import { useLiveQuery } from "dexie-react-hooks"
import Loading from "../components/Loading";
import { useIdentity, useSettings } from "@root/hooks";
import { stringifyUserAttributeValue } from "../components/AccountView";

export default function Me() {
    const [identityData, setIdentityData] = useState<Partial<Identity['userAttributes']>>();
    const [submitted, setSubmitted] = useState<boolean>(false);
    const settings = useSettings();
    const identity = useIdentity();
    useEffect(() => {
        if (identity) {
            // should we?
            setIdentityData({
                ...identityData,
                ...identity.userAttributes
            });
        }
    }, [identity]);
    if (! settings || ! identity || ! identityData) {
        return <Loading />
    }
    return (
        <>
            <MeView
                submitted={submitted}
                newIdentity={!identity?.userAttributes}
                getIdentityValue={(key) => (identityData[key]
                        ? stringifyUserAttributeValue(identityData[key]!)
                        : undefined)}
                setIdentityValue={(key, value) => {
                    // reset submission
                    setSubmitted(false);
                    setIdentityData({
                        ...identityData,
                        [key]: value
                    })
                }}
                saveIdentity={async () => {
                    const change: Pick<Identity, 'userAttributes'> = {
                        userAttributes: {
                            ...identity.userAttributes,
                            ...identityData as NonNullable<Identity['userAttributes']>
                        }
                    };
                    await db.identity.update(identity, change);
                    setSubmitted(true);
                }}
                settings={settings}
                onSettingChange={async (change) => {
                    return db.updateSettings(change);
                }}
                getEmail={() => identity?.emailAddress}
                getMobileNumber={() => identity?.mobileNumber}
                setEmail={async (email) => {
                    const change: Pick<Identity, 'emailAddress'> = {
                        emailAddress: email
                    };
                    await db.identity.update(identity, change);
                }}
                setMobileNumber={async (number) => {
                    const change: Pick<Identity, 'mobileNumber'> = {
                        mobileNumber: number
                    };
                    await db.identity.update(identity, change);
                }}
            />
        </>
    )
}