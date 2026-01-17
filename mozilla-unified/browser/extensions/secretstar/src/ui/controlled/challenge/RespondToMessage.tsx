import { Session, SrpChallenge, db } from "@root/db";
import { useLiveQuery } from "dexie-react-hooks";
import { useCallback, useState } from "react";
import { useParams } from "react-router-dom";
import Loading from "../../components/Loading";
import GetIdentityFieldsMessage from "./Identification/messages/GetIdentityFields/Message";
import { useAbortRequestOnWindowClose, useCipherbox, useNavigateWithPageContextRoute, useRequest, useSettings } from "@root/hooks";
import UnknownChallengeMessage from "./UnknownChallengeMessage";
import ErrorPage from "@root/ui/components/ErrorPage";
import GetPasswordFieldsMessage from "./Password/messages/GetPasswordFields/Message";
import GetForeignIdentityOtpMessage from "./ForeignIdentityOtp/messages/GetOtp/Message";
import SelectSecurePasswordFieldMessage from "./SecureRemotePassword/messages/SelectSecurePasswordField/Message";
import ExchangePublicKeysMessage from "./SecureRemotePassword/messages/ExchangePublicKeys/Message";
import ComputeClientProofMessage from "./SecureRemotePassword/messages/ComputeClientProof/Message";
import VerifyServerProofMessage from "./SecureRemotePassword/messages/VerifyServerProof/Message";
import GetPublicKeyMessage from "./DigitalSignature/messages/GetPublicKey/Message";
import SignNonceMessage from "./DigitalSignature/messages/SignNonce/Message";
import { ab2base64, bufToPem, decodeHex, formatSignatureBufToString } from "@root/key-utils";
import {
    EBerytusPasswordChallengeMessageName,
    EBerytusOffChannelOtpChallengeMessageName,
    EBerytusIdentificationChallengeMessageName,
    EBerytusDigitalSignatureChallengeMessageName,
    EBerytusSecureRemotePasswordChallengeMessageName,
    EBerytusChallengeType,
    BerytusReceiveMessageUnion
} from "@berytus/types-extd";
import { RespondToChallengeMessageResult } from "@berytus/types";
import { InternalError } from "@root/errors/InternalError";
export interface RespondToMessageProps {
}

export default function RespondToMessage({}: RespondToMessageProps) {
    const { sessionId, challengeId, messageId } = useParams();
    const [error, setError] = useState<Error>();
    const query = useLiveQuery(async () => {
        if (
            sessionId === undefined ||
            challengeId === undefined ||
            messageId === undefined
        ) {
            return;
        }
        const record = await db.sessions.get(sessionId);
        if (! record) {
            setError(new Error('Session was not found!'));
            return;
        }
        const channel = await db.channel.get(record.channel.id);
        if (! channel) {
            setError(new Error('Channel not found!'));
            return;
        }
        if (! record.challenges) {
            setError(new Error('Session is missing a challenge dictionary! What?'));
            return;
        }
        if (! (challengeId in record.challenges)) {
            setError(new Error('Session is missing the needed challenge'));
            return;
        }
        const challenge = record.challenges[challengeId];
        if (! (messageId in challenge.messages)) {
            // message not added yet
            return;
        }

        return {
            session: record,
            channel,
            challenge,
            message: challenge.messages[messageId]
        };
    }, [sessionId, challengeId, messageId]);
    const request = query ?
        query.session.requests[query.session.requests.length - 1] :
        undefined;

    const tabId = query?.session.context.document.id;

    const navigate = useNavigateWithPageContextRoute();
    const { cipherbox, loading: cipherboxLoading } = useCipherbox(query?.channel);
    const preResolveCb = useCallback(async (value: RespondToChallengeMessageResult): Promise<RespondToChallengeMessageResult> => {
        if (cipherboxLoading) {
            throw new InternalError("Cipherbox not loaded in CreateField preResolve()");
        }
        if (! cipherbox) {
            return value; // e2e not enabled
        }
        return await cipherbox.encryptDictionary(value);
    }, [cipherboxLoading, cipherbox]);
    const onProcessed = useCallback(() => {
        navigate('/loading');
    }, [navigate]);
    const { maybeResolve, maybeReject } = useRequest<"AccountAuthentication_RespondToChallengeMessage">(
        request,
        { onProcessed, preResolve: preResolveCb }
    );
    useAbortRequestOnWindowClose({ maybeReject, tabId });
    const settings = useSettings();
    const loading = ! query ||
        maybeResolve === undefined ||
        maybeReject === undefined ||
        settings === undefined;

    if (error) {
        return <ErrorPage text={error.message} />
    }

    if (loading) {
        return <Loading />;
    }

    const submit = (value: BerytusReceiveMessageUnion) => {
        if (loading) {
            setError(new Error('Submit called before all the goods have been loaded'));
            return false;
        }
        const { sent } = maybeResolve(value);
        if (! sent) {
            setError(new Error("Unable to resolve request; this request has been already resolved."));
        }
    }

    const { session, challenge, message } = query;
    if (challenge.type === EBerytusChallengeType.Identification) {
        switch (message.name) {
            case EBerytusIdentificationChallengeMessageName.GetIdentityFields:
                return <GetIdentityFieldsMessage
                    session={session}
                    challenge={challenge}
                    message={message}
                    settings={settings}
                    onSubmit={(fieldValues) => {
                        submit({ response: fieldValues });
                    }}
                />;
        }
    }
    if (challenge.type === EBerytusChallengeType.Password) {
        switch (message.name) {
            case EBerytusPasswordChallengeMessageName.GetPasswordFields:
                return <GetPasswordFieldsMessage
                    session={session}
                    challenge={challenge}
                    message={message}
                    settings={settings}
                    onSubmit={(fieldValues) => {
                        submit({ response: fieldValues });
                    }}
                />;
        }
    }
    if (challenge.type === EBerytusChallengeType.SecureRemotePassword) {
        switch (message.name) {
            case EBerytusSecureRemotePasswordChallengeMessageName.SelectSecurePassword:
                return <SelectSecurePasswordFieldMessage
                    session={session}
                    challenge={challenge}
                    message={message}
                    settings={settings}
                    onSubmit={async ({ username, password }) => {
                        await db.sessions.update(session, {
                            [`challenges.${challenge.id}.srpState`]: {
                                fields: { username, password }
                            }
                        });
                        submit({ response: username });
                    }}
                />
            case EBerytusSecureRemotePasswordChallengeMessageName.ExchangePublicKeys:
                return <ExchangePublicKeysMessage
                    session={session}
                    challenge={challenge}
                    message={message}
                    settings={settings}
                    onSubmit={async (
                        serverPublicKeyHexB,
                        clientPublicKeyHexA,
                        clientPrivateKeyHexa
                    ) => {
                        // const ch = challenge as SrpChallenge;
                        // await db.sessions.update(session, {
                        //     challenges: {
                        //         ...session.challenges,
                        //         [challenge.id]: {
                        //             ...challenge,
                        //             srpState: {
                        //                 ...ch.srpState,
                        //                 clientPrivateKeyHexa,
                        //                 clientPublicKeyHexA,
                        //                 serverPublicKeyHexB
                        //             }
                        //         }
                        //     }
                        // });
                        await db.sessions.update(session, {
                            [`challenges.${challengeId}.srpState.clientPrivateKeyHexa`]: clientPrivateKeyHexa,
                            [`challenges.${challengeId}.srpState.clientPublicKeyHexA`]: clientPublicKeyHexA,
                            [`challenges.${challengeId}.srpState.serverPublicKeyHexB`]: serverPublicKeyHexB,
                        });

                        submit({
                            response: challenge.parameters.encoding === "Hex"
                                ? clientPublicKeyHexA
                                : decodeHex(clientPublicKeyHexA)
                        });
                    }}
                />;
            case EBerytusSecureRemotePasswordChallengeMessageName.ComputeClientProof:
                return <ComputeClientProofMessage
                    session={session}
                    challenge={challenge}
                    message={message}
                    settings={settings}
                    onSubmit={async (salt, clientProof) => {
                        await db.sessions.update(session, {
                            [`challenges.${challengeId}.srpState.salt`]: salt,
                            [`challenges.${challengeId}.srpState.clientProof`]: clientProof,
                        });
                        submit({
                            response: challenge.parameters.encoding === "Hex"
                                ? clientProof
                                : decodeHex(clientProof)
                        });
                    }}
                />;
            case EBerytusSecureRemotePasswordChallengeMessageName.VerifyServerProof:
                return <VerifyServerProofMessage
                    session={session}
                    challenge={challenge}
                    message={message}
                    settings={settings}
                    onSubmit={async (serverProofValid) => {
                        await db.sessions.update(session, {
                            [`challenges.${challengeId}.srpState.serverProofValid`]: serverProofValid,
                        });
                        submit({});
                    }}
                />
        }
    }
    if (challenge.type === EBerytusChallengeType.OffChannelOtp) {
        switch (message.name) {
            case EBerytusOffChannelOtpChallengeMessageName.GetOtp:
                return <GetForeignIdentityOtpMessage
                    session={session}
                    challenge={challenge}
                    message={message}
                    settings={settings}
                    onSubmit={(fieldValues) => {
                        submit({
                            response: fieldValues
                        })
                    }}
                />
        }
    }
    if (challenge.type === EBerytusChallengeType.DigitalSignature) {
        switch (message.name) {
            case EBerytusDigitalSignatureChallengeMessageName.SelectKey:
                return <GetPublicKeyMessage
                    session={session}
                    challenge={challenge}
                    message={message}
                    settings={settings}
                    onSubmit={async (privateKey, publicKey) => {
                        const pem = bufToPem(privateKey, false);
                        await db.sessions.update(session, {
                            [`challenges.${challengeId}.dsState.privateKey`]: pem,
                        });
                        submit({
                            response: {
                                publicKey
                            }
                        })
                    }}
                />;
            case EBerytusDigitalSignatureChallengeMessageName.SignNonce:
                return <SignNonceMessage
                session={session}
                    challenge={challenge}
                    message={message}
                    settings={settings}
                    onSubmit={async (nonce, signedMessage) => {
                        const pem = formatSignatureBufToString(signedMessage);
                        await db.sessions.update(session, {
                            [`challenges.${challengeId}.dsState.signedMessage`]: pem,
                            [`challenges.${challengeId}.dsState.nonce`]: ab2base64(nonce),
                        });
                        submit({
                            response: signedMessage
                        });
                    }}
                />
        }
    }

    return <UnknownChallengeMessage
        challengeType={challenge.type}
        messageName={message.name}
    />;
}