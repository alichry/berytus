import { E2EEHandler } from "@root/berytus/channel/handlers/E2EEHandler.js";
import { AbstractAccountStageHandler } from "../AbstractAccountHandler.js";
import { AuthAccountNotFoundError, AuthIncorrectResponseError, AuthSessionHandler } from "../AuthSessionHandler.js";
import type { TypedStageHandler } from "@root/berytus/types";
import { assert } from "../assertions.js";
import { FetchError } from "@root/backend/errors/FetchError.js";

const version = 2000 as const;
const category = "Employee" as const;
const description = "(E2EE) Composite Username Identification and Secure Password Authentication" as const;
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
    "createSrpChallenge",
    "selectSecurePassword",
    "exchangePublicKeys",
    "computeClientProof",
    "verifyServerProof",
    "finishLogin",
    "closeChannel"
] as const;

export class EmployeeHandlerV2000 extends AbstractAccountStageHandler<typeof steps[number]>
    implements TypedStageHandler<EmployeeHandlerV2000> {
    protected authHandler?: AuthSessionHandler;

    public constructor() {
        super(new E2EEHandler());
    }

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
        assert(ed25519Key === 'MCowBQYDK2VwAyEAjTDlbx9pgxXagW81+z+1TyNBqZ1kp715hP8GgH6S9LE=');
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
                    schemaVersion: 2000,
                    category: "Employee"
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
            return { nextStep: "addFields" as const };
            //! EXPORT_FN_IGNORE_END
        } else {
            /*! Handle authentication operation */
            //! EXPORT_FN_IGNORE_START
            return { nextStep: "addFields" as const };
            //! EXPORT_FN_IGNORE_END
        }
    }

    async addFields() {
        //! EXPORT_FN_IGNORE_START
        const channel = this.channel;
        const actor = channel!.webApp;
        const operation = this.operation!;
        AbstractAccountStageHandler.assertIsCreationOperation(operation);
        const getEncryptedPartyId = async () => {
            const resp = await fetch(
                `/channel/${this.channel!.id}/login/${this.category}/${this.version}/constants`,
                {
                    method: "POST",
                    body: JSON.stringify(["partyId.ClassA"])
                }
            );
            if (! resp.ok) {
                throw new FetchError(resp, 'failed to retrieve encrypted partyId');
            }
            const body = await resp.json();
            if (!("partyId.ClassA" in body)) {
                throw new Error(
                    "Expected partyId.ClassA to exist in constants dictionary, got otherwise."
                );
            }
            const partyId = body["partyId.ClassA"];
            return new BerytusJWEPacket(partyId);
        }
        //! EXPORT_FN_IGNORE_END
        const partyId = await getEncryptedPartyId();
        //! assume we are registering accounts
        //! under party 0001.
        console.assert(partyId instanceof BerytusJWEPacket);
        const fields = await operation.addFields(
            new BerytusIdentityField(
                'partyId',
                {
                    private: false,
                    humanReadable: false,
                    maxLength: 24
                },
                partyId
            ),
            new BerytusIdentityField(
                'username',
                {
                    private: false,
                    humanReadable: true,
                    maxLength: 24
                }
            ),
            new BerytusSecurePasswordField(
                'securePassword',
                { identityFieldId: "username" }
            )
        );
        //! EXPORT_FN_IGNORE_START
        return { nextStep: "validateFields" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async validateFields() {
        //! EXPORT_FN_IGNORE_START
        const operation = this.operation;
        AbstractAccountStageHandler.assertIsCreationOperation(operation);
        const accountExists = (partyId: BerytusField, username: BerytusField) =>
            this.accountExists([partyId, username]);
        //! EXPORT_FN_IGNORE_END
        const partyIdField = operation.fields.get('partyId');
        //! EXPORT_FN_IGNORE_START
        if (! partyIdField) {
            throw new Error("Expecting partyId field to be set in validateFields!");
        }
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
        while (await accountExists(partyIdField, usernameField)) {
            /*!
             * The provided username is registered under the party,
             * reject it and request a new revision. Once
             * rejectAndReviseFields() resolves,
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
            partyId: BerytusEncryptedPacket,
            username: BerytusEncryptedPacket,
            securePassword: BerytusSecurePasswordFieldValue,
            attrsMap: BerytusUserAttributeMap
        ) => {
            const fields = [
                {
                    id: "partyId",
                    value: partyId
                },
                {
                    id: "username",
                    value: username
                },
                {
                    id: "securePassword",
                    value: {
                        salt: securePassword.salt instanceof ArrayBuffer
                            ? new Blob([securePassword.salt], { type: "application/octet-stream" })
                            : securePassword.salt,
                        verifier: securePassword.verifier instanceof ArrayBuffer
                            ? new Blob([securePassword.verifier], { type: "application/octet-stream" })
                            : securePassword.verifier,
                    }
                }
            ];
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
        const partyIdField = operation.fields.get('partyId');
        //! EXPORT_FN_IGNORE_START
        if (! partyIdField) {
            throw new Error("Expecting partyId field to be set in save()!");
        }
        //! EXPORT_FN_IGNORE_END
        const usernameField = operation.fields.get('username');
        //! EXPORT_FN_IGNORE_START
        if (! usernameField) {
            throw new Error("Expecting username field to be set in save()!");
        }
        //! EXPORT_FN_IGNORE_END
        const securePasswordField = operation.fields.get('securePassword');
        //! EXPORT_FN_IGNORE_START
        if (! securePasswordField) {
            throw new Error("Expecting securePassword field to be set in save()!");
        }
        //! EXPORT_FN_IGNORE_END
        /*!
         * We use a web app-specific routine, `registerAccountInBackEnd`,
         * to register the account in the backend. This dispatches an HTTP
         * request containing the account username and password fields.
         * @var registerAccountInBackEnd
         * @type {(username: string, password: string): Promise<void>}
         */
        await registerAccountInBackEnd(
            partyIdField.value as BerytusEncryptedPacket,
            usernameField.value as BerytusEncryptedPacket,
            securePasswordField.value as BerytusSecurePasswordFieldValue,
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
            { fields: ['partyId', 'username'] } /*! idt fields to retrieve */
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
        const accountExists = async (
            partyId: BerytusEncryptedPacket,
            username: BerytusEncryptedPacket
        ): Promise<boolean> => {
            try {
                this.authHandler = await AuthSessionHandler.create(
                    this.channel!.id,
                    this.version,
                    this.category,
                    [
                        { id: "partyId", value: partyId },
                        { id: "username", value: username }
                    ]
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
        const { response: { partyId, username } } = await idCh.getIdentityFields();
        //! EXPORT_FN_IGNORE_START
        if (typeof partyId === "string") {
            throw new Error("Expected partyId to be encrypted");
        }
        if (typeof username === "string") {
            throw new Error("Expected username to be encrypted");
        }
        //! EXPORT_FN_IGNORE_END
        /*!
            * We use a web app-specific routine, `accountExists`,
            * to check whether the account exists or not given its username.
            * @var accountExists
            * @type {(username: string): Promise<boolean>}
            */
        if (! await accountExists(partyId, username)) {
            await idCh.abortWithIdentityDoesNotExistsError();
            throw new Error("User failed to pass identification challenge");
        }
        await idCh.seal();
        //! EXPORT_FN_IGNORE_START
        this.loginState.identityFields.push({
            id: 'partyId',
            value: await partyId.text()
        });
        this.loginState.identityFields.push({
            id: 'username',
            value: await username.text()
        });
        return { nextStep: "createSrpChallenge" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async selectSecurePassword() {
        //! EXPORT_FN_IGNORE_START
        return { nextStep: "exchangePublicKeys" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async exchangePublicKeys() {
        //! EXPORT_FN_IGNORE_START
        return { nextStep: "computeClientProof" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async computeClientProof() {
        //! EXPORT_FN_IGNORE_START
        return { nextStep: "verifyServerProof" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async verifyServerProof() {
        //! EXPORT_FN_IGNORE_START
        return { nextStep: "finishLogin" as const };
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
