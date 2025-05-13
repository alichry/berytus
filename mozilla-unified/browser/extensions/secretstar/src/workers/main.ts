import { Session, db } from "@root/db/db";
import { MODE, MODE_EXTERNAL_WINDOW } from "../env";
import { openWindow } from "./window-utils";
import { getSessionRecord, openWindowOrRedirectTab } from "./session-utils";
import { url } from "./paths";
import { randomPassword } from "@root/utils";
import { PAGECONTEXT_POPUP } from "@root/pagecontext";
import { ab2base64, base64ToArrayBuffer } from "@root/key-utils";
import { Channel, isChannelE2EReady } from "@root/db/Channel";
import { ERejectionCode, EOperationType, EMetadataStatus, RequestType } from "@berytus/enums";
import type { KeyAgreementParameters, PreliminaryRequestContext } from "@berytus/types";
import { userAttributesLabels } from "@root/ui/utils/userAttributesLabels";
import { toClearFieldValue } from "./field-utils";
import { openPageActionPopupIfNecessary } from "./pageAction-popup-fix";

console.debug("secretstar(bg): loaded");

const plainContext = (context: PreliminaryRequestContext) => {
    return {
        ...context,
        response: undefined,
    }
}

async function printSessionForSimiluationPrep(
    phase: RequestType | `${RequestType}:${string}`,
    sessionId: string
) {
    // const session = await getSessionRecord(sessionId);
    // console.log('simulate:', JSON.stringify({
    //     phase, session
    // }, null, 2));
}

async function printSessionWithChannelForSimulationPrep(
    phase: RequestType,
    sessionId: string
) {
    // const session = await getSessionRecord(sessionId);
    // const channel = await db.channel.get(session.channel.id);
    // if (! channel) {
    //     throw new Error('Cant find channel to print simulation prep');
    // }
    // console.log('simulate', phase, JSON.stringify({
    //     session,
    //     channel
    // }, null, 2));
}

async function showUi(
    context: PreliminaryRequestContext,
    sessionId: string,
    relativePath: string,
    sessionRecordPromise?: ReturnType<typeof getSessionRecord>
) {
    if (MODE === MODE_EXTERNAL_WINDOW) {
        const { tabId } = await (sessionRecordPromise || getSessionRecord(sessionId));
        if (! tabId) {
            console.error('Secret* is in External Mode but session record had no tab id');
            browser.berytus.rejectRequest(context.request.id, ERejectionCode.GeneralError);
            return;
        }
        await openWindowOrRedirectTab(sessionId, tabId, relativePath);
        return;
    }
    openPageActionPopupIfNecessary(
        { requestId: context.request.id, tabId: context.document.id },
        url(relativePath, PAGECONTEXT_POPUP)
    );
}

browser.berytus.registerRequestHandler({
    manager: {
        async getSigningKey(context, args): Promise<void> {
            const keys = await db.signingKeys.toArray();
            if (keys.length > 0) {
                context.response.resolve(keys[0].public);
                return;
            }
            const keyPair = await window.crypto.subtle.generateKey(
                "Ed25519",
                true,
                ['sign', 'verify']
            ) as CryptoKeyPair;
            const publicKey = ab2base64(
                await window.crypto.subtle.exportKey("spki", keyPair.publicKey)
            );
            await db.signingKeys.add({
                public: publicKey,
                private: await window.crypto.subtle.exportKey("pkcs8", keyPair.privateKey)
            })
            context.response.resolve(publicKey);
        },
        async getCredentialsMetadata(context, args): Promise<void> {
            const accounts = db.findCorrespondingAccounts(
                args.webAppActor,
                args.accountConstraints.category,
                args.accountConstraints.schemaVersion
            );
            context.response.resolve(await accounts.count());
        },
    },
    channel: {
        async createChannel(context, args) {
            const key = await db.signingKeys.get(args.channel.scmActor.ed25519Key);
            if (! key) {
                throw new Error(
                    'Unable to find signing key '
                    + args.channel.scmActor.ed25519Key
                );
            }
            await db.channel.add({
                id: args.channel.id,
                scmEd25519: key,
                webAppEd25519Pub: "ed25519Key" in args.channel.webAppActor
                    ? args.channel.webAppActor.ed25519Key
                    : undefined,
                e2eeActvie: false
            });
            context.response.resolve();
        },
        async generateX25519Key(context) {
            const channel = await db.channel.get(context.channel.id);
            if (! channel) {
                throw new Error('Unable to find channel by id ' + context.channel.id);
            }
            if (channel.scmX25519) {
                throw new Error("Expecting our scm x25519 key to be unset; got otherwise.");
            }
            const scmX25519Key = await window.crypto.subtle.generateKey(
                "X25519",
                true,
                ["deriveKey"],
            ) as CryptoKeyPair;
            const b64Spki = ab2base64(
                await window.crypto.subtle.exportKey("spki", scmX25519Key.publicKey)
            );
            const change: Pick<Channel, 'scmX25519'> = {
                scmX25519: {
                    private: await window.crypto.subtle.exportKey("pkcs8", scmX25519Key.privateKey),
                    public: b64Spki
                }
            };
            await db.channel.update(channel, change);
            context.response.resolve({
                public: b64Spki
            });
        },
        async verifySignedKeyExchangeParameters(context, args): Promise<void> {
            const channel = await db.channel.get(context.channel.id);
            if (! channel) {
                throw new Error('Unable to find channel by id ' + context.channel.id);
            }
            if (! channel.scmEd25519) {
                throw new Error('Expecting our scm Ed25519 to be set; got otherwise.');
            }
            if (! channel.scmX25519) {
                throw new Error('Expecting our scm X25519 to be set; got otherwise.');
            }
            if (channel.keyAgreement) {
                throw new Error('Expecting key agreement parameters to be unset; got otherwise.');
            }
            if (! channel.webAppEd25519Pub) {
                throw new Error('Expecting channel to have a crypto web app actor set; got otherwise.');
            }
            const parameters: KeyAgreementParameters = (() => {
                const obj = JSON.parse(args.canonicalJson);
                obj.session.fingerprint.salt =
                    new Uint8Array(obj.session.fingerprint.salt).buffer;
                obj.session.fingerprint.value =
                    new Uint8Array(obj.session.fingerprint.value).buffer;
                obj.derivation.salt =
                    new Uint8Array(obj.derivation.salt).buffer;
                obj.derivation.info =
                    new Uint8Array(obj.derivation.info).buffer;
                return obj;
            })();
            if (parameters.authentication.public.scm !== channel.scmEd25519.public) {
                throw new Error("Crypto scm actor mismatch");
            }
            if (parameters.authentication.public.webApp !== channel.webAppEd25519Pub) {
                throw new Error("Crypto web app actor mismatch");
            }
            if (parameters.exchange.public.scm !== channel.scmX25519.public) {

                throw new Error("Scm X25519 mismatch");
            }
            const webAppKey = await crypto.subtle.importKey(
                'spki',
                base64ToArrayBuffer(parameters.authentication.public.webApp),
                'Ed25519',
                false,
                ['verify']
            );
            const data = new TextEncoder().encode(args.canonicalJson);
            const valid = await crypto.subtle.verify(
                'Ed25519',
                webAppKey,
                args.webAppSignature,
                data
            );
            if (! valid) {
                context.response.reject(ERejectionCode.GeneralError);
                return;
            }
            const change: Pick<Channel, 'keyAgreement'> = {
                keyAgreement: {
                    parameters,
                    signatures: {
                        canonicalJson: args.canonicalJson,
                        webAppSignature: args.webAppSignature
                    },
                }
            };
            await db.channel.update(channel, change);
            context.response.resolve();
        },
        async signKeyExchangeParameters(context, args) {
            const channel = await db.channel.get(context.channel.id);
            if (! channel) {
                throw new Error('Unable to find channel by id ' + context.channel.id);
            }
            if (! channel.scmEd25519) {
                throw new Error('Expecting our scm ed25519Key to have been genenrated');
            }
            if (! channel.scmX25519) {
                throw new Error('Expecting our scm scmX25519 to have been genenrated');
            }
            if (! channel.webAppEd25519Pub) {
                throw new Error('Expecting web app ed25519 to have been given previously');
            }
            if (! channel.keyAgreement) {
                throw new Error('Expecting key agreement parameters to be set; got otherwise.');
            }
            if (channel.keyAgreement.signatures.scmSignature) {
                throw new Error('Expecting our scm signature to be unset; got otherwise.');
            }
            if (channel.keyAgreement.signatures.canonicalJson !== args.canonicalJson) {
                throw new Error('Expecting our stored key agreement JSON to match what was provided; got otherwise.');
            }
            const ed25519Key = await window.crypto.subtle.importKey(
                'pkcs8',
                channel.scmEd25519.private,
                "Ed25519",
                false,
                ['sign']
            );
            const scmSignature = await window.crypto.subtle.sign(
                "Ed25519",
                ed25519Key,
                new TextEncoder().encode(args.canonicalJson)
            );
            await db.channel.update(channel, {
                'keyAgreement.signatures.scmSignature': scmSignature
            });
            context.response.resolve({
                scmSignature
            });
        },
        async enableEndToEndEncryption(context): Promise<void> {
            // TODO(berytus): It seems showUi depends on a session id
            // (the operation id) retrieved from approveOperation.
            // Once we refactor showUi to accept a channel id instead,
            // we can consider showing a UI fo enableE2EE
            // let window: chrome.windows.Window | undefined = undefined;
            // const relativePath = `enable-e2e/${context.channel.id}/${context.request.id}/${context.request.type}`;
            // if (MODE === MODE_EXTERNAL_WINDOW) {
            //     window = await openWindow(relativePath);
            // } else {
            //     openPageActionPopupIfNecessary(
            //         { requestId: context.request.id, tabId: context.document.id },
            //         url(relativePath, PAGECONTEXT_POPUP)
            //     );
            // }
            const channel = await db.channel.get(context.channel.id);
            if (! channel) {
                throw new Error('Expecting channel to be created; got otherwise.');
            }
            if (! isChannelE2EReady(channel)) {
                throw new Error('Expecting channel to be E2E ready; got otherwise.');
            }
            const webAppKey = await crypto.subtle.importKey(
                'spki',
                // @ts-ignore: NOTE(berytus): Firefox only
                Uint8Array.fromBase64(channel.keyAgreement.parameters.exchange.public.webApp),
                'X25519',
                false,
                []
            );
            const scmKey = await window.crypto.subtle.importKey(
                'pkcs8',
                channel.scmX25519.private,
                "X25519",
                false,
                ['deriveKey']
            );
            const sharedKey = await crypto.subtle.deriveKey(
                {
                    name: "X25519",
                    public: webAppKey
                },
                scmKey,
                'HKDF',
                false,
                ['deriveKey']
            );
            const encryptionKey = await crypto.subtle.deriveKey(
                channel.keyAgreement.parameters.derivation,
                sharedKey,
                channel.keyAgreement.parameters.generation,
                true,
                ['encrypt', 'decrypt']
            );
            const change: Pick<Channel, 'e2eeActvie' | 'e2eeKey'> = {
                e2eeActvie: true,
                e2eeKey: encryptionKey
            };
            await db.channel.update(channel.id, change);
            context.response.resolve();
        },
        closeChannel: function (context): void {
            context.response.resolve();
        }
    },
    login: {
        async approveOperation(context, args): Promise<void> {
            let window: chrome.windows.Window | undefined = undefined;
            const relativePath = `intent/${args.operation.id}/0`;
            if (MODE === MODE_EXTERNAL_WINDOW) {
                window = await openWindow(relativePath);
            } else {
                openPageActionPopupIfNecessary(
                    { requestId: context.request.id, tabId: context.document.id },
                    url(relativePath, PAGECONTEXT_POPUP)
                );
            }
            await db.sessions.add({
                id: args.operation.id,
                requests: [context.request],
                channel: context.channel,
                operation: args.operation,
                context: plainContext(context),
                tabId: window?.tabs ? window.tabs[0].id : undefined,
                metadata: {
                    version: context.channel.constraints.account?.schemaVersion || 0,
                    status: EMetadataStatus.Pending,
                    category: context.channel.constraints.account?.category || "",
                    changePassUrl: ""
                },
                version: 1
            });
            printSessionForSimiluationPrep('Login_ApproveOperation', args.operation.id);
        },
        async closeOperation(context): Promise<void> {
            const accountId = randomPassword();
            const sessionId = context.operation.id;
            const sessionRecordPromise = getSessionRecord(sessionId);
            if (context.operation.type === EOperationType.Registration) {
                const relativePath = `save/${sessionId}/${accountId}`;
                await showUi(context, sessionId, relativePath, sessionRecordPromise);
                const sessionRecord = await sessionRecordPromise;
                const newAccountId = await db.accounts.add({
                    id: accountId,
                    date: new Date().toISOString(),
                    registrationUri: context.document.uri,
                    fields: sessionRecord.fields || [],
                    webAppActor: context.channel.webAppActor,
                    userAttributes: sessionRecord.userAttributes || {},
                    metadata: sessionRecord.metadata
                });
                const change: Pick<Session, 'createdAccountId' | 'requests' | 'version' | 'closed'> = {
                    createdAccountId: newAccountId as string, // BRTTODO: Perhaps change ids from strings to IndexableType for all references.
                    requests: sessionRecord.requests.concat(context.request),
                    version: sessionRecord.version + 1,
                    closed: true
                };
                await db.sessions.update(sessionId, change);
            } else {
                const relativePath = `auth/${sessionId}/finished`;
                await showUi(context, sessionId, relativePath, sessionRecordPromise);

                const sessionRecord = await sessionRecordPromise;
                const change: Pick<Session, 'requests' | 'version' | 'closed'> = {
                    requests: sessionRecord.requests.concat(context.request),
                    version: sessionRecord.version + 1,
                    closed: true
                };
                await db.sessions.update(sessionId, change);
            }
            printSessionForSimiluationPrep("Login_CloseOperation", sessionId);
        },
        async getRecordMetadata(context): Promise<void> {
            const sessionId = context.operation.id;
            const sessionRecord = await getSessionRecord(sessionId);
            if (context.operation.type === EOperationType.Registration) {
                context.response.resolve(sessionRecord.metadata);
                return;
            }
            const account = await db.accounts.get(sessionRecord.selectedAccountId!);
            if (! account) {
                throw new Error('unable to get account record, the account id does not exist.');
            }
            context.response.resolve(account.metadata);
        },
        async updateMetadata(context, args): Promise<void> {
            const sessionId = context.operation.id;
            const sessionRecord = await getSessionRecord(sessionId);
            const change: Pick<Session, 'metadata' | 'version'> = {
                metadata: {
                    ...sessionRecord.metadata,
                    ...args.metadata
                },
                version: sessionRecord.version + 1
            };
            await db.sessions.update(sessionRecord, change);
            context.response.resolve();
        }
    },
    accountCreation: {
        async approveTransitionToAuthOp(context, args): Promise<void> {
            const currSessionId = context.operation.id;
            const nextSessionId = args.newAuthOp.id;
            const currSessionRecordPromise = getSessionRecord(currSessionId);
            const relativePath = `transition-to-auth-op/${currSessionId}/${nextSessionId}`;
            await showUi(context, currSessionId, relativePath, currSessionRecordPromise);

            const currSession = await currSessionRecordPromise;
            if (! currSession.createdAccountId) {
                throw new Error("Expecting createdAccountId to be set when transitioning!");
            }
            const account = await db.accounts.get(currSession.createdAccountId);
            if (! account) {
                throw new Error("Expecting account to be created before transitioning.");
            }
            await db.sessions.add({
                id: nextSessionId,
                channel: context.channel,
                operation: args.newAuthOp,
                requests: [context.request],
                transitionedFromSessionId: currSessionId,
                context: plainContext(context),
                tabId: currSession.tabId, // window tab id. not the content page tab id.
                selectedAccountId: currSession.createdAccountId,
                metadata: account.metadata,
                version: 1,
            });
            printSessionForSimiluationPrep('AccountCreation_ApproveTransitionToAuthOp', nextSessionId);
        },
        async getUserAttributes(context): Promise<void> {
            const sessionId = context.operation.id;
            const sessionRecordPromise = getSessionRecord(sessionId);
            {/* BRTTODO: 23/1/2024 Remove afterVersion -- this seems to be problematic. We should refactor how we process requests to use request id instead */}
            const relativePath = `get-user-attributes/${sessionId}/1`;
            await showUi(context, sessionId, relativePath, sessionRecordPromise);
            const sessionRecord = await sessionRecordPromise;
            const requiredUserAttributes: Partial<Record<keyof typeof userAttributesLabels, boolean>> = {};
            (Object.keys(userAttributesLabels) as Array<keyof typeof userAttributesLabels>)
                .map(key => {
                    requiredUserAttributes[key] = false;
                });
            context.operation.requestedUserAttributes.forEach(({ id, required }) => {
                requiredUserAttributes[id] = required;
            });
            const change: Pick<Session, 'requests' | 'requiredUserAttributes' | 'version'> = {
                requiredUserAttributes: requiredUserAttributes as Record<keyof typeof userAttributesLabels, boolean>,
                requests: sessionRecord.requests.concat(context.request),
                version: 2
            };
            await db.sessions.update(sessionId, change);
            printSessionForSimiluationPrep('AccountCreation_GetUserAttributes', sessionId);
        },
        updateUserAttributes(context, args): void {
            throw new Error("Function not implemented.");
        },
        async addField(context, args): Promise<void> {
            const sessionId = context.operation.id;
            const sessionRecordPromise = getSessionRecord(sessionId);
            if (args.field.value === null) {
                const relativePath = `create-field/${sessionId}/${args.field.id}`;
                await showUi(context, sessionId, relativePath, sessionRecordPromise);
                const sessionRecord = await sessionRecordPromise;
                if (
                    sessionRecord.createFieldOptions &&
                    sessionRecord.createFieldOptions.find(o => o.id === args.field.id)
                ) {
                    browser.berytus.rejectRequest(context.request.id, ERejectionCode.GeneralError);
                    return;
                }
                const change: Pick<Session, 'requests' | 'createFieldOptions' | 'version'> = {
                    requests: sessionRecord.requests.concat(context.request),
                    createFieldOptions: (sessionRecord.createFieldOptions || []).concat({
                        id: args.field.id,
                        type: args.field.type,
                        options: args.field.options
                    }),
                    version: sessionRecord.version + 1
                };
                await db.sessions.update(sessionId, change);
                printSessionForSimiluationPrep('AccountCreation_AddField', sessionId);
                return;
            }
            const relativePath = `put-field/${sessionId}/${args.field.id}`;
            await showUi(context, sessionId, relativePath, sessionRecordPromise);
            const sessionRecord = await sessionRecordPromise;
            if (
                sessionRecord.putFields?.find(o => o.id === args.field.id)
            ) {
                browser.berytus.rejectRequest(context.request.id, ERejectionCode.GeneralError);
                return;
            }
            if (args.field.type === "Key" || args.field.type === "SecurePassword") {
                console.warn("PutField of type Key or SecurePassword are not allowed.");
                // TODO(berytus): Implement a logical validator in ValidatedRequestHandler
                browser.berytus.rejectRequest(context.request.id, ERejectionCode.GeneralError);
                return;
            }
            const clearValue = await toClearFieldValue(
                context.channel.id,
                args.field.value
            );
            const change: Pick<Session, 'requests' | 'putFields' | 'version'> = {
                requests: sessionRecord.requests.concat(context.request),
                putFields: (sessionRecord.putFields || []).concat({
                    id: args.field.id,
                    type: args.field.type,
                    options: args.field.options,
                    value: typeof clearValue === "string"
                        ? clearValue
                        : ab2base64(clearValue.privateKey)
                }),
                version: sessionRecord.version + 1
            };
            await db.sessions.update(sessionId, change);
            printSessionForSimiluationPrep('AccountCreation_AddField:Put', sessionId);
        },
        async rejectFieldValue(context, args): Promise<void> {
            const sessionId = context.operation.id;
            const sessionRecordPromise = getSessionRecord(sessionId);

            const relativePath = `create-field/${sessionId}/${args.fieldId}/rejected`;
            await showUi(context, sessionId, relativePath, sessionRecordPromise);
            const sessionRecord = await sessionRecordPromise;
            const field = sessionRecord.fields?.find(f => f.id === args.fieldId);
            const rejectedFieldValueIndex = sessionRecord.rejectedFieldValues?.findIndex(f => f.fieldId === args.fieldId);
            if (
                !sessionRecord.createFieldOptions?.find(o => o.id === args.fieldId) ||
                !field
            ) {
                browser.berytus.rejectRequest(context.request.id, ERejectionCode.GeneralError);
                return;
            }
            if (
                typeof args.optionalNewValue === "object" && (
                    "publicKey" in args.optionalNewValue ||
                    "salt" in args.optionalNewValue
                )
            ) {
                console.warn("RejectField of type Key or SecurePassword are not allowed.");
                // TODO(berytus): Implement a logical validator in ValidatedRequestHandler
                browser.berytus.rejectRequest(context.request.id, ERejectionCode.GeneralError);
                return;
            }
            let change: Pick<Session, 'requests' | 'rejectedFieldValues' | 'version'>;
            let sameValue = false;
            let value = undefined;
            do {
                if (! args.optionalNewValue) {
                    break;
                }
                const newValue = await toClearFieldValue(
                    context.channel.id,
                    args.optionalNewValue
                );
                if (typeof newValue === "string") {
                    value = newValue;
                    sameValue = value === field.value;
                    break;
                }
                if (newValue.privateKey instanceof ArrayBuffer) {
                    value = ab2base64(newValue.privateKey);
                    sameValue = value === field.value;
                    break;
                }
            } while (false);
            if (sameValue) {
                // BRTTODO: A web app can reject a field value while dictating its new
                // value the same as the previous one (the one being rejected). This
                // should not happen, but since the AuthRealm API does not
                // store field values, it cannot validate whether the optionalNewValue
                // is different than the rejected one (or previously rejected).
                // So for now, we throw an error here.
                console.error('BRTTODO: WebApp rejected field value with an new value that equals the rejected one. Genuis.');
                throw new Error('Unable to rejectRegistrationField. WebApp rejected the field value with an invalid new value.');
            }
            if (rejectedFieldValueIndex === undefined || rejectedFieldValueIndex === -1) {
                change = {
                    requests: sessionRecord.requests.concat(context.request),
                    rejectedFieldValues: (sessionRecord.rejectedFieldValues || []).concat({
                        fieldId: args.fieldId,
                        code: args.reason.code,
                        webAppDictatedValue: value,
                        previousValues: [field.value]
                    }),
                    version: sessionRecord.version + 1
                };
            } else {
                if (sessionRecord.rejectedFieldValues![rejectedFieldValueIndex].webAppDictatedValue) {
                    throw new Error('This field has been rejected and overriden previously!');
                }
                sessionRecord.rejectedFieldValues![rejectedFieldValueIndex].webAppDictatedValue =
                    value;
                sessionRecord.rejectedFieldValues![rejectedFieldValueIndex].previousValues.push(
                    field.value
                );
                change = {
                    requests: sessionRecord.requests.concat(context.request),
                    rejectedFieldValues: sessionRecord.rejectedFieldValues!,
                    version: sessionRecord.version + 1
                };
            }
            await db.sessions.update(sessionId, change);
            printSessionForSimiluationPrep('AccountCreation_RejectFieldValue', sessionId);
        }
    },
    accountAuthentication: {
        async approveChallengeRequest(context, args): Promise<void> {
            const sessionId = context.operation.id;
            const sessionRecordPromise = getSessionRecord(sessionId);
            const challengeId = args.challenge.id;
            const relativePath = `auth/${sessionId}/challenge/${challengeId}/approve`;

            await showUi(context, sessionId, relativePath, sessionRecordPromise);
            const sessionRecord = await sessionRecordPromise;
            const change: Pick<Session, 'challenges' | 'requests' | 'version'> = {
                challenges: {
                    ...sessionRecord.challenges,
                    [challengeId]: {
                        ...args.challenge,
                        messages: {}
                    }
                },
                requests: sessionRecord.requests.concat(context.request),
                version: sessionRecord.version + 1
            };

            await db.sessions.update(sessionRecord, change);
            printSessionForSimiluationPrep('AccountAuthentication_ApproveChallengeRequest', sessionId);
        },
        async abortChallenge(context, args): Promise<void> {
            const sessionId = context.operation.id;
            const sessionRecordPromise = getSessionRecord(sessionId);
            const challengeId = args.challenge.id;
            const relativePath = `auth/${sessionId}/challenge/${challengeId}/aborted`;

            await showUi(context, sessionId, relativePath, sessionRecordPromise);
            const sessionRecord = await sessionRecordPromise;
            await db.sessions.update(sessionRecord, {
                requests: sessionRecord.requests.concat(context.request),
                [`challenges.${challengeId}.closed`]: true,
                [`challenges.${challengeId}.abortionReasonCode`]: args.reason
            });
            printSessionForSimiluationPrep('AccountAuthentication_AbortChallenge', sessionId);
        },
        async closeChallenge(context, args): Promise<void> {
            const sessionId = context.operation.id;
            const sessionRecordPromise = getSessionRecord(sessionId);
            const challengeId = args.challenge.id;
            const relativePath = `auth/${sessionId}/challenge/${challengeId}/sealed`;

            await showUi(context, sessionId, relativePath, sessionRecordPromise);
            const sessionRecord = await sessionRecordPromise;
            await db.sessions.update(sessionRecord, {
                requests: sessionRecord.requests.concat(context.request),
                [`challenges.${challengeId}.closed`]: true
            });
            printSessionForSimiluationPrep('AccountAuthentication_CloseChallenge', sessionId);
        },
        async respondToChallengeMessage(context, args): Promise<void> {
            const sessionId = context.operation.id;
            const sessionRecordPromise = getSessionRecord(sessionId);
            const challengeId = args.challenge.id;
            const messageId = args.name;
            const relativePath = `auth/${sessionId}/challenge/${challengeId}/message/${messageId}`;

            await showUi(context, sessionId, relativePath, sessionRecordPromise);
            const sessionRecord = await sessionRecordPromise;
            let payload;
            if (typeof args.payload === "string") {
                payload = args.payload;
            } else if (args.payload instanceof ArrayBuffer) {
                payload = ab2base64(args.payload);
            } else if (ArrayBuffer.isView(args.payload)) {
                console.warn("TODO(berytus): Remove ArrayBufferView support");
                context.response.reject(ERejectionCode.GeneralError);
                return;
            } else if (Array.isArray(args.payload)) {
                payload = args.payload;
            } else {
                console.warn("TODO(berytus): Support Encrypted Packet");
                context.response.reject(ERejectionCode.GeneralError);
                return;
            }
            await db.sessions.update(sessionRecord, {
                [`challenges.${challengeId}.messages.${messageId}`]: {
                    name: messageId,
                    payload
                },
                requests: sessionRecord.requests.concat(context.request),
                version: sessionRecord.version + 1
            });
            printSessionForSimiluationPrep('AccountAuthentication_RespondToChallengeMessage', sessionId);
        }
    }
});