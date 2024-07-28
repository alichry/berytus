/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

export interface ChannelConstraints {
    secretManagerPublicKey?: string[];
    enableEndToEndEncryption?: boolean;
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
    webAppActor: CryptoActor | OriginActor;
    scmActor: CryptoActor; /* retrieved from getSigningKey */
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
    type: RequestType;
}

export interface DocumentMetadata {
    id: number; /* e.g. tabId if sent to extensions */
}

enum EOperationStatus {
    Pending = 0,
    Created = 1,
    Aborted = 2,
    Finished = 4
}

enum EOpeationType {
    PendingDeclaration = 0,
    Registration = 1,
    Authentication = 2
}

export interface OperationMetadata {
    id: string;
    type: EOpeationType;
    status: EOperationStatus;
    /**
     * State about the operation, e.g.:
     * { fields:  [BerytusIdentityField, ...], ... }
     */
    state: unknown;
}

export interface LoginOperationMetadata extends OperationMetadata {
    intent: ELoginUserIntent;
    requestedUserAttributes: RequestedUserAttributes;
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

export type CredentialsMetadata = number;

enum ELoginUserIntent {
    PendingDeclaration = 0,
    Authenticate = 1,
    Register = 2
}

type WebAppActor = CryptoActor | OriginActor;

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
    Identification,
    DigitalSignature,
    Password,
    SecureRemotePassword,
    ForeignIdentityOtp
}

enum EChallengeStatus {
    Invalid,
    Pending,
    Active,
    Aborted,
    Sealed
}

export interface ChallengeMetadata {
    readonly id: string;
    readonly type: EChallengeType;
    readonly parameters: object;
    readonly status: EChallengeStatus;
}


export interface ChallengeMessage {
    /**
     * Unique message name across the challenge.
     */
    name: string; // one of EMT{x}
    payload: any;
};

export interface ChallengeMessageResponse {
    payload: unknown;
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


export type UserAttribute = {
    id: UserAttributeKey;
    mimeType: string;
    value: string; // always a string, base64 encode if necessary
};
export type RequestedUserAttribute = {
    id: UserAttributeKey;
    required: boolean;
}

export type UserAttributes = Array<UserAttribute>;
export type RequestedUserAttributes = Array<RequestedUserAttribute>;

enum EFieldType {
    Identity = 0,
    ForeignIdentity = 1,
    Password = 2,
    SecurePassword = 3,
    Key = 4
};

export interface BaseFieldMetadata {
    fieldType: EFieldType;
    fieldId: string;
    description: string | undefined;
}

export type FieldValue = string | Uint8Array | ArrayBuffer;

export interface FieldValueRejectionReason {
    code: string;
}

enum EMetadataProperty {
    Version = 0,
    Status = 1,
    Category = 2,
    ChangePassUrl = 4
};

enum EMetadataStatus {
    Pending = 0,
    Created = 1,
    Retired = 2,
};

export interface RecordMetadata {
    [EMetadataProperty.Version]: number;
    [EMetadataProperty.Status]: EMetadataStatus;
    [EMetadataProperty.Category]: string;
    [EMetadataProperty.ChangePassUrl]: string;
}

/* Request Arguments */
export type GetSigningKeyArgs = { webAppActor: WebAppActor }
export type GetCredentialsMetadataArgs = {
    webAppActor: WebAppActor,
    accountConstraints: AccountConstraints
};
export type GenerateKeyExchangeParametersArgs = {
    paramsDraft: InitialKeyExchangeParametersDraft
}
export type EnableEndToEndEncryptionArgs = {
    params: KeyExchangeParameters,
    webAppPacketSignature: ArrayBuffer
}
export type ApproveOperationArgs = {
    operation: LoginOperationMetadata
}
export type UpdateMetadataArgs = {
    metadata: RecordMetadata
}
export type ApproveTransitionToAuthOpArgs = {
    newAuthOp: LoginOperationMetadata
}
export type AddFieldArgs = {
    field: BaseFieldMetadata,
    value: FieldValue | null
}
export type RejectFieldValueArgs = {
    field: BaseFieldMetadata,
    reason: FieldValueRejectionReason,
    optionalNewValue: FieldValue | null
}
export type ApproveChallengeRequestArgs = {
    challenge: ChallengeMetadata,
}
export type AbortChallengeArgs = {
    challenge: ChallengeMetadata,
    reason: ChallengeAbortionReason
}
export type CloseChallengeArgs = {
    challenge: ChallengeMetadata,
}
export type RespondToChallengeMessageArgs = {
    challenge: ChallengeMetadata,
    challengeMessage: ChallengeMessage
}
/* Request Arguments */

export interface RequestHandler {
    manager: {
        getSigningKey(
            context: PreliminaryRequestContext,
            args: GetSigningKeyArgs
        ): string;
        getCredentialsMetadata(
            context: PreliminaryRequestContext,
            args: GetCredentialsMetadataArgs
        ): CredentialsMetadata;
    };
    channel: {
        generateKeyExchangeParameters(
            context: RequestContext,
            args: GenerateKeyExchangeParametersArgs
        ): PartialKeyExchangeParametersFromScm;
        enableEndToEndEncryption(
            context: RequestContext,
            args: EnableEndToEndEncryptionArgs
        ): ArrayBuffer;
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
        closeOpeation(
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
            context: RequestContextWithOperation
        ): UserAttributes;
        addField(
            context: RequestContextWithOperation,
            args: AddFieldArgs
        ): void;
        rejectFieldValue(
            context: RequestContextWithOperation,
            args: RejectFieldValueArgs
        ): FieldValue | undefined;
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
            context: RequestContextWithOperation,
            args: RespondToChallengeMessageArgs
        ): ChallengeMessageResponse;
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