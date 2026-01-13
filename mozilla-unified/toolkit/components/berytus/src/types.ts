/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import type { BerytusChallengeAbortionCode, BerytusChallengeInfoUnion, BerytusFieldOptionsUnion, BerytusFieldUnion, BerytusFieldValue, BerytusFieldValueUnion, BerytusKeyDerivationParams, BerytusKeyExchangeAuthentication, BerytusKeyExchangeParams, BerytusKeyExchangeSession, BerytusKeyGenParams, BerytusReceiveMessageUnion, BerytusSendMessageUnion, BerytusUserAttributeDefinition, EBerytusChallengeType, EBerytusFieldType } from "./generated/berytus.web.js";

export interface ChannelConstraints {
    secretManagerPublicKey?: string[];
    enableEndToEndEncryption: boolean;
    account?: AccountConstraints;
}

export interface PartialAccountIdentity {
    fieldId: string;
    fieldValue: string;
}

export type AccountIdentity = Array<PartialAccountIdentity>;

export interface AccountConstraints {
    category?: string;
    schemaVersion?: number;
    identity?: AccountIdentity;
}

export interface ChannelMetadata {
    id: string;
    constraints: ChannelConstraints;
    webAppActor: WebAppActor;
    scmActor: CryptoActor; /* retrieved from getSigningKey() */
    e2eeEnabled: boolean;
}

export interface UriParams {
    uri: string;
    scheme: string;
    hostname: string;
    port: number;
    path: string;
};

export interface OriginActor {
    originalUri: UriParams;
    currentUri: UriParams;
}

export interface CryptoActor {
    ed25519Key: string;
}

export interface Request {
    id: string;
    type: string;
}

export interface DocumentMetadata {
    id: number; /* e.g. tabId if sent to extensions */
    uri: UriParams;
}

export enum EOperationStatus {
    Pending = "Pending",
    Created = "Created",
    Aborted = "Aborted",
    Finished = "Finished"
}

export enum EOperationType {
    PendingDeclaration = "PendingDeclaration",
    Registration = "Registration",
    Authentication = "Authentication"
}

export interface OperationState {}

export interface OperationMetadata {
    id: string;
    type: EOperationType;
    status: EOperationStatus;
    /**
     * State about the operation, e.g.:
     * { fields:  [BerytusIdentityField, ...], ... }
     */
    // TODO(berytus): This is currently unused.
    state: OperationState;
}

export interface FieldInfo {
    id: string;
    type: EBerytusFieldType;
    // NOTE(berytus): ValidatedRequestHandler does not
    // (yet) check if the options is logically conformant.
    // Alternatively, type FieldInfo = Omit<BerytusFieldUnion, "value">
    // -- however, type-parser would not be able to parse Omit<..>
    // A better option is to generate *FieldInfoUnion
    options: BerytusFieldOptionsUnion;
}

export interface LoginOperationMetadata extends OperationMetadata {
    intent: ELoginUserIntent;
    requestedUserAttributes: RequestedUserAttributes;
    fields: Record<string, FieldInfo>;
    challenges: Record<string, BerytusChallengeInfoUnion>;
}

// TODO(berytus): Addd LoginOperation : *Metadata which includes state
// about the fields created and so on.

export interface PreliminaryRequestContext {
    request: Request;
    document: DocumentMetadata;
}

export interface RequestContext extends PreliminaryRequestContext {
    channel: ChannelMetadata;
}

export interface RequestContextWithOperation extends RequestContext {
    operation: OperationMetadata;
}

export interface RequestContextWithLoginOperation extends RequestContext {
    operation: LoginOperationMetadata;
}

export type CredentialsMetadata = number;

export enum ELoginUserIntent {
    PendingDeclaration = "PendingDeclaration",
    Authenticate = "Authenticate",
    Register = "Register"
}

export type WebAppActor = CryptoActor | OriginActor;

export interface GenerateX25519KeyResult {
    public: string;
};

export interface InitialKeyExchangeParametersDraft {
    readonly channelId: string; // provided by Berytus
    readonly webAppX25519Key: string; // provided by the web app
}

/**
 * What the SCM should return when generating
 * the key exch paramters.
 */
export interface PartialKeyExchangeParametersFromScm {
    scmX25519Key: string;
    hkdfHash: string; //hash name
    hkdfSalt: ArrayBuffer;
    hkdfInfo: ArrayBuffer;
    aesKeyLength: number; // bits
}

export interface KeyExchangeParameters
    extends InitialKeyExchangeParametersDraft,
            PartialKeyExchangeParametersFromScm {
    /**
     * This is what should be signed.
     */
    packet: string;
}

enum EChallengeType {
    Identification = "Identification",
    DigitalSignature = "DigitalSignature",
    Password = "Password",
    SecureRemotePassword = "SecureRemotePassword",
    // TODO(berytus): Test this, I think we have mooved
    // to OffChannelOtp
    ForeignIdentityOtp = "ForeignIdentityOtp"
}

enum EChallengeStatus {
    Invalid = "Invalid",
    Pending = "Pending",
    Active = "Active",
    Aborted = "Aborted",
    Sealed = "Sealed"
}

export interface ChallengeParameters {}

export interface ChallengeMetadata {
    readonly id: string;
    readonly type: EChallengeType;
    readonly parameters: ChallengeParameters;
    readonly status: EChallengeStatus;
}

export interface ChallengePayload {}

export interface ChallengeMessage {
    /**
     * Unique message name across the challenge.
     */
    name: string; // one of EMT{x}
    payload: ChallengePayload;
};

export interface ChallengeMessageResponse {
    payload: ChallengePayload;
};

export interface ChallengeAbortionReason {
    /**
     * TODO(berytus): Use string type instead of
     * ChallengeAbortionCode UnionType until we can
     * handle unions in schema generation using `choices`
     */
    code: string;
}

// TODO(berytus): Addd ChallengeDetail : ChallengeMetadata which includes state
// about the fields created and so on.

export type UserAttributeKey = "name" | "givenName" | "familyName" | "middleName" |
    "nickname" |/* "preferredUsername" |*/ "profile" | "picture" |
    "website" |/* "email" |*/ "gender" | "birthdate" | "zoneinfo" |
    "locale" |/* "phoneNumber" |*/ "address" | `custom:${string}`;


export interface UserAttribute extends BerytusUserAttributeDefinition {
    id: UserAttributeKey;
};

export interface RequestedUserAttribute {
    id: UserAttributeKey;
    required: boolean;
}

export type UserAttributes = Array<UserAttribute>;
export type RequestedUserAttributes = Array<RequestedUserAttribute>;

enum EFieldType {
    Identity = "Identity",
    ForeignIdentity = "ForeignIdentity",
    Password = "Password",
    SecurePassword = "SecurePassword",
    Key = "Key"
};

export interface BaseFieldMetadata {
    fieldType: EFieldType;
    fieldId: string;
    description?: string;
}

export interface FieldValueRejectionReason {
    code: string;
}

// TODO(berytus): ERejectionCode should be in the API.
export enum ERejectionCode {
    UnknownReason,
    GeneralError,
    NetworkError,
    UserError,
    OperationAborted,
    /* begin challenge trap codes */
    ChallengeNotSupported,
    UnexpectedChallengeMessage,
    InvalidChallengeMessage
    /* end challenge trap codes */
}

enum EMetadataProperty {
    Version = "Version",
    Status = "Status",
    Category = "Category",
    ChangePassUrl = "ChangePassUrl"
};

export enum EMetadataStatus {
    Pending = "Pending",
    Created = "Created",
    Retired = "Retired",
};

export interface RecordMetadata {
    version: number;
    status: EMetadataStatus;
    category: string;
    changePassUrl: string;
}

/* Request Arguments */
export type GetCredentialsMetadataArgs = {
    webAppActor: WebAppActor,
    channelConstraints: ChannelConstraints,
    accountConstraints: AccountConstraints
};
export type GetSigningKeyArgs = {
    webAppActor: WebAppActor;
};
export type GetSigningKeyResult = string;

export type CreateChannelArgs = {
    channel: ChannelMetadata;
};

export interface KeyAgreementParameters {
    authentication: BerytusKeyExchangeAuthentication;
    session: BerytusKeyExchangeSession;
    exchange: BerytusKeyExchangeParams;
    derivation: BerytusKeyDerivationParams;
    generation: BerytusKeyGenParams;
};
export type SignKeyAgreementParametersArgs = {
    /**
     * CanonicalJSON-stringified KeyAgreementParameters
     */
    canonicalJson: string;
}
export type SignKeyAgreementParametersResult = {
    scmSignature: ArrayBuffer;
}

export type GenerateKeyExchangeParametersArgs = {
    paramsDraft: InitialKeyExchangeParametersDraft
}
export type VerifySignedKeyExchangeParametersArgs = {
    /**
     * CanonicalJSON-stringified KeyAgreementParameters
     */
    canonicalJson: string;
    webAppSignature: ArrayBuffer;
}
export type ApproveOperationArgs = {
    operation: LoginOperationMetadata
}
export type UpdateMetadataArgs = {
    metadata: RecordMetadata
}

export interface UpdateUserAttributesArgs {
    userAttributes: Array<UserAttribute>;
};

export type ApproveTransitionToAuthOpArgs = {
    newAuthOp: LoginOperationMetadata
}
export type AddFieldArgs = {
    field: BerytusFieldUnion;
}

/**
 * The secret manager should resolve with null
 * when a field value was specified in the args.
 */
export type AddFieldResult = BerytusFieldValueUnion | null;

export type RejectFieldValueArgs = {
    fieldId: string;
    reason: FieldValueRejectionReason,
    optionalNewValue?: BerytusFieldValueUnion;
}

/**
 * The secret manager should resolve with null
 * when a field value was specified in the args.
 */
export type RejectFieldResult = BerytusFieldValueUnion | null;

export type ApproveChallengeRequestArgs = {
    challenge: BerytusChallengeInfoUnion;
}
export type AbortChallengeArgs = {
    challenge: BerytusChallengeInfoUnion;
    reason: BerytusChallengeAbortionCode
}
export type CloseChallengeArgs = {
    challenge: BerytusChallengeInfoUnion;
}
export type RespondToChallengeMessageArgs = BerytusSendMessageUnion;
export type { BerytusSendMessageUnion };
export type RespondToChallengeMessageResult = BerytusReceiveMessageUnion;
export type { BerytusReceiveMessageUnion };
export type { BerytusChallengeInfoUnion };
export { EBerytusChallengeType };
/* Request Arguments */

export interface RequestHandler {
    manager: {
        getSigningKey(
            context: PreliminaryRequestContext,
            args: GetSigningKeyArgs
        ): GetSigningKeyResult;
        getCredentialsMetadata(
            context: PreliminaryRequestContext,
            args: GetCredentialsMetadataArgs
        ): CredentialsMetadata;
    };
    channel: {
        createChannel(
            context: PreliminaryRequestContext,
            args: CreateChannelArgs
        ): void;
        generateX25519Key(context: RequestContext): GenerateX25519KeyResult;
        signKeyExchangeParameters(
            context: RequestContext,
            args: SignKeyAgreementParametersArgs
        ): SignKeyAgreementParametersResult;
        verifySignedKeyExchangeParameters(
            context: RequestContext,
            args: VerifySignedKeyExchangeParametersArgs
        ): void;
        enableEndToEndEncryption(context: RequestContext): void;
        closeChannel(context: RequestContext): void;
    };
    /**
     * Common requests between the account creation/registration
     * operations, grouped under login.
     */
    login: {
        approveOperation(
            context: RequestContext,
            args: ApproveOperationArgs
        ): ELoginUserIntent;
        closeOperation(
            context: RequestContextWithOperation
        ): void;
        /* Berytus setPasswordUrl/setStatus/setVersion/setCategory: */
        getRecordMetadata(
            context: RequestContextWithOperation
        ): RecordMetadata
        updateMetadata(
            context: RequestContextWithOperation,
            args: UpdateMetadataArgs
        ): void
    },
    accountCreation: {
        approveTransitionToAuthOp(
            context: RequestContextWithOperation,
            args: ApproveTransitionToAuthOpArgs
        ): void;
        getUserAttributes(
            context: RequestContextWithLoginOperation
        ): UserAttributes;
        updateUserAttributes(
            context: RequestContextWithOperation,
            args: UpdateUserAttributesArgs
        ): void;
        addField(
            context: RequestContextWithLoginOperation,
            args: AddFieldArgs
        ): AddFieldResult;
        rejectFieldValue(
            context: RequestContextWithLoginOperation,
            args: RejectFieldValueArgs
        ): RejectFieldResult;
    };
    accountAuthentication: {
        approveChallengeRequest(
            context: RequestContextWithOperation,
            args: ApproveChallengeRequestArgs
        ): void;
        abortChallenge(
            context: RequestContextWithOperation,
            args: AbortChallengeArgs
        ): void;
        closeChallenge(
            context: RequestContextWithOperation,
            args: CloseChallengeArgs,
        ): void;
        respondToChallengeMessage(
            context: RequestContextWithLoginOperation,
            args: RespondToChallengeMessageArgs
        ): RespondToChallengeMessageResult;
    }
}

export type ResponseContext<G extends keyof RequestHandler, M extends keyof RequestHandler[G]> = {
    response:
        RequestHandler[G][M] extends (...args: any[]) => any
        ? {
            resolve: (val: ReturnType<RequestHandler[G][M]>) => void,
            reject: (val: unknown) => void
        }
        : never
}

export type IUnderlyingRequestHandler = {
    [group in keyof RequestHandler]: {
        [method in keyof RequestHandler[group]]:
            RequestHandler[group][method] extends (...args: any[]) => any
                ? Parameters<RequestHandler[group][method]> extends [infer context, ...infer tail]
                    ? (context: context & ResponseContext<group,method> , ...rest: tail) => void
                    : never
                : never
    }
}

export type IPublicRequestHandler = {
    [group in keyof RequestHandler]: {
        [method in keyof RequestHandler[group]]:
            RequestHandler[group][method] extends (...args: any[]) => any
                ? Parameters<RequestHandler[group][method]> extends [infer context, ...infer tail]
                    ?   (context: Omit<context, 'request'>, ...rest: tail) => Promise<ReturnType<RequestHandler[group][method]>>
                    : never
                : never
    }
}

export type RequestGroup = keyof RequestHandler;
export type RequestType = keyof {
    [K in RequestGroup as `${Capitalize<K>}_${Capitalize<keyof RequestHandler[K] & string>}`]: true
}

export type RequestHandlerFunction<ER extends RequestType> =
    Uncapitalize<ER> extends `${infer G}_${infer T}`
        ? G extends RequestGroup
            ? Uncapitalize<T> extends `${infer M}`
                ? M extends keyof RequestHandler[G]
                    ? RequestHandler[G][M]
                    : never
                : never
            : never
    : never;

export type RequestHandlerFunctionParameters<ER extends RequestType>
    = Parameters<RequestHandlerFunction<ER>>;

export type RequestHandlerFunctionReturnType<ER extends RequestType>
    = Awaited<ReturnType<RequestHandlerFunction<ER>>>;