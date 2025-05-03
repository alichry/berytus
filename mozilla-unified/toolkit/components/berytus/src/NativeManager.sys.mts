/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import type { IUnderlyingRequestHandler, AbortChallengeArgs, AddFieldArgs, ApproveChallengeRequestArgs, ApproveOperationArgs, ELoginUserIntent, ApproveTransitionToAuthOpArgs, ChallengeMessageResponse, CloseChallengeArgs, CredentialsMetadata, GenerateKeyExchangeParametersArgs, GetCredentialsMetadataArgs, CreateChannelArgs, PartialKeyExchangeParametersFromScm, PreliminaryRequestContext, RecordMetadata, RejectFieldValueArgs, RequestContext, RequestContextWithOperation, RequestHandler, RespondToChallengeMessageArgs, UpdateMetadataArgs, ResponseContext, UpdateUserAttributesArgs, RequestContextWithLoginOperation, EMetadataStatus, SignKeyAgreementParametersArgs, GetSigningKeyArgs, VerifySignedKeyExchangeParametersArgs } from './types';

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
        if (args.operation.intent === "PendingDeclaration") {
            context.response.resolve("Register" as ELoginUserIntent);
            return;
        }
        context.response.resolve(args.operation.intent);
    }
    closeOperation(context: RequestContextWithOperation & ResponseContext<'login', 'closeOperation'>): void {
        context.response.resolve();
    }
    getRecordMetadata(context: RequestContextWithOperation & ResponseContext<'login', 'getRecordMetadata'>): void {
        context.response.resolve({
            category: "",
            version: 0,
            status: "Created" as EMetadataStatus.Created,
            changePassUrl: ""
        });
    }
    updateMetadata(context: RequestContextWithOperation & ResponseContext<'login', 'updateMetadata'>, args: UpdateMetadataArgs): void {
        context.response.resolve();
    }
}

class ChannelRequestHandler implements ChannelRequests {
    createChannel(context: PreliminaryRequestContext & ResponseContext<'channel', 'createChannel'>, args: CreateChannelArgs): void {
        // priv: MC4CAQAwBQYDK2VwBCIEINceTfVAd0DzkZKmfmGurcoljjOPm6Ix9CTNBXLcWt3b
        context.response.resolve();
    }
    generateX25519Key(context: RequestContext & ResponseContext<'channel', 'generateX25519Key'>): void {
        throw new Error('Method not implemented.');
    }
    signKeyExchangeParameters(context: RequestContext & ResponseContext<'channel', 'signKeyExchangeParameters'>, args: SignKeyAgreementParametersArgs): void {
        throw new Error('Method not implemented.');
    }
    verifySignedKeyExchangeParameters(context: RequestContext & ResponseContext<'channel', 'verifySignedKeyExchangeParameters'>, args: VerifySignedKeyExchangeParametersArgs): void {
        throw new Error('Method not implemented.');
    }
    enableEndToEndEncryption(context: RequestContext & ResponseContext<'channel', 'enableEndToEndEncryption'>): void {
        throw new Error('Method not implemented.');
    }
    closeChannel(context: RequestContext & ResponseContext<'channel', 'closeChannel'>): void {
        context.response.resolve();
    }
}

class AccountCreationRequestHandler implements AccountCreationRequests {
    approveTransitionToAuthOp(context: RequestContextWithOperation & ResponseContext<'accountCreation', 'approveTransitionToAuthOp'>, args: ApproveTransitionToAuthOpArgs): void {
        context.response.resolve();
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
    addField(context: RequestContextWithLoginOperation & ResponseContext<'accountCreation', 'addField'>, args: AddFieldArgs): void {
        switch (args.field.type) {
            case "Identity":
                context.response.resolve(
                    args.field.value !== null
                        ? null
                        : "testUsername"
                    );
                break;
            case "ForeignIdentity":
                context.response.resolve(
                    args.field.value !== null
                        ? null
                        : "test@example.tld"
                    );
                break;
            case "Password":
                context.response.resolve(args.field.value !== null
                    ? null
                    : "password1234"
                );
                break;
            case "SecurePassword":
                context.response.resolve(
                    args.field.value !== null
                        ? null
                        : {
                            salt: new Uint8Array([1,2,3]).buffer,
                            verifier: new Uint8Array([4,5,6]).buffer
                        }
                );
                break;
            case "Key":
                context.response.resolve(
                    args.field.value !== null
                        ? null
                        : {
                            publicKey: new Uint8Array([1,2,3]).buffer
                        }
                );
                break;
            case "SharedKey":
                context.response.resolve(
                    args.field.value !== null
                        ? null
                        : {
                            privateKey: new Uint8Array([1,2,3]).buffer
                        }
                );
                break;
            default:
                throw new Error("Unrecognised field type");
        }
    }
    rejectFieldValue(context: RequestContextWithLoginOperation & ResponseContext<'accountCreation', 'rejectFieldValue'>, args: RejectFieldValueArgs): void {
        const field = context.operation.fields[args.fieldId];
        switch (field.type) {
            case "Identity":
                context.response.resolve(
                    args.optionalNewValue !== undefined
                        ? null
                        : "revisedTestUsername"
                );
                break;
            case "ForeignIdentity":
                context.response.resolve(
                    args.optionalNewValue !== undefined
                        ? null
                        : "revised.test@example.tld"
                );
                break;
            case "Password":
                context.response.resolve(
                    args.optionalNewValue !== undefined
                        ? null
                        : "revisedPassword1234"
                );
                break;
            case "SecurePassword":
                context.response.resolve(
                    args.optionalNewValue !== undefined
                        ? null
                        : {
                            salt: new Uint8Array([10, 20, 30]).buffer,
                            verifier: new Uint8Array([40, 50, 60]).buffer
                        }
                );
                break;
            case "Key":
                context.response.resolve(
                    args.optionalNewValue !== undefined
                        ? null
                        : {
                            publicKey: new Uint8Array([10,20,30]).buffer
                        }
                    );
                break;
            case "SharedKey":
                context.response.resolve(
                    args.optionalNewValue !== undefined
                        ? null
                        : {
                                privateKey: new Uint8Array([10,20,30]).buffer
                        }
                );
                break;
            default:
                throw new Error("Unrecognised field type");
        }
    }
}

class AccountAuthenticationRequestHandler implements AccountAuthenticationRequests {
    approveChallengeRequest(context: RequestContextWithOperation & ResponseContext<'accountAuthentication', 'approveChallengeRequest'>, args: ApproveChallengeRequestArgs): void {
        context.response.resolve();
    }
    abortChallenge(context: RequestContextWithOperation & ResponseContext<'accountAuthentication', 'abortChallenge'>, args: AbortChallengeArgs): void {
        context.response.resolve();
    }
    closeChallenge(context: RequestContextWithOperation & ResponseContext<'accountAuthentication', 'closeChallenge'>, args: CloseChallengeArgs): void {
        context.response.resolve();
    }
    respondToChallengeMessage(context: RequestContextWithLoginOperation & ResponseContext<'accountAuthentication', 'respondToChallengeMessage'>, args: RespondToChallengeMessageArgs): void {
        switch (args.challenge.type) {
            case "Identification": {
                const result: Record<string, string> = {};
                (args.payload as Array<string>).forEach(fieldId => {
                    result[fieldId] = "user123";
                });
                context.response.resolve({
                    response: result
                });
                return;
            }
            case "Password": {
                const result: Record<string, string> = {};
                (args.payload as Array<string>).forEach(fieldId => {
                    result[fieldId] = "pass123";
                });
                context.response.resolve({
                    response: result
                });
                return;
            }
            case "DigitalSignature": {
                if (args.name === "SelectKey") {
                    context.response.resolve({
                        response: {
                            publicKey: new Uint8Array([1,2,3]).buffer
                        }
                    });
                    return;
                }
                context.response.resolve({
                    response: new Uint8Array([1,2,3]).buffer
                });
                return;
            }
            case "OffChannelOtp": {
                context.response.resolve({
                    response: "123456"
                });
                return;
            }
            case "SecureRemotePassword": {
                switch (args.name) {
                    case 'SelectSecurePassword': {
                        context.response.resolve({
                            response: "user123"
                        });
                        return;
                    }
                    case "ExchangePublicKeys": {
                        const clientPublicKeyHex = "96b608dfa551a481331a18818dd30c69431f86e0438cfa2cca2c1638626fcffbb50bb688b4ef49c47abc2f9da629e88f09a2c306a5fddf2d9c4fe1237c29a6939ad01dd62613f38b5e08fc60da1e63d857324ea2ba7a82bac682a3d425c4a3ece1ba3ea9113d13fbb0a01d2998e95de366bb26243374b2665ff20a40f80534f0b6d2bb2ddc727a877de4725a52bf38f0e9534f1629ff36896b95aab14f19224e8fa7d51c136e2aa1ce321ca1758d53bec97fd5d97d0896f4dd3a4630a62c265a550243b8535dece439e62977da4ed41b914810f03c1f0c7885609e96bd6375d7cafd084dc1b40ddc5f467aae2cfa478bdd89a81caf8ed1de5d4e85dc2c56a7bec60ecd0df57d24a5dbaac049b74fb615f0f4c83420279b4f46cfae93795937f6b6e86a80b8f9a942ffedca93f31c210ccb2956c851aed8d737ab233c54bc87a04ccdf8f885eedefcee6378c54e9090d1e2b1ad9c8ba1898c63e0cf3913e810c6684bb36effa503df0288eef6720330d8cda8ad54194166200d6461b14809380e385afa68e5387188e293d59db1c46b1589ad3c48d99550fc104345b9d61231ac0ff753f28ae150aeaea1cf3fd448017fac7c534d1d608579deb3944bc0555482a0ce2102a19d02fe5fe18c463c460a88a93cc1c6b8052576de1c32c2204a7adc0a4d6fb9648cb93364e44b0c8c80e3f4a50c19c9c41bf771ae4d42e8d508285b";
                        if (args.challenge.parameters.encoding === "Hex") {
                            context.response.resolve({
                                response: clientPublicKeyHex
                            });
                            return;
                        }
                        context.response.resolve({
                            response: new Uint8Array(clientPublicKeyHex.match(/[\da-f]{2}/gi)!.map(function (h) {
                                return parseInt(h, 16)
                            })).buffer
                        });
                        return;
                    }
                    case "ComputeClientProof": {
                        const dummyHex = "AA55";
                        if (args.challenge.parameters.encoding === "Hex") {
                            context.response.resolve({
                                response: dummyHex
                            });
                            return;
                        }
                        context.response.resolve({
                            response: new Uint8Array(dummyHex.match(/[\da-f]{2}/gi)!.map(function (h) {
                                return parseInt(h, 16)
                            })).buffer
                        });
                        return;
                    }
                    case "VerifyServerProof": {
                        context.response.resolve({});
                        return;
                    }
                }

            }
        }
    }
}

export class NativeManager implements IUnderlyingRequestHandler {
    manager = new ManagerRequestHandler();
    login = new LoginRequestHandler();
    channel = new ChannelRequestHandler();
    accountCreation = new AccountCreationRequestHandler();
    accountAuthentication = new AccountAuthenticationRequestHandler();
}