import { AbstractAccountStageHandler } from "../AbstractAccountHandler";
import type { TypedStageHandler } from "@root/berytus/types";
import { AuthAccountNotFoundError, AuthIncorrectResponseError, AuthSessionHandler } from "../AuthSessionHandler";
import { assert, assertIsString } from "../assertions";

const version = 1 as const;
const category = "Customer" as const;
const description = "Username Identification and Password Authentication" as const;
const steps = [
    "createChannel",
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

export class CustomerHandlerV1 extends AbstractAccountStageHandler<typeof steps[number]>
    implements TypedStageHandler<CustomerHandlerV1> {
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
                    schemaVersion: 1,
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
            await idCh.abort("IdentityDoesNotExists");
            throw new Error("User failed to pass identification challenge");
        }
        await idCh.seal();
        //! EXPORT_FN_IGNORE_START
        this.loginState.identityFields.push({
            id: 'username',
            value: username
        });
        return { nextStep: "createPasswordChallenge" as const };
        //! EXPORT_FN_IGNORE_END
    }

    async createPasswordChallenge() {
        //! EXPORT_FN_IGNORE_START
        const operation = this.operation;
        AbstractAccountStageHandler.assertIsAuthenticationOperation(operation);
        //! EXPORT_FN_IGNORE_END
        const passCh = new BerytusPasswordChallenge("pass");
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
        const username = "";
        const login = async (username: string, password: string) => {
            if (!this.authHandler) {
                throw new Error("Expecting authHandler to be set.");
            }
            try {
                await this.authHandler.newChallenge(
                    "password"
                );
                await this.authHandler.sendResponse([
                    {
                        id: "password",
                        value: password
                    }
                ]);
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
        };
        //! EXPORT_FN_IGNORE_END
        const { response: { password } } = await passCh.getPasswordFields(['password']);
        //! EXPORT_FN_IGNORE_START
        assertIsString(password);
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
            value: password
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
            username: string,
            password: string,
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
         * request containing the account username and password fields.
         * @var registerAccountInBackEnd
         * @type {(username: string, password: string): Promise<void>}
         */
        await registerAccountInBackEnd(
            operation.fields.get('username')!.value as string,
            operation.fields.get('password')!.value as string,
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
}