/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import type { IUnderlyingRequestHandler, AbortChallengeArgs, AddFieldArgs, ApproveChallengeRequestArgs, ApproveOperationArgs, ELoginUserIntent, ApproveTransitionToAuthOpArgs, ChallengeMessageResponse, CloseChallengeArgs, CredentialsMetadata, EnableEndToEndEncryptionArgs, FieldValue, GenerateKeyExchangeParametersArgs, GetCredentialsMetadataArgs, GetSigningKeyArgs, PartialKeyExchangeParametersFromScm, PreliminaryRequestContext, RecordMetadata, RejectFieldValueArgs, RequestContext, RequestContextWithOperation, RequestHandler, RespondToChallengeMessageArgs, UpdateMetadataArgs, ResponseContext } from './types';

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
        throw new Error('Method not implemented.');
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
        throw new Error('Method not implemented.');
    }
    addField(context: RequestContextWithOperation & ResponseContext<'accountCreation', 'addField'>, args: AddFieldArgs): void {
        throw new Error('Method not implemented.');
    }
    rejectFieldValue(context: RequestContextWithOperation & ResponseContext<'accountCreation', 'rejectFieldValue'>, args: RejectFieldValueArgs): void {
        throw new Error('Method not implemented.');
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