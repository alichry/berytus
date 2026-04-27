import { AbstractAccountStageHandler } from "../AbstractAccountHandler";
import type { TypedStageHandler } from "@root/berytus/types";
import { assert, assertIsString } from "../assertions";
import { AuthAccountNotFoundError, AuthIncorrectResponseError, AuthSessionHandler } from "../AuthSessionHandler";
import { E2EEHandler } from "@root/berytus/channel/handlers/E2EEHandler.js";

const version = 3 as const;
const category = "Customer" as const;
const description = "(E2EE) Username Identification and Digital Signature Authentication" as const;
const steps = [
    "createChannel",
    "setupE2EE",
    "login",
    "addFields",
    "validateFields",
    "metadata",
    "save",
    "transitionToAuth",
    "createIdentificationChallenge",
    "identification",
    "createDsChallenge",
    "selectKey",
    "signNonce",
    "finishLogin",
    "closeChannel"
] as const;

const armorKey = (body: ArrayBuffer, type: "PUBLIC" | "PRIVATE" = "PUBLIC") => {
    let res = `-----BEGIN ${type} KEY-----\n`;

    let b64 = (new Uint8Array(body) as any).toBase64({
        alphabet: "base64",
        omitPadding: false
    });

    while(b64.length > 0) {
        res += b64.substring(0, 64) + '\n';
        b64 = b64.substring(64);
    }

    res = res + `-----END ${type} KEY-----`;

    return res;
}

export class CustomerE2EEHandlerV3 extends AbstractAccountStageHandler<typeof steps[number]>
    implements TypedStageHandler<CustomerE2EEHandlerV3> {
    protected authHandler?: AuthSessionHandler;

    public constructor() {
        super(new E2EEHandler());
    }

    get isE2EE() { return true; }

    get version(): number {
        return version;
    }

    get category(): string {
        return category;
    }

    get description(): string {
        return description;
    }

    get steps(): readonly (typeof steps[number])[] {
        return steps;
    }

    async createChannel() {
        //! EXPORT_FN_IGNORE_START
        await this.channelHandler.prepare();
        //! EXPORT_FN_IGNORE_END
        /*! Key-based credential mapping actor */
        const actor = new BerytusCryptoWebAppActor(
            "MCowBQYDK2VwAyEAjTDlbx9pgxXagW81+z+1TyNBqZ1kp715hP8GgH6S9LE="
        );
        //!
        const channel = await BerytusChannel.create({
            webApp: actor,
            constraints: {
                account: {
                    schemaVersion: 3,
                    category: "Customer"
                },
                enableEndToEndEncryption: true
            }
        });
        //!
        //! EXPORT_FN_IGNORE_START
        this.channel = channel;
        await this.channelHandler.init(this.channel);
        return { nextStep: "setupE2EE" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async setupE2EE() {
        //! EXPORT_FN_IGNORE_START
        const getX25519PublicKey = async () => {
            assert(this.channelHandler.kapInput);
            return this.channelHandler.kapInput.public;
        }
        const getUnmaskAllowlist = async () => {
            assert(this.channelHandler.kapInput);
            return this.channelHandler.kapInput.unmaskAllowlist;
        }
        const signKap = async (channel: BerytusChannel, kap: BerytusKeyAgreementParameters) => {
            assert(this.channelHandler instanceof E2EEHandler);
            return await this.channelHandler.signKap();
        };
        const verifyScmKapSignature = async (channel: BerytusChannel, scmSignature: ArrayBuffer) => {
            assert(this.channelHandler instanceof E2EEHandler);
            await this.channelHandler.verifyScmKapSignature(scmSignature);
        }
        const deriveSessionKey = async (channel: BerytusChannel) => {
            assert(this.channelHandler instanceof E2EEHandler);
            await this.channelHandler.deriveSessionKey();
        }
        assert(this.channel);
        //! EXPORT_FN_IGNORE_END
        /*! Use Web app-specific routines getX25519PublicKey
            and getUnmaskAllowlist to prepare KAP. */
        const kap = await this.channel.prepareKeyAgreementParameters({
            public: await getX25519PublicKey(),
            unmaskAllowlist: await getUnmaskAllowlist()
        });
        /*! Use web app-specific routine to sign KAP. The
            signing process occurs in the backend. */
        const signature = await signKap(this.channel, kap);
        /*! Exchange signatures */
        const scmSignature = await this.channel
            .exchangeKeyAgreementSignatures(signature);
        /*! Verify SCM signature using web app-specific routine. The
            verification process occurs in the backend. */
        await verifyScmKapSignature(this.channel, scmSignature);
        /*! Use web app-specific routine to derive session key,
            storing it in the database. */
        await deriveSessionKey(this.channel);
        /*! Finally, enable E2EE */
        await this.channel.enableEndToEndEncryption();
        //! EXPORT_FN_IGNORE_START
        return { nextStep: "login" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async login() {
        //! EXPORT_FN_IGNORE_START
        const channel = this.channel!;
        //! EXPORT_FN_IGNORE_END
        const operation = await channel.login({
            requiredUserAttributes: {
                name: true,
                picture: false,
                gender: true,
                birthdate: true,
                address: true,
            }
        });
        //! EXPORT_FN_IGNORE_START
        this.operation = operation;
        //! EXPORT_FN_IGNORE_END
        if (operation.intent === 'Register') {
            /*! Handle registration operation */
            //! EXPORT_FN_IGNORE_START
            await this.cacheRegistrationFields();
            return { nextStep: "addFields" as const };
            //! EXPORT_FN_IGNORE_END
        } else {
            /*! Handle authentication operation */
            //! EXPORT_FN_IGNORE_START
            return { nextStep: "createIdentificationChallenge" as const };
            //! EXPORT_FN_IGNORE_END
        }
    }

    async addFields() {
        //! EXPORT_FN_IGNORE_START
        const channel = this.channel;
        const operation = this.operation!;
        AbstractAccountStageHandler.assertIsCreationOperation(operation);
        //! EXPORT_FN_IGNORE_END
        const fields = await operation.addFields(
            new BerytusIdentityField(
                'username',
                {
                    private: false,
                    humanReadable: true,
                    maxLength: 24
                }
            ),
            new BerytusKeyField(
                'key',
                { alg: -51 }
            )
        );
        //! EXPORT_FN_IGNORE_START
        await this.cacheRegistrationFields();
        return { nextStep: "validateFields" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async validateFields() {
        //! EXPORT_FN_IGNORE_START
        const operation = this.operation;
        AbstractAccountStageHandler.assertIsCreationOperation(operation);
        const usernameExists = (field: BerytusField) => this.accountExists([field]);
        //! EXPORT_FN_IGNORE_END
        const usernameField = operation.fields.get('username');
        //! EXPORT_FN_IGNORE_START
        if (! usernameField) {
            throw new Error("Expecting username field to be set in validateFields!");
        }
        //! EXPORT_FN_IGNORE_END
        /*!
         * We use a web app-specific routine, `usernameExists`,
         * to check whether the username exists or not.
         * @var usernameExists
         * @type {(field: BerytusIdentityField): Promise<boolean>}
         */
        while (await usernameExists(usernameField)) {
            /*!
             * The provided username is registered, reject it and request
             * a new revision. Once rejectAndReviseFields() resolves,
             * `usernameField.value` reflects the new field value.
             */
            await operation.rejectAndReviseFields({
                field: usernameField,
                reason: "Identity:IdentityAlreadyExists",
                /*!
                 * The web app can propose a revised value by
                 * specifying a `newValue` property here. E.g.,
                 * `newValue: "usernameThatDoesNotExists"`.
                 * Otherwise, the secret manager will produce one
                 * as it is the case here.
                 */
            });
        }
        //! EXPORT_FN_IGNORE_START
        await this.cacheRegistrationFields();
        return { nextStep: "metadata" as const };
        //! EXPORT_FN_IGNORE_END
    }


    async metadata() {
        //! EXPORT_FN_IGNORE_START
        const operation = this.operation;
        AbstractAccountStageHandler.assertIsCreationOperation(operation);
        //! EXPORT_FN_IGNORE_END
        await operation.setCategory("Customer");
        await operation.setVersion(3);
        //! EXPORT_FN_IGNORE_START
        return { nextStep: "save" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async save() {
        //! EXPORT_FN_IGNORE_START
        let operation = this.operation;
        AbstractAccountStageHandler.assertIsCreationOperation(operation);
        const registerAccountInBackEnd = async (
            username: BerytusEncryptedPacket,
            key: BerytusKeyFieldValue,
            attrsMap: BerytusUserAttributeMap
        ) => {
            assert(username instanceof BerytusJWEPacket);
            assert(key.publicKey instanceof BerytusJWEPacket);
            const fields = [{
                id: "username",
                value: username
            },
            {
                id: "key",
                value: {
                    publicKey: key.publicKey instanceof ArrayBuffer
                        ? new Blob([key.publicKey], { type: "application/octet-stream" })
                        : key.publicKey
                }
            }];
            const attrs: Record<string, string | Blob> = {};
            for (const [key, obj] of attrsMap) {
                attrs[key] = typeof obj.value === "string"
                    ? obj.value
                    : obj.value instanceof ArrayBuffer
                    ? new Blob([obj.value], { type: obj.mimeType || undefined })
                    : obj.value;
            }
            return this.createAccount(fields, attrs);
        }
        //! EXPORT_FN_IGNORE_END
        /*!
         * We use a web app-specific routine, `registerAccountInBackEnd`,
         * to register the account in the backend. This dispatches an HTTP
         * request containing the account username and key fields.
         * @var registerAccountInBackEnd
         * @type {(username: BerytusEncryptedPacket, key: BerytusKeyFieldValue, userAttrs: BerytusUserAttributeMap): Promise<void>}
         */
        await registerAccountInBackEnd(
            operation.fields.get('username')!.value as BerytusEncryptedPacket,
            operation.fields.get('key')!.value as BerytusKeyFieldValue,
            operation.userAttributes
        );
        await operation.setStatus("Created");
        await operation.save();
        //! EXPORT_FN_IGNORE_START
        return { "nextStep": "transitionToAuth" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async transitionToAuth() {
        //! EXPORT_FN_IGNORE_START
        let operation = this.operation;
        AbstractAccountStageHandler.assertIsCreationOperation(operation);
        this.loginState.userAttributes = {};
        this.loginState.credentialFields = [];
        this.loginState.identityFields = [];
        //! EXPORT_FN_IGNORE_END
        /*!
         * Here, after saving the account, the web application
         * can turn the account creation operation into an
         * account authentication operation for the saved account.
         */
        operation = await operation.transitionToAuthOperation();
        //! EXPORT_FN_IGNORE_START
        this.operation = operation;
        return { "nextStep": "createIdentificationChallenge" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async createIdentificationChallenge() {
        //! EXPORT_FN_IGNORE_START
        const operation = this.operation;
        AbstractAccountStageHandler.assertIsAuthenticationOperation(operation);
        //! EXPORT_FN_IGNORE_END
        const idCh = new BerytusIdentificationChallenge(
            "id", /*! challenge id */
            { fields: ['username'] } /*! idt fields to retrieve */
        );
        await operation.challenge(idCh);
        //! EXPORT_FN_IGNORE_START
        return { nextStep: "identification" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async identification() {
        //! EXPORT_FN_IGNORE_START
        const operation = this.operation;
        AbstractAccountStageHandler.assertIsAuthenticationOperation(operation);
        const idCh = operation.challenges.get('id') as BerytusIdentificationChallenge;
        if (! idCh) {
            throw new Error("ID challenge not set.");
        }
        const accountExists = async (username: BerytusEncryptedPacket): Promise<boolean> => {
            try {
                this.authHandler = await AuthSessionHandler.create(
                    this.channel!.id,
                    this.version,
                    this.category,
                    [{ id: "username", value: username }]
                );
                return true;
            } catch (e) {
                if (e instanceof AuthAccountNotFoundError) {
                    return false;
                }
                throw e;
            }
        }
        //! EXPORT_FN_IGNORE_END
        const { response: { username } } = await idCh.getIdentityFields();
        //! EXPORT_FN_IGNORE_START
        assert(typeof username !== "string");
        //! EXPORT_FN_IGNORE_END
        /*!
            * We use a web app-specific routine, `accountExists`,
            * to check whether the account exists or not given its username.
            * @var accountExists
            * @type {(username: string): Promise<boolean>}
            */
        if (! await accountExists(username)) {
            await idCh.abortWithIdentityDoesNotExistsError();
            throw new Error("User failed to pass identification challenge");
        }
        await idCh.seal();
        //! EXPORT_FN_IGNORE_START
        this.loginState.identityFields.push({
            id: 'username',
            value: await this.stringifyBerytusValue(username)
        });
        return { nextStep: "createDsChallenge" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async createDsChallenge() {
        //! EXPORT_FN_IGNORE_START
        const operation = this.operation;
        AbstractAccountStageHandler.assertIsAuthenticationOperation(operation);
        //! EXPORT_FN_IGNORE_END
        const dsCh = new BerytusDigitalSignatureChallenge(
            "ds", /*! challenge id */
            { field: "key" } /*! key field to assume */
        );
        await operation.challenge(dsCh)
        //! EXPORT_FN_IGNORE_START
        return { nextStep: "selectKey" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async selectKey() {
        //! EXPORT_FN_IGNORE_START
        const operation = this.operation;
        AbstractAccountStageHandler.assertIsAuthenticationOperation(operation);
        const dsCh = operation.challenges.get('ds') as BerytusDigitalSignatureChallenge;
        assert(!!dsCh);
        const username = null;
        const validateKey = async (username: null, key: BerytusKeyFieldValue) => {
            if (!this.authHandler) {
                throw new Error("Expecting authHandler to be set.");
            }
            try {
                const chParams = await this.authHandler.newChallenge(
                    "digital-signature"
                );
                if (!("field" in chParams) || chParams.field !== "key") {
                    throw new Error(
                        "Inconsistency between client and server challenge parameters"
                    );
                }
                assert(key.publicKey instanceof BerytusJWEPacket);
                await this.authHandler.sendResponse({
                    id: "key",
                    value: {
                        publicKey: key.publicKey
                    }
                }, "multipart");
                return true;
            } catch (e) {
                if (e instanceof AuthIncorrectResponseError) {
                    return false;
                }
                throw e;
            }
        };
        //! EXPORT_FN_IGNORE_END
        const { response: key } = await dsCh.selectKey();
        /*!
         * We use a web app-specific routine, `validateKey`, to
         * check whether the secret manager-suppied public key
         * is valid and associated with the account identified
         * by `username`.
         * @var validateKey
         * @type {(username: string, key: BerytusKeyFieldValue): Promise<boolean>}
         */
        if (! await validateKey(username, key)) {
            await dsCh.abortWithPublicKeyMismatchError();
            throw new Error(
                "User failed to pass the digital signature challenge; " +
                "reason: Publiic Key Mismatch."
            );
        }
        //! EXPORT_FN_IGNORE_START
        assert(key.publicKey instanceof BerytusJWEPacket);
        this.loginState.credentialFields.push({
            id: 'key',
            value: key.publicKey instanceof ArrayBuffer
                ? armorKey(key.publicKey)
                : await this.stringifyBerytusValue(key.publicKey)
        });
        return { nextStep: "signNonce" as const }
        //! EXPORT_FN_IGNORE_END
    }

    async signNonce() {
        //! EXPORT_FN_IGNORE_START
        const operation = this.operation;
        AbstractAccountStageHandler.assertIsAuthenticationOperation(operation);
        const dsCh = operation.challenges.get('ds') as BerytusDigitalSignatureChallenge;
        assert(!!dsCh);
        const generateNonce = async (): Promise<BerytusEncryptedPacket> => {
            assert(!!this.authHandler);
            const { nextMessage: {
                request: nonceJwe
            } } = await this.authHandler.pendingMessage();
            return new BerytusJWEPacket(nonceJwe);
        }
        const verifySignature = async (signature: BerytusEncryptedPacket): Promise<boolean> => {
            assert(!!this.authHandler);
            try {
                await this.authHandler.sendResponse(
                    signature,
                    "blob"
                );
                const res = await this.authHandler.finish();
                res.identity.forEach(idf => {
                    this.loginState.identityFields.push({
                        id: `${idf.id}.cleartext`,
                        value: idf.value
                    });
                });
                await Promise.all(res.userAttributes.map(async u => {
                    this.loginState.userAttributes[u.id] =
                        await this.stringifyBerytusValue(u.value);
                }));
                return true;
            } catch (e) {
                if (e instanceof AuthIncorrectResponseError) {
                    return false;
                }
                throw e;
            }
        }
        //! EXPORT_FN_IGNORE_END

        /*!
         * We use a web app-specific routine, `generateNonce`, to
         * generate a nonce for the secret manager to sign.
         * @var generateNonce
         * @type {(): Promise<BerytusEncryptedPacket>}
         */
        const nonce = await generateNonce();
        const { response: signature } = await dsCh.signNonce(nonce);
        //! EXPORT_FN_IGNORE_START
        assert(!(signature instanceof ArrayBuffer));
        //! EXPORT_FN_IGNORE_END
        /*!
         * We use a web app-specific routine, `verifySignature`, to
         * validate the signature.
         * @var verifySignature
         * @type {(signature: BerytusEncryptedPacket): Promise<boolean>}
         */
        if (!(await verifySignature(signature))) {
            await dsCh.abortWithInvalidSignatureError();
            throw new Error(
                "User failed to pass the digital signature " +
                "challenge; reason: Invalid Signature."
            );
        }
        await dsCh.seal();
        //! EXPORT_FN_IGNORE_START
        return { nextStep: "finishLogin" as const }
        //! EXPORT_FN_IGNORE_END
    }

    async finishLogin() {
        //! EXPORT_FN_IGNORE_START
        const operation = this.operation;
        AbstractAccountStageHandler.assertIsAuthenticationOperation(operation);
        //! EXPORT_FN_IGNORE_END
        await operation.finish();
        //! EXPORT_FN_IGNORE_START
        return { nextStep: "closeChannel" as const }
        //! EXPORT_FN_IGNORE_END
    }

    async closeChannel() {
        //! EXPORT_FN_IGNORE_START
        const channel = this.channel;
        if (! channel) {
            throw new Error("Expecting channel to be set during closeChannel");
        }
        //! EXPORT_FN_IGNORE_END
        await channel.close();
        await this.channelHandler.close();
        //! EXPORT_FN_IGNORE_START
        return { finished: true as const }
        //! EXPORT_FN_IGNORE_END
    }
}
