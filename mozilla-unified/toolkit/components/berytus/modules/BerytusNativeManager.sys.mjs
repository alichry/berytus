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
        throw new Error('Method not implemented.');
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
        throw new Error('Method not implemented.');
    }
    addField(context, args) {
        throw new Error('Method not implemented.');
    }
    rejectFieldValue(context, args) {
        throw new Error('Method not implemented.');
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
