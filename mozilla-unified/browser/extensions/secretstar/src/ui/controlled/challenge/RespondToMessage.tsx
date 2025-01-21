import { Session, SrpChallenge, db } from "@root/db";
import { useLiveQuery } from "dexie-react-hooks";
import { useState } from "react";
import { useParams } from "react-router-dom";
import Loading from "../../components/Loading";
import GetIdentityFieldsMessage from "./Identification/messages/GetIdentityFields/Message";
import { useAbortRequestOnWindowClose, useNavigateWithPopupContextAndPageContextRoute, useRequest, useSettings } from "@root/hooks";
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
import { ab2base64, bufToPem, formatSignatureBufToString } from "@root/key-utils";
import {
    EBerytusPasswordChallengeMessageName,
    EBerytusOffChannelOtpChallengeMessageName,
    EBerytusIdentificationChallengeMessageName,
    EBerytusDigitalSignatureChallengeMessageName,
    EBerytusSecureRemotePasswordChallengeMessageName,
    EBerytusChallengeType
} from "@berytus/types-extd";
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
            challenge,
            message: challenge.messages[messageId]
        };
    }, [sessionId, challengeId, messageId]);
    const request = query ?
        query.session.requests[query.session.requests.length - 1] :
        undefined;
    const { maybeResolve, maybeReject } = useRequest(request);
    const tabId = query?.session.context.document.id;
    useAbortRequestOnWindowClose({ maybeReject, tabId });
    const navigate = useNavigateWithPopupContextAndPageContextRoute(tabId);
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
                        maybeResolve(fieldValues) && navigate('/loading');
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
                        maybeResolve(fieldValues) && navigate('/loading');
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
                        const res = await db.sessions.update(session, {
                            [`challenges.${challenge.id}.srpState`]: {
                                fields: { username, password }
                            }
                        });
                        console.log("updated res:", res);
                        const newSession = await db.sessions.get(session.id);
                        console.log("new session:", newSession);
                        maybeResolve(username) && navigate('/loading');
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
                        maybeResolve(clientPublicKeyHexA) && navigate('/loading');
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
                        maybeResolve(clientProof) && navigate('/loading');
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
                        maybeResolve({}) && navigate('/loading');
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
                        maybeResolve(fieldValues) && navigate('/loading');
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
                        maybeResolve(publicKey) && navigate('/loading');
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
                        maybeResolve(signedMessage) && navigate('/loading');
                    }}
                />
        }
    }

    return <UnknownChallengeMessage
        challengeType={challenge.type}
        messageName={message.name}
    />;
}