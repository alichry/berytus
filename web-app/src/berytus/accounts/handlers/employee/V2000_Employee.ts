import { E2EEHandler } from "@root/berytus/channel/handlers/E2EEHandler.js";
import { AbstractAccountStageHandler } from "../AbstractAccountHandler.js";
import type { TypedStageHandler } from "@root/berytus/types";
import { assert } from "../assertions.js";

const version = 2000 as const;
const category = "Employee" as const;
const description = "(E2EE) Composite Username Identification and Password Authentication" as const;
const steps = [
    "createChannel",
    "setupE2EE",
    "login",
    "addFields",
    "save"
] as const;

export class EmployeeHandlerV2000 extends AbstractAccountStageHandler<typeof steps[number]>
    implements TypedStageHandler<EmployeeHandlerV2000> {

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
        //! EXPORT_FN_IGNORE_END
        const partyId = "1234-5678" // assume we are registering accounts
                                    // underr party 1234-5678
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
                { identityFieldId: "partyId" }
            )
        );
        //! EXPORT_FN_IGNORE_START
        return { nextStep: "save" as const };
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
        if (! partyIdField) {
            throw new Error("Expecting partyId field to be set in validateFields!");
        }
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
        while (await accountExists(partyIdField, usernameField)) {
            /*!
             * The provided username is registered under the party, reject it and request
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

    async save() {
        return { finished: true as const }
    }
}
