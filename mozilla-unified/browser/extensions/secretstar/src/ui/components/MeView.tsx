import { useState, createRef, RefObject } from "react";
import { Container } from "./Container";
import TopBar from "../controlled/TopBar";
import CommonBodyContainer from "./CommonBodyContainer";
import TabBar from "../controlled/TabBar";
import Breadcrumbs from "./Breadcrumbs";
import { faAddressCard } from "@fortawesome/free-regular-svg-icons"
import { userAttributesLabels } from "../utils/userAttributesLabels";
import SuperInput from "./SuperInput";
import TextArea from "./TextArea";
import type { UserAttributeKey } from "@berytus/types";
import SelectContent from "./SelectContent";
import Button from "./Button";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faCircleCheck } from "@fortawesome/free-regular-svg-icons";
import { Identity, PRIMARY_IDENTITY_ID, Settings, db, isUserAttributeKeyOptional } from "@root/db";
import LightSeparator from "./LightSeparator";
import Switch from "./Switch";
import { faCheck, faEnvelope, faPhone, faUser } from "@fortawesome/free-solid-svg-icons";
import AutoSuperInput from "./AutoSuperInput";
import Select from "./Select";
import zones from "@root/zoneinfo-list.json";
import locales from "@root/locale-list.json";
import PictureInput from "../controlled/PictureInput";
import { notEmpty, scrollTopSmooth } from "@root/utils";

export type SettingOption = "SeamlessSignUp" | "SeamlessLogin";

export interface MeViewProps {
    getIdentityValue(key: UserAttributeKey): string | undefined;
    setIdentityValue(key: UserAttributeKey, value: string | undefined): void;
    saveIdentity(): void;
    settings?: Settings;
    onSettingChange(change: Parameters<typeof db['updateSettings']>[0]): void;
    newIdentity?: boolean;
    submitted?: boolean;

    getEmail(): string | undefined;
    setEmail(value: string): void;
    getMobileNumber(): string | undefined;
    setMobileNumber(value: string): void;
}

const keys = Object.keys(userAttributesLabels) as Array<keyof typeof userAttributesLabels>;

export default function MeView({ getIdentityValue, setIdentityValue, saveIdentity, newIdentity, submitted, settings, onSettingChange, getEmail, setEmail, getMobileNumber, setMobileNumber }: MeViewProps) {
    // BRTTODO: make pure
    const [position, setPosition] = useState<number>(0);
    const [refs] = useState(() => {
        return (Object.keys(userAttributesLabels) as Array<keyof typeof userAttributesLabels>)
            .reduce((prev, curr) => {
                prev[curr] = createRef<HTMLInputElement & HTMLTextAreaElement & HTMLSelectElement>();
                return prev;
            }, {} as Record<UserAttributeKey, RefObject<HTMLInputElement & HTMLTextAreaElement & HTMLSelectElement>>);
    });

    const submit = () => {
        saveIdentity();
        scrollTopSmooth();
    }

    const validateField = (key: UserAttributeKey): boolean => {
        const ref = refs[key];
        // profile picture ref is null.
        if (ref.current === null || ref.current.validity.valid) {
            return true;
        }
        ref.current?.setAttribute('dirty', "");
        ref.current?.reportValidity();
        return false;
    }

    const selectContentEnabled = !!newIdentity;
    const items = keys.map((key, i) => (
        <div key={i} className={i !== keys.length - 1 ? "mb-4" : undefined}>
            <p className="mb-3 text-sm">
                {userAttributesLabels[key]}
                {!isUserAttributeKeyOptional(key) ? <span aria-label="required">*</span> : null}
            </p>
            {key === "address" ? (
                <TextArea
                    ref={refs[key]}
                    autoFocus={selectContentEnabled}
                    noMargin
                    defaultValue={getIdentityValue(key)}
                    onChange={(e) => setIdentityValue(key, e.target.value)}
                    placeholder={isUserAttributeKeyOptional(key) ? "Optional" : undefined}
                    spellCheck={false}
                    required
                />
            ) : key === "picture" ? (
                <PictureInput
                    onUpload={async (pictureId) => {
                        setIdentityValue('picture', pictureId.toString());
                    }}
                    onCancel={async () => {
                        setIdentityValue('picture', undefined);
                    }}
                    placeholder="Upload an avatar (optional)"
                    defaultPictureId={getIdentityValue('picture')}
                />
            ) : key === "zoneinfo" ? (
                <Select
                    ref={refs[key]}
                    className="w-full"
                    onChange={(e) => setIdentityValue(key, e.target.value)}
                    required
                    defaultValue={getIdentityValue(key) || ""}
                >
                    <option disabled value=""></option>
                    {(Object.values(zones))
                    .map((zone, i) => (
                        <option key={i} value={zone}>
                            {zone}
                        </option>
                    ))}
                </Select>
            ) : key === 'locale' ? (
                <Select
                    ref={refs[key]}
                    className="w-full"
                    onChange={(e) => setIdentityValue(key, e.target.value)}
                    required
                    defaultValue={getIdentityValue(key) || ""}
                >
                    <option disabled value=""></option>
                    {(Object.values(locales))
                    .map((locale, i) => (
                        <option key={i} value={locale}>
                            {locale}
                        </option>
                    ))}
                </Select>
            ) : (<SuperInput
                ref={refs[key]}
                type={
                    key === "birthdate"
                    ? "date"
                    : (key === "website" || key === "profile")
                    ? "url"
                    : "text"
                }
                autoFocus={selectContentEnabled}
                noMargin
                className="w-full"
                defaultValue={getIdentityValue(key)}
                onChange={(e) => setIdentityValue(key, e.target.value)}
                onKeyUp={(e) => {
                    if (
                        e.key === 'Enter' &&
                        e.shiftKey &&
                        position !== 0
                    ) {
                        setPosition(position - 1);
                        return;
                    }
                    if (
                        e.key === 'Enter' &&
                        validateField(key)
                    ) {
                        if (position === keys.length - 1) {
                            submit();
                        } else {
                            setPosition(position + 1);
                        }
                    }
                }}
                placeholder={
                    isUserAttributeKeyOptional(key) ? "Optional " : "" +
                        (key === "birthdate"
                            ? "YYYY-MM-DD"
                            : "")
                }
                required={!isUserAttributeKeyOptional(key)}
                title="This field is required"
            />)}
        </div>
    ));
    const saveButton = <Button color="blue" text="Save" onClick={() => {
        const keys = Object.keys(refs) as Array<keyof typeof refs>;
        for (const key of keys) {
            if (! validateField(key)) {
                return;
            }
        }
        submit();
    }} />;
    const identityContent = (
        <>
            {submitted ? (
                <div className="text-base border-b-[0.5px] border-gray-200 mb-4">
                    <FontAwesomeIcon
                        icon={faCircleCheck}
                        size="2x"
                        className="block mx-auto mb-4 text-teal-300"
                    />
                    <p className="text-base text-center mb-4 font-bold">Identity saved.</p>
                </div>
            ) : null}
            <p className="text-xs mb-4">{newIdentity ? "Please fill your identity information. " : ""}Secret* stores your information and serves it to web applications when requested by them and confirmed by you.</p>
            {!newIdentity ? (
                <>
                    {items}
                    <div className="flex flex-row mt-4">
                        <div className="flex-1"/>
                        {saveButton}
                    </div>
                </>
            ) : (
                <SelectContent
                    position={position}
                    setPosition={setPosition}
                    arrowLocation="bottom"
                    barClassName="mt-6"
                    enableProgressBar
                    submitButton={saveButton}
                    overrideDisableRight={!isUserAttributeKeyOptional(keys[position]) && !notEmpty(getIdentityValue(keys[position]))}
                    validateBeforeNext={() => {
                        const key = keys[position];
                        return validateField(key);
                    }}
                    items={items}
                />
            )}

        </>
    )
    return (
        <Container>
            <TopBar />
            <CommonBodyContainer>
                <Breadcrumbs
                    headerIcon={faAddressCard}
                    locations={["User preferrences"]}
                    className="text-base"
                />
                <TabBar
                    tabs={[
                        [
                            "Identity",
                            identityContent,
                        ],
                        [
                            "Settings",
                            <div className="text-sm">
                                <Switch
                                    text="Seamless onboarding"
                                    checked={settings?.seamless.signup}
                                    onChange={(e) => onSettingChange({
                                        "seamless.signup": e.target.checked,
                                        "seamless.login": e.target.checked
                                    })}
                                />
                                { settings?.seamless.signup ? (
                                    <>
                                        <p className="text-sm mb-4">To fully enable seamless onboarding, please provide a valid email address and mobile number. Otherwise, you will be prompted to enter one when necessary.</p>
                                        <AutoSuperInput
                                            type="email"
                                            inputIcon={<FontAwesomeIcon icon={faEnvelope} />}
                                            placeholder="Email address"
                                            className="mb-0"
                                            onChange={(e) => {
                                                if (e.target.validity.valid) {
                                                    setEmail(e.target.value);
                                                    return;
                                                }
                                            }}
                                            defaultValue={getEmail()}
                                        />
                                        <AutoSuperInput
                                            type="tel"
                                            pattern="^\+[0-9]{1,3}[0-9]{1}[0-9]*$"
                                            className="mt-4"
                                            inputIcon={<FontAwesomeIcon icon={faPhone} />}
                                            placeholder="Phone number"
                                            onChange={(e) => {
                                                if (e.target.validity.valid) {
                                                    setMobileNumber(e.target.value);
                                                    return;
                                                }
                                            }}
                                            defaultValue={getMobileNumber()}
                                        />
                                        {/* <LightSeparator /> */}
                                    </>
                                ) : null}
                                {/* <Switch text="Seamless Login" /> */}
                                <div className="mb-5 pb-2 border-b-[0.5px] border-gray-200">
                                    DevTools
                                </div>
                                <Button color="red" text="Use Development Identity" onClick={async () => {
                                    const change: Pick<Identity, 'userAttributes'> = {
                                        userAttributes: dummyAttributes
                                    };
                                    await db.identity.update(PRIMARY_IDENTITY_ID, change);
                                }} />
                            </div>
                        ]
                    ]}
                />
            </CommonBodyContainer>

        </Container>
    )
}

const dummyAttributes = {
    name: {
        id: "name",
        mimeType: "text/plain",
        value: "John Bobby"
    },
    givenName: {
        id: "givenName",
        mimeType: "text/plain",
        value: "John"
    },
    middleName: {
        id: "middleName",
        mimeType: "text/plain",
        value: "A."
    },
    familyName: {
        id: "familyName",
        mimeType: "text/plain",
        value: "Bobby"
    },
    address: {
        id: "address",
        mimeType: "text/plain",
        value: "12 Atlantic road"
    },
    birthdate: {
        id: "birthdate",
        mimeType: "text/plain",
        value: "1970-12-12"
    },
    gender: {
        id: "gender",
        mimeType: "text/plain",
        value: "Male"
    },
    locale: {
        id: "locale",
        mimeType: "text/plain",
        value: "en_US"
    },
    nickname: {
        id: "nickname",
        mimeType: "text/plain",
        value: "Bobby"
    },
    //picture: "", // string must be a pictureId
    profile: {
        id: "profile",
        mimeType: "text/plain",
        value: "https://bob.site/me"
    },
    website: {
        id: "website",
        mimeType: "text/plain",
        value: "https://blog.bob.site/"
    },
    zoneinfo: {
        id: "zoneinfo",
        mimeType: "text/plain",
        value: "Europe/London"
    }
} as const;