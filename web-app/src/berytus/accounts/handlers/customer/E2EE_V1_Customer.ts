import { AbstractAccountStageHandler } from "../AbstractAccountHandler";
import type { TypedStageHandler } from "@root/berytus/types";
import { AuthAccountNotFoundError, AuthIncorrectResponseError, AuthSessionHandler } from "../AuthSessionHandler";
import { assert } from "../assertions";
import { E2EEHandler } from "@root/berytus/channel/handlers/E2EEHandler.js";

const version = 1 as const;
const category = "Customer" as const;
const description = "(E2EE) Username Identification and Password Authentication" as const;
const steps = [
    "createChannel",
    "setupE2EE",
    "login",
        "addFields",
        "validateFields",
        "metadata",
        "save",
        "transitionToAuth",
        //
        "createIdentificationChallenge",
        "identification",
        "createPasswordChallenge",
        "passwordAuth",
        "finishLogin",
        "closeChannel",
    ] as const;

export class CustomerE2EEHandlerV1 extends AbstractAccountStageHandler<typeof steps[number]>
    implements TypedStageHandler<CustomerE2EEHandlerV1> {
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
        assert(this.channelHandler.webAppActor);
        assert('ed25519Key' in this.channelHandler.webAppActor);
        const ed25519Key = this.channelHandler.webAppActor.ed25519Key;
        const getEd25519Key = async () => ed25519Key;
        //! EXPORT_FN_IGNORE_END
        /*! Key-based credential mapping actor */
        const actor = new BerytusCryptoWebAppActor(
            await getEd25519Key() //! e.g., MCowBQY...
        );
        //!
        const channel = await BerytusChannel.create({
            webApp: actor,
            constraints: {
                account: {
                    schemaVersion: 1,
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
        const channel = this.channel;
        if (! channel) {
            throw new Error("Expecting channel to be set in login!");
        }
        //! EXPORT_FN_IGNORE_END
        let operation = await channel.login({
            requiredUserAttributes: {
                name: true,
                picture: false,
                gender: true,
                birthdate: true,
                address: true,
            },
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
        return { nextStep: "createPasswordChallenge" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async createPasswordChallenge() {
        //! EXPORT_FN_IGNORE_START
        const operation = this.operation;
        AbstractAccountStageHandler.assertIsAuthenticationOperation(operation);
        //! EXPORT_FN_IGNORE_END
        const passCh = new BerytusPasswordChallenge(
            "pass", /*! challenge id */
            { fields: ["password"] } /*! pwd fields to retrieve */
        );
        await operation.challenge(passCh)
        //! EXPORT_FN_IGNORE_START
        return { nextStep: "passwordAuth" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async passwordAuth() {
        //! EXPORT_FN_IGNORE_START
        const operation = this.operation;
        AbstractAccountStageHandler.assertIsAuthenticationOperation(operation);
        const passCh = operation.challenges.get('pass') as BerytusPasswordChallenge;
        if (! passCh) {
            throw new Error("Pass challenge not set.");
        }
        const username = null;
        const login = async (username: null, password: BerytusEncryptedPacket) => {
            if (!this.authHandler) {
                throw new Error("Expecting authHandler to be set.");
            }
            try {
                const chParams = await this.authHandler.newChallenge(
                    "password"
                );
                if (
                    !("fields" in chParams) ||
                    ! Array.isArray(chParams.fields) ||
                    chParams.fields.length !== 1 ||
                    chParams.fields[0] !== "password"
                ) {
                    throw new Error("Inconsistency between client and server challenge parameters");
                }
                await this.authHandler.sendResponse([
                    {
                        id: "password",
                        value: password
                    }
                ], "multipart");
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
        };
        //! EXPORT_FN_IGNORE_END
        const { response: { password } } = await passCh.getPasswordFields();
        //! EXPORT_FN_IGNORE_START
        assert(typeof password !== "string");
        //! EXPORT_FN_IGNORE_END
        /*!
         * We use a web app-specific routine, `login`, for password
         * authentication.
         * @var login
         * @type {(username: string, password: string): Promise<boolean>}
         */
        if (! await login(username, password)) {
            await passCh.abort("IncorrectPassword");
            throw new Error("User failed to pass the password challenge");
        }
        await passCh.seal();

        //! EXPORT_FN_IGNORE_START
        this.loginState.credentialFields.push({
            id: 'password',
            value: await this.stringifyBerytusValue(password)
        });
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
        //! EXPORT_FN_IGNORE_START
        await this.channelHandler.close();
        return { finished: true as const }
        //! EXPORT_FN_IGNORE_END
    }

    async addFields() {
        //! EXPORT_FN_IGNORE_START
        const operation = this.operation;
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
            new BerytusPasswordField(
                'password'
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
        await operation.setVersion(1);
        //! EXPORT_FN_IGNORE_START
        return { nextStep: "save" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async save() {
        //! EXPORT_FN_IGNORE_START
        let operation = this.operation;
        AbstractAccountStageHandler.assertIsCreationOperation(operation);
        const registerAccountInBackEnd = (
            username: BerytusEncryptedPacket,
            password: BerytusEncryptedPacket,
            attrsMap: BerytusUserAttributeMap
        ) => {
            const fields = [{
                id: "username",
                value: username
            },
            {
                id: "password",
                value: password
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
         * request containing the account username and password fields.
         * @var registerAccountInBackEnd
         * @type {(username: BerytusEncryptedPacket, password: BerytusEncryptedPacket, userAttrs: BerytusUserAttributeMap): Promise<void>}
         */
        await registerAccountInBackEnd(
            operation.fields.get('username')!.value as BerytusEncryptedPacket,
            operation.fields.get('password')!.value as BerytusEncryptedPacket,
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
}