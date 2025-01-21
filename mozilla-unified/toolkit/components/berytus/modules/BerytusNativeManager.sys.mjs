/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
class ManagerRequestHandler {
    getSigningKey(context, args) {
        // priv: MC4CAQAwBQYDK2VwBCIEINceTfVAd0DzkZKmfmGurcoljjOPm6Ix9CTNBXLcWt3b
        context.response.resolve("MCowBQYDK2VwAyEAJevlUdx72BF8mxdwurBJI9WNgRDMaoYfb0VqywaLOJE=");
    }
    getCredentialsMetadata(context, args) {
        context.response.resolve(3);
    }
}
class LoginRequestHandler {
    approveOperation(context, args) {
        if (args.operation.intent === "PendingDeclaration") {
            context.response.resolve("Register");
            return;
        }
        context.response.resolve(args.operation.intent);
    }
    closeOperation(context) {
        context.response.resolve();
    }
    getRecordMetadata(context) {
        context.response.resolve({
            category: "",
            version: 0,
            status: "Created",
            changePassUrl: ""
        });
    }
    updateMetadata(context, args) {
        context.response.resolve();
    }
}
class ChannelRequestHandler {
    generateKeyExchangeParameters(context, args) {
        throw new Error('Method not implemented.');
    }
    enableEndToEndEncryption(context, args) {
        throw new Error('Method not implemented.');
    }
    closeChannel(context) {
        context.response.resolve();
    }
}
class AccountCreationRequestHandler {
    approveTransitionToAuthOp(context, args) {
        context.response.resolve();
    }
    getUserAttributes(context) {
        context.response.resolve([
            { id: "name", value: "Ali", mimeType: "text/plain" },
            { id: "familyName", value: "Cherry", mimeType: "text/plain" }
        ]);
    }
    updateUserAttributes(context, args) {
        context.response.resolve();
    }
    addField(context, args) {
        switch (args.field.type) {
            case "Identity":
                context.response.resolve(args.field.value !== null
                    ? args.field.value
                    : "testUsername");
                break;
            case "ForeignIdentity":
                context.response.resolve(args.field.value !== null
                    ? args.field.value
                    : "test@example.tld");
                break;
            case "Password":
                context.response.resolve(args.field.value !== null
                    ? args.field.value
                    :
                        "password1234");
                break;
            case "SecurePassword":
                context.response.resolve(args.field.value !== null
                    ? args.field.value
                    : {
                        salt: new Uint8Array([1, 2, 3]).buffer,
                        verifier: new Uint8Array([4, 5, 6]).buffer
                    });
                break;
            case "Key":
                context.response.resolve(args.field.value !== null
                    ? args.field.value
                    : {
                        publicKey: new Uint8Array([1, 2, 3]).buffer
                    });
                break;
            case "SharedKey":
                context.response.resolve(args.field.value !== null
                    ? args.field.value
                    : {
                        privateKey: new Uint8Array([1, 2, 3]).buffer
                    });
                break;
            default:
                throw new Error("Unrecognised field type");
        }
    }
    rejectFieldValue(context, args) {
        const field = context.operation.fields[args.fieldId];
        switch (field.type) {
            case "Identity":
                context.response.resolve(args.optionalNewValue !== undefined
                    ? args.optionalNewValue
                    : "revisedTestUsername");
                break;
            case "ForeignIdentity":
                context.response.resolve(args.optionalNewValue !== undefined
                    ? args.optionalNewValue
                    : "revised.test@example.tld");
                break;
            case "Password":
                context.response.resolve(args.optionalNewValue !== undefined
                    ? args.optionalNewValue
                    : "revisedPassword1234");
                break;
            case "SecurePassword":
                context.response.resolve(args.optionalNewValue !== undefined
                    ? args.optionalNewValue
                    : {
                        salt: new Uint8Array([10, 20, 30]).buffer,
                        verifier: new Uint8Array([40, 50, 60]).buffer
                    });
                break;
            case "Key":
                context.response.resolve(args.optionalNewValue !== undefined
                    ? args.optionalNewValue
                    : {
                        publicKey: new Uint8Array([10, 20, 30]).buffer
                    });
                break;
            case "SharedKey":
                context.response.resolve(args.optionalNewValue !== undefined
                    ? args.optionalNewValue
                    : {
                        privateKey: new Uint8Array([10, 20, 30]).buffer
                    });
                break;
            default:
                throw new Error("Unrecognised field type");
        }
    }
}
class AccountAuthenticationRequestHandler {
    approveChallengeRequest(context, args) {
        context.response.resolve();
    }
    abortChallenge(context, args) {
        context.response.resolve();
    }
    closeChallenge(context, args) {
        context.response.resolve();
    }
    respondToChallengeMessage(context, args) {
        switch (args.challenge.type) {
            case "Identification": {
                const result = {};
                args.payload.forEach(fieldId => {
                    result[fieldId] = "user123";
                });
                context.response.resolve({
                    response: result
                });
                return;
            }
            case "Password": {
                const result = {};
                args.payload.forEach(fieldId => {
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
                            publicKey: new Uint8Array([1, 2, 3]).buffer
                        }
                    });
                    return;
                }
                context.response.resolve({
                    response: new Uint8Array([1, 2, 3]).buffer
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
                            response: new Uint8Array(clientPublicKeyHex.match(/[\da-f]{2}/gi).map(function (h) {
                                return parseInt(h, 16);
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
                            response: new Uint8Array(dummyHex.match(/[\da-f]{2}/gi).map(function (h) {
                                return parseInt(h, 16);
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
export class NativeManager {
    manager = new ManagerRequestHandler();
    login = new LoginRequestHandler();
    channel = new ChannelRequestHandler();
    accountCreation = new AccountCreationRequestHandler();
    accountAuthentication = new AccountAuthenticationRequestHandler();
}
