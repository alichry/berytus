import { useCallback, useEffect, useState } from 'react';
import { Field, FieldValueRejection, Session, db } from "@root/db/db";
import { useRequest, useAbortRequestOnWindowClose, useNavigateWithPageContextRoute, useSettings, useIdentity, useCipherbox } from "@root/hooks";
import { useLiveQuery } from "dexie-react-hooks";
import { useParams } from "react-router-dom";
import Loading from "@components/Loading";
import CreateFieldView from "../components/CreateFieldView";
import JsrpClient from '../../JsrpClient';
import { ab2base64, ab2str, base64ToArrayBuffer, formatBase64AsPem, pemToBuf, privateKeyBufToPublicKeyBuf, str2ab } from "@root/key-utils";
import { randomFieldValue } from '@root/utils';
import type { FieldInfo } from '@berytus/types';
import { BerytusFieldValueUnion, BerytusForeignIdentityFieldOptions, BerytusSecurePasswordFieldOptions } from "@berytus/types-extd";
import { EBerytusFieldType, ERejectionCode } from "@berytus/enums";

const isSecurePasswordOptions = (
    opts: FieldInfo['options']
): opts is BerytusSecurePasswordFieldOptions => {
    return "salt" in opts;
}

export interface CreateFieldProps {
    rejected?: boolean;
}

async function generateRsaPrivateKeyBuf(): Promise<ArrayBuffer> {
    const genKey = await crypto.subtle.generateKey(
        {
            name: "RSA-OAEP",
            modulusLength: 4096,
            publicExponent: new Uint8Array([1, 0, 1]),
            hash: "SHA-256",
        },
        true,
        ["encrypt", "decrypt", "sign"]
    );
    const privKeyBuf = await window.crypto.subtle.exportKey("pkcs8", genKey.privateKey);
    return privKeyBuf;
}

function canGenerateValue(field: FieldInfo, dictatedValue: boolean): boolean {
    return field.type !== EBerytusFieldType.ForeignIdentity && !dictatedValue;
}

export default function CreateField({ rejected }: CreateFieldProps) {
    const settings = useSettings();
    const identity = useIdentity();
    const { sessionId, fieldId } = useParams<string>();
    const [error, setError] = useState<Error | undefined>();
    const query = useLiveQuery(
        async () => {
            if (! sessionId || ! fieldId) {
                return;
            }
            const record = await db.sessions.get(sessionId);
            if (! record || ! record.createFieldOptions) {
                return;
            }
            if (! record.createFieldOptions.find(f => f.id === fieldId)) {
                return;
            }
            const channel = await db.channel.get(record.channel.id);
            if (! channel) {
                setError(new Error('Channel not found!'));
                return;
            }
            return {
                session: record,
                channel
            };
        }
    );
    const { session, channel } = query || {};
    const tabId = session?.context.document.id;
    const navigate = useNavigateWithPageContextRoute();
    const [isGenerating, setIsGenerating] = useState<boolean>(false);
    const [value, setValue] = useState<Uint8Array>(new Uint8Array());
    const onProcessed = useCallback(() => {
        navigate('/loading');
    }, [navigate]);
    const { cipherbox, loading: cipherboxLoading } = useCipherbox(channel);
    const { maybeResolve, maybeReject } = useRequest<"AccountCreation_AddField">(
        session?.requests[session?.requests.length - 1],
        { onProcessed, cipherbox }
    );
    useAbortRequestOnWindowClose({ maybeReject, tabId });
    const field = session?.createFieldOptions?.find(f => f.id === fieldId);
    const rejection: FieldValueRejection | undefined =
        session?.rejectedFieldValues?.find(f => f.fieldId === fieldId);
    const [generateValue, setGenerateValue] = useState<null | (() => Promise<Uint8Array>)>();
    const [seamlessTried, setSeamlessTried] = useState<boolean>(false);
    useEffect(() => {
        if (! field || (rejected && ! rejection)) {
            return;
        }
        if (! canGenerateValue(field, rejection?.webAppDictatedValue !== undefined)) {
            setGenerateValue(null);
            return;
        }
        const fn = async () => {
            const generated = await randomFieldValue(field.type);
            let value: Uint8Array;
            if (generated instanceof ArrayBuffer) {
                value = new Uint8Array(generated)
            } else {
                value = new Uint8Array(str2ab(generated));
            }
            return value;
        };
        setGenerateValue(() => fn);
    }, [field, rejected, rejection]);
    const loaded = session && !cipherboxLoading && (!rejected || rejection) && settings && maybeReject && maybeResolve && identity && field && (generateValue !== undefined);

    useEffect(() => {
        if (! rejection?.webAppDictatedValue) {
            return;
        }
        setValue(new Uint8Array(str2ab(rejection.webAppDictatedValue)));
    }, [rejection]);

    // BRTTODO: Refactor this, it is too bloated.
    const submit = async (value: Uint8Array): Promise<boolean> => {
        if (! loaded) {
            setError(new Error('Submit called before all the goods have been loaded'));
            return false;
        }
        let resolveWith: BerytusFieldValueUnion | null;
        let newField: Field;
        if (field.type === EBerytusFieldType.Key) {
            const privateKeyBase64 = ab2base64(value.buffer); // we store this
            const publicKeyBuf = await privateKeyBufToPublicKeyBuf(value.buffer); // we resolve with this
            newField = {
                ...field,
                value: privateKeyBase64
            };
            resolveWith = {
                publicKey: publicKeyBuf
            };
        } else if (isSecurePasswordOptions(field.options)) {
            const client = new JsrpClient();
            const { identityFieldId } = field.options;
            const identityField =
                (session.fields || [])
                    .find(({ id }) =>
                        id === identityFieldId
                    );
            if (! identityField) {
                setError(new Error("CreateField error! passed identityFieldId " + identityFieldId + " was not found!"));
                maybeReject(ERejectionCode.GeneralError);
                return false;
            }
            if (
                identityField.type !== EBerytusFieldType.Identity &&
                identityField.type !== EBerytusFieldType.ForeignIdentity
            ) {
                setError(new Error("CreateField error! passed identityFieldId " + identityFieldId + " has an invalid field type "+ identityField.type));
                maybeReject(ERejectionCode.GeneralError);
                return false;
            }
            if (typeof identityField.value !== "string") {
                setError(new Error("CreateField error! passed identityFieldId " + identityFieldId + " has an invalid value type "+ typeof identityField.value));
                maybeReject(ERejectionCode.GeneralError);
                return false;
            }
            const v = ab2str(value.buffer);
            await client.init({ username: identityField.value, password: v });
            const { salt, verifier } = await client.createVerifier();
            newField = {
                ...field,
                value: v
            };
            resolveWith = {
                salt: base64ToArrayBuffer(salt),
                verifier: base64ToArrayBuffer(verifier)
            };
        } else {
            const v = ab2str(value.buffer);
            newField = {
                ...field,
                value: v
            };
            resolveWith = v;
        }
        let change: Pick<Session, 'fields'>;
        if (rejected && rejection) {
            if (rejection.previousValues.indexOf(newField.value) !== -1) {
                // cannot resolve with same value!
                setError(new Error("The value you have entered has been previously rejected! Please enter an alternative value."));
                return false;
            }
            const existingField = session.fields?.find(f => f.id === field.id);
            if (! existingField) {
                setError(new Error('Unable to find existing field!'));
                return false;
            }
            existingField.options = newField.options;
            existingField.value = newField.value;
            change = {
                fields: session.fields
            };
            if (
                rejection.webAppDictatedValue
            ) {
                // Berytus API specifies that when a dictated value
                // is provided, we should resolve with undefined.
                resolveWith = null;
            }
        } else {
            change = {
                fields: (session.fields || []).concat(newField)
            };
        }
        await db.sessions.update(session.id, change);
        const { sent } = maybeResolve(resolveWith);
        if (! sent) {
            setError(new Error('Unable to resolve. The request has been resolved previously.'));
            return false;
        }
        return true;
    }

    useEffect(() => {
        if (! loaded) {
            return;
        }
        const run = async (): Promise<boolean> => {
            if (settings.seamless.signup) {
                /**
                 * if null, we cannot generate a value and we do not have one saved.
                 * If undefined, the fieldType is not a ForeignIdentity type and
                 * we can
                 * if string, we have a saved value and the fieldType is a
                 * ForeignIdentity with a known ForeignIdentityKind
                 */
                const savedValue = field.type === EBerytusFieldType.ForeignIdentity
                    ? (field.options as BerytusForeignIdentityFieldOptions).kind === "EmailAddress"
                        ? identity.emailAddress
                        : (field.options as BerytusForeignIdentityFieldOptions).kind === "PhoneNumber"
                        ? identity.mobileNumber
                        : null
                    : undefined;
                if (rejected) {
                    if (rejection?.webAppDictatedValue) {
                        return submit(new Uint8Array(str2ab(rejection.webAppDictatedValue)));
                    }
                    if (
                        typeof savedValue === "string" &&
                        savedValue.length > 0 &&
                        rejection?.previousValues.indexOf(savedValue) === -1
                    ) {
                        return submit(new Uint8Array(str2ab(savedValue)));
                    }
                } else if (typeof savedValue === "string" && savedValue.length > 0) {
                    return submit(new Uint8Array(str2ab(savedValue)));
                }
                if (generateValue) {
                    return generateValue().then(value => {
                        return submit(value)
                    });
                }
            }
            return false;
        }
        run()
            .then((submitted) => {
                if (! submitted) {
                    setSeamlessTried(true);
                }
            })
            .catch(e => {
                setSeamlessTried(true);
                setError(e);
            });
    }, [loaded]);
    if (! loaded || ! seamlessTried) {
        // BRTTODO: Put an error since the session record should exist by now.
        // Or is that the useLiveQuery is still fetching obj...
        // Maybe use a different hook.
        return <Loading />
    }
    if (! session.createFieldOptions?.length) {
        return <p className="text-lg red text-center mt-4">Where createFieldOptions?</p>
    }
    return <CreateFieldView
        page={session.context.document}
        field={field}
        onCancel={() => window.close()}
        value={value}
        previouslyRejected={rejected ? rejection : undefined}
        toString={() => {
            if (field.type === EBerytusFieldType.Key) {
                if (value.length === 0) {
                    return "";
                }
                return formatBase64AsPem(ab2base64(value.buffer), false);
            }
            return new TextDecoder().decode(value);
        }}
        error={error}
        isEmpty={value.length === 0}
        generateAndSetValue={generateValue ? async () => {
            setIsGenerating(true);
            const value = await generateValue();
            setValue(value);
            setIsGenerating(false);
        } : undefined}
        isGenerating={isGenerating}
        setValue={(str) => {
            if (rejection?.webAppDictatedValue) {
                throw new Error('Why are you changing the value despite it being dictated?')
            }
            if (field.type === EBerytusFieldType.Key) {
                // parse the PEM and save it as buffer
                let buf: ArrayBufferLike;
                try {
                    buf = pemToBuf(str);
                } catch (e) {
                    setError(e as Error);
                    return;
                }
                setValue(new Uint8Array(buf));
                return;
            }
            setValue(new Uint8Array(str2ab(str)))
        }}
        onSubmit={async () => submit(value)}
    />;
}