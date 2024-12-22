/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import type { IUnderlyingRequestHandler, AbortChallengeArgs, AddFieldArgs, ApproveChallengeRequestArgs, ApproveOperationArgs, ELoginUserIntent, ApproveTransitionToAuthOpArgs, ChallengeMessageResponse, CloseChallengeArgs, CredentialsMetadata, EnableEndToEndEncryptionArgs, GenerateKeyExchangeParametersArgs, GetCredentialsMetadataArgs, GetSigningKeyArgs, PartialKeyExchangeParametersFromScm, PreliminaryRequestContext, RecordMetadata, RejectFieldValueArgs, RequestContext, RequestContextWithOperation, RequestHandler, RespondToChallengeMessageArgs, UpdateMetadataArgs, ResponseContext, UpdateUserAttributesArgs } from './types';

type ManagerRequests = IUnderlyingRequestHandler['manager'];
type LoginRequests = IUnderlyingRequestHandler['login'];
type ChannelRequests = IUnderlyingRequestHandler['channel'];
type AccountCreationRequests = IUnderlyingRequestHandler['accountCreation'];
type AccountAuthenticationRequests = IUnderlyingRequestHandler['accountAuthentication'];

class ManagerRequestHandler implements ManagerRequests {
    getSigningKey(context: PreliminaryRequestContext & ResponseContext<'manager', 'getSigningKey'>, args: GetSigningKeyArgs): void {
        // priv: MC4CAQAwBQYDK2VwBCIEINceTfVAd0DzkZKmfmGurcoljjOPm6Ix9CTNBXLcWt3b
        context.response.resolve(
            "MCowBQYDK2VwAyEAJevlUdx72BF8mxdwurBJI9WNgRDMaoYfb0VqywaLOJE="
        );
    }
    getCredentialsMetadata(context: PreliminaryRequestContext & ResponseContext<'manager', 'getCredentialsMetadata'>, args: GetCredentialsMetadataArgs): void {
        context.response.resolve(3);
    }
}

class LoginRequestHandler implements LoginRequests {
    approveOperation(context: RequestContext & ResponseContext<'login', 'approveOperation'>, args: ApproveOperationArgs): void {
        if (args.operation.intent !== "PendingDeclaration") {
            context.response.resolve("Register" as ELoginUserIntent);
            return;
        }
        context.response.resolve(args.operation.intent);
    }
    closeOpeation(context: RequestContextWithOperation & ResponseContext<'login', 'closeOpeation'>): void {
        throw new Error('Method not implemented.');
    }
    getRecordMetadata(context: RequestContextWithOperation & ResponseContext<'login', 'getRecordMetadata'>): void {
        throw new Error('Method not implemented.');
    }
    updateMetadata(context: RequestContextWithOperation & ResponseContext<'login', 'updateMetadata'>, args: UpdateMetadataArgs): void {
        throw new Error('Method not implemented.');
    }
}

class ChannelRequestHandler implements ChannelRequests {
    generateKeyExchangeParameters(context: RequestContext & ResponseContext<'channel', 'generateKeyExchangeParameters'>, args: GenerateKeyExchangeParametersArgs): void {
        throw new Error('Method not implemented.');
    }
    enableEndToEndEncryption(context: RequestContext & ResponseContext<'channel', 'enableEndToEndEncryption'>, args: EnableEndToEndEncryptionArgs): void {
        throw new Error('Method not implemented.');
    }
    closeChannel(context: RequestContext & ResponseContext<'channel', 'closeChannel'>): void {
        throw new Error('Method not implemented.');
    }
}

class AccountCreationRequestHandler implements AccountCreationRequests {
    approveTransitionToAuthOp(context: RequestContextWithOperation & ResponseContext<'accountCreation', 'approveTransitionToAuthOp'>, args: ApproveTransitionToAuthOpArgs): void {
        throw new Error('Method not implemented.');
    }
    getUserAttributes(context: RequestContextWithOperation & ResponseContext<'accountCreation', 'getUserAttributes'>): void {
        context.response.resolve([
            { id: "name", value: "Ali", mimeType: "text/plain" },
            { id: "familyName", value: "Cherry", mimeType: "text/plain" }
        ]);
    }
    updateUserAttributes(context: RequestContextWithOperation & ResponseContext<'accountCreation', 'updateUserAttributes'>, args: UpdateUserAttributesArgs): void {
        context.response.resolve();
    }
    addField(context: RequestContextWithOperation & ResponseContext<'accountCreation', 'addField'>, args: AddFieldArgs): void {
        switch (args.field.type) {
            case "Identity":
                context.response.resolve("testUsername");
                break;
            case "ForeignIdentity":
                context.response.resolve("test@example.tld");
                break;
            case "Password":
                context.response.resolve("password1234");
                break;
            case "SecurePassword":
                context.response.resolve({
                    salt: new Uint8Array([1,2,3]).buffer,
                    verifier: new Uint8Array([4,5,6]).buffer
                });
                break;
            case "Key":
                context.response.resolve({
                    publicKey: new Uint8Array([1,2,3]).buffer
                });
                break;
            case "SharedKey":
                context.response.resolve({
                    privateKey: new Uint8Array([1,2,3]).buffer
                });
                break;
            default:
                throw new Error("Unrecognised field type");
        }
    }
    rejectFieldValue(context: RequestContextWithOperation & ResponseContext<'accountCreation', 'rejectFieldValue'>, args: RejectFieldValueArgs): void {
        const fieldType = "Identity"; // TODO(berytus): provide fields in op metadata
        switch (fieldType as any) {
            case "Identity":
                context.response.resolve(
                    "revisedTestUsername"
                );
                break;
            case "ForeignIdentity":
                context.response.resolve(
                    "revised.test@example.tld"
                );
                break;
            case "Password":
                context.response.resolve(
                    "revisedPassword1234"
                );
                break;
            case "SecurePassword":
                context.response.resolve({
                    salt: new Uint8Array([10,20,30]).buffer,
                    verifier: new Uint8Array([40,50,60]).buffer
                });
                break;
            case "Key":
                context.response.resolve({
                    publicKey: new Uint8Array([10,20,30]).buffer
                });
                break;
            case "SharedKey":
                context.response.resolve({
                    privateKey: new Uint8Array([10,20,30]).buffer
                });
                break;
            default:
                throw new Error("Unrecognised field type");
        }
    }
}

class AccountAuthenticationRequestHandler implements AccountAuthenticationRequests {
    approveChallengeRequest(context: RequestContextWithOperation & ResponseContext<'accountAuthentication', 'approveChallengeRequest'>, args: ApproveChallengeRequestArgs): void {
        throw new Error('Method not implemented.');
    }
    abortChallenge(context: RequestContextWithOperation & ResponseContext<'accountAuthentication', 'abortChallenge'>, args: AbortChallengeArgs): void {
        throw new Error('Method not implemented.');
    }
    closeChallenge(context: RequestContextWithOperation & ResponseContext<'accountAuthentication', 'closeChallenge'>, args: CloseChallengeArgs): void {
        throw new Error('Method not implemented.');
    }
    respondToChallengeMessage(context: RequestContextWithOperation & ResponseContext<'accountAuthentication', 'respondToChallengeMessage'>, args: RespondToChallengeMessageArgs): void {
        throw new Error('Method not implemented.');
    }
}

export class NativeManager implements IUnderlyingRequestHandler {
    manager = new ManagerRequestHandler();
    login = new LoginRequestHandler();
    channel = new ChannelRequestHandler();
    accountCreation = new AccountCreationRequestHandler();
    accountAuthentication = new AccountAuthenticationRequestHandler();
}