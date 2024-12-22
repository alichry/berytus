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
        if (args.operation.intent !== "PendingDeclaration") {
            context.response.resolve("Register");
            return;
        }
        context.response.resolve(args.operation.intent);
    }
    closeOpeation(context) {
        throw new Error('Method not implemented.');
    }
    getRecordMetadata(context) {
        throw new Error('Method not implemented.');
    }
    updateMetadata(context, args) {
        throw new Error('Method not implemented.');
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
        throw new Error('Method not implemented.');
    }
}
class AccountCreationRequestHandler {
    approveTransitionToAuthOp(context, args) {
        throw new Error('Method not implemented.');
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
                    salt: new Uint8Array([1, 2, 3]).buffer,
                    verifier: new Uint8Array([4, 5, 6]).buffer
                });
                break;
            case "Key":
                context.response.resolve({
                    publicKey: new Uint8Array([1, 2, 3]).buffer
                });
                break;
            case "SharedKey":
                context.response.resolve({
                    privateKey: new Uint8Array([1, 2, 3]).buffer
                });
                break;
            default:
                throw new Error("Unrecognised field type");
        }
    }
    rejectFieldValue(context, args) {
        const fieldType = "Identity"; // TODO(berytus): provide fields in op metadata
        switch (fieldType) {
            case "Identity":
                context.response.resolve("revisedTestUsername");
                break;
            case "ForeignIdentity":
                context.response.resolve("revised.test@example.tld");
                break;
            case "Password":
                context.response.resolve("revisedPassword1234");
                break;
            case "SecurePassword":
                context.response.resolve({
                    salt: new Uint8Array([10, 20, 30]).buffer,
                    verifier: new Uint8Array([40, 50, 60]).buffer
                });
                break;
            case "Key":
                context.response.resolve({
                    publicKey: new Uint8Array([10, 20, 30]).buffer
                });
                break;
            case "SharedKey":
                context.response.resolve({
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
        throw new Error('Method not implemented.');
    }
    abortChallenge(context, args) {
        throw new Error('Method not implemented.');
    }
    closeChallenge(context, args) {
        throw new Error('Method not implemented.');
    }
    respondToChallengeMessage(context, args) {
        throw new Error('Method not implemented.');
    }
}
export class NativeManager {
    manager = new ManagerRequestHandler();
    login = new LoginRequestHandler();
    channel = new ChannelRequestHandler();
    accountCreation = new AccountCreationRequestHandler();
    accountAuthentication = new AccountAuthenticationRequestHandler();
}
