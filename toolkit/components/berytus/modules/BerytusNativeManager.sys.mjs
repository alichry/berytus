class ManagerRequestHandler {
    getSigningKey(context, args) {
        throw new Error('Method not implemented.');
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
