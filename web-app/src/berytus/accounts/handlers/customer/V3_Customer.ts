import { AbstractAccountStageHandler } from "../AbstractAccountHandler";
import type { TypedStageHandler } from "@root/berytus/types";
import { assert, assertIsString } from "../assertions";
import { AuthAccountNotFoundError, AuthIncorrectResponseError, AuthSessionHandler } from "../AuthSessionHandler";

const version = 3 as const;
const category = "Customer" as const;
const description = "Username Identification and Digital Signature Authentication" as const;
const steps = ["createChannel", "login", "addFields", "validateFields", "metadata", "save", "transitionToAuth", "createIdentificationChallenge", "identification", "createDsChallenge", "selectKey", "signNonce", "finishLogin", "closeChannel"] as const;

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

export class CustomerHandlerV3 extends AbstractAccountStageHandler<typeof steps[number]>
    implements TypedStageHandler<CustomerHandlerV3> {
    protected authHandler?: AuthSessionHandler;

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
        /*! Domain-based credential mapping actor */
        const actor = new BerytusAnonymousWebAppActor();
        //!
        const channel = await BerytusChannel.create({
            webApp: actor,
            constraints: {
                account: {
                    schemaVersion: 3,
                    category: "Customer"
                }
            }
        });
        //!
        //! EXPORT_FN_IGNORE_START
        this.channel = channel;
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
        return { nextStep: "validateFields" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async validateFields() {
        //! EXPORT_FN_IGNORE_START
        const operation = this.operation;
        AbstractAccountStageHandler.assertIsCreationOperation(operation);
        const usernameExists = this.accountExists.bind(this);
        //! EXPORT_FN_IGNORE_END
        const usernameField = operation.fields.get('username');
        if (! usernameField) {
            throw new Error("Expecting username field to be set in validateFields!");
        }
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
        const registerAccountInBackEnd = (
            username: string,
            key: BerytusKeyFieldValue,
            attrsMap: BerytusUserAttributeMap
        ) => {
            assert(key.publicKey instanceof ArrayBuffer);
            const fields = [{
                id: "username",
                value: username
            },
            {
                id: "key",
                value: {
                    publicKey: armorKey(key.publicKey)
                }
            }];
            const attrs: Record<string, string> = {};
            for (const [key, obj] of attrsMap) {
                attrs[key] = typeof obj.value === "string"
                    ? obj.value
                    : JSON.stringify(obj.value)
            }
            return this.createAccount(fields, attrs);
        }
        //! EXPORT_FN_IGNORE_END
        /*!
         * We use a web app-specific routine, `registerAccountInBackEnd`,
         * to register the account in the backend. This dispatches an HTTP
         * request containing the account username and key fields.
         * @var registerAccountInBackEnd
         * @type {(username: string, key: BerytusKeyFieldValue): Promise<void>}
         */
        await registerAccountInBackEnd(
            operation.fields.get('username')!.value as string,
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
        const idCh = new BerytusIdentificationChallenge("id");
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
        const accountExists = async (username: string): Promise<boolean> => {
            try {
                this.authHandler = await AuthSessionHandler.create(
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
        const { response: { username } } = await idCh.getIdentityFields(['username']);
        //! EXPORT_FN_IGNORE_START
        assertIsString(username);
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
            value: username
        });
        return { nextStep: "createDsChallenge" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async createDsChallenge() {
        //! EXPORT_FN_IGNORE_START
        const operation = this.operation;
        AbstractAccountStageHandler.assertIsAuthenticationOperation(operation);
        //! EXPORT_FN_IGNORE_END
        const dsCh = new BerytusDigitalSignatureChallenge("ds");
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
        const username = "";
        const validateKey = async (username: string, key: BerytusKeyFieldValue) => {
            if (!this.authHandler) {
                throw new Error("Expecting authHandler to be set.");
            }
            try {
                await this.authHandler.newChallenge(
                    "digital-signature"
                );
                assert(key.publicKey instanceof ArrayBuffer);
                await this.authHandler.sendResponse({
                    id: "key",
                    value: {
                        publicKey: armorKey(key.publicKey)
                    }
                });
                // const res= await this.authHandler.finish();
                // res.userAttributes.forEach(u => {
                //     this.loginState.userAttributes[u.id] = u.value;
                // });
                return true;
            } catch (e) {
                if (e instanceof AuthIncorrectResponseError) {
                    return false;
                }
                throw e;
            }
        };
        //! EXPORT_FN_IGNORE_END
        const { response: key } = await dsCh.selectKey('key');
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
        assert(key.publicKey instanceof ArrayBuffer);
        this.loginState.credentialFields.push({
            id: 'key',
            value: armorKey(key.publicKey)
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
        const generateNonce = async (): Promise<ArrayBuffer> => {
            assert(!!this.authHandler);
            const { nextMessage: {
                request: nonceB64
            } } = await this.authHandler.pendingMessage();
            const byteArray: Uint8Array =
                // @ts-ignore: fromBase64 is Firefox only.
                Uint8Array.fromBase64(
                    nonceB64,
                    {
                        alphabet: "base64"
                    }
                );
            // @ts-ignore: TODO(berytus): Check if views can be passed
            return byteArray.buffer;
        }
        const verifySignature = async (signature: ArrayBuffer): Promise<boolean> => {
            assert(!!this.authHandler);
            try {
                await this.authHandler.sendResponse(
                    new Uint8Array(signature)
                        // @ts-ignore: Firefox only.
                        .toBase64({
                            alphabet: "base64",
                            omitPadding: false
                        })
                )
                const res= await this.authHandler.finish();
                res.userAttributes.forEach(u => {
                    this.loginState.userAttributes[u.id] = u.value;
                });
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
         * @type {(): Promise<ArrayBuffer>}
         */
        const nonce = await generateNonce();
        const { response: signature } = await dsCh.signNonce(nonce);
        /*!
         * We use a web app-specific routine, `verifySignature`, to
         * validate the signature.
         * @var verifySignature
         * @type {(signature: ArrayBuffer): Promise<boolean>}
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
        //! EXPORT_FN_IGNORE_START
        return { finished: true as const }
        //! EXPORT_FN_IGNORE_END
    }
}
