import Alpine from "alpinejs";
import { extractFunctionCode } from './code-utils';
import * as prettier from 'prettier';
import prettierPluginBabel from 'prettier/plugins/babel';
import prettierPluginEstree from 'prettier/plugins/estree';

export function computeHeight(code: string): `${number}px` {
    // 89px per line
    // 129.8px for the createChannel block text container
    // 5/6 lines. 21.633 px per line.
    // add 48 + 18 px for y-padding
    // 251.6px for 11/12 lines
    return (code.split('\n').length * 21.55 + 48 + 18) + 'px' as `${number}px`;
}

async function exportSampleCode(fn: Function) {
    let code = extractFunctionCode(fn);
    code = code
        .replaceAll(
            /^[ \t]*\/\/![ \t]*EXPORT_FN_IGNORE_START(\n|.)*?\/\/![ \t]*EXPORT_FN_IGNORE_END[^\n]*/gm,
            ''
        )
        .replaceAll(/^[ \t]*\/\/![ \t]*$/gm, '')
        .trim();
    return prettier.format(code, {
        //parser: 'babel',
        parser: 'babel',
        plugins: [prettierPluginBabel, prettierPluginEstree],
        printWidth: 60,
        tabWidth: 4,
        semi: true
    })
}

interface Steps {
    channel: ['createChannel'],
    login: ['login'],
    register_addFields: ['register_addUserPass'],
    auth_getIds: ['auth_getIds'],
    register_save: ['register_save'],
}

type StepKey = keyof Steps;
type Option = Steps[keyof Steps] extends Array<infer P> ? P : never;

interface CodeBlockMetadata {
    code: string;
    title: string;
    subtitle?: string;
    width: `${number}px`;
    height: `${number}px`;
}

interface StepWalker {
    channel?: BerytusChannel;
    actor?: BerytusWebAppActor;
    operation?: BerytusLoginOperation;

    ready: boolean;
    started: boolean;
    finished: boolean;
    step: keyof Steps;
    option: Option;
    steps: { [key in StepKey]: Steps[key] };
    loading: boolean;
    currentBlock: CodeBlockMetadata;

    start(): void;

    canGoNext(): boolean;
    canGoPrev(): boolean;
    previousOption(): void;
    nextOption(): void;

    setNext(option: Option): void; // set the desired option in the current step.
    continue(): void;

    getBlock(option?: Option): CodeBlockMetadata;
    getAllBlocks(): Array<CodeBlockMetadata>;
}


interface StepWalkerPrivate {
    optionChanged: boolean;
    _code?: Record<Option, string>;
    _codeBlockMetadata: Record<Option, Omit<CodeBlockMetadata, 'code'>>;
    _getFn(option?: Option): Function;
    _nextResolve?(value?: unknown): void;
    _call(): Promise<void>;
    _goToStep(step: StepKey): Promise<void>;
    _finish(): void;
}

type StepWalkerMethods = {
    [fn in Option]: () => Promise<void>;
}

export type StepWalkerStore = StepWalker & StepWalkerMethods
type _StepWalkerStore = StepWalkerStore & StepWalkerPrivate;

const store: _StepWalkerStore & {  init(): void } = {
    _nextResolve: undefined,
    actor: undefined,
    channel: undefined,
    operation: undefined,

    ready: false,
    started: false,
    finished: false,
    loading: false,
    step: 'channel',
    option: 'createChannel',
    steps: {
        channel: ['createChannel'],
        login: ['login'],
        register_addFields: ['register_addUserPass'],
        auth_getIds: ['auth_getIds'],
        register_save: ['register_save'],
    },

    _code: undefined,
    _codeBlockMetadata: {
        createChannel: {
            title: "Create a channel with an anonymous actor (domain-based credential mapping",
            width: '500px',
            height: '192px'
        },
        login: {
            title: "Test",
            width: '500px',
            height: '293px'
        },
        register_addUserPass: {
            title: "Test",
            width: '500px',
            height: '293px'
        },
        register_save: {
            title: "Test",
            width: '500px',
            height: '293px'
        },
        auth_getIds: {
            title: "Test",
            width: '443px',
            height: '150px'
        }
    },

    optionChanged: false,

    async init() {
        const codeblocks: Partial<Record<Option, string>> = {};
        const allOptions = Object.values(this.steps).reduce((val, curr) => {
            return [...val, ...curr];
        }, [] as Array<Option>);
        const promises = allOptions.map(
            opt =>
                exportSampleCode(this[opt])
                    .then((code) => {
                        codeblocks[opt] = code;
                        this._codeBlockMetadata[opt].height = computeHeight(code);
                    })
        );
        await Promise.all(promises)
        this._code = codeblocks as Record<Option, string>;
        this.ready = true;
    },

    _finish() {
        this.loading = false;
        this.finished = true;
    },

    _getFn(option?: Option): Function {
        if (! option) {
            option = this.option;
        }
        const ops: Array<string> = this.steps[this.step];
        if (ops.indexOf(option) === -1) {
            throw new Error(`Invalid option ${option} at step ${this.step}`);
        }
        if (!this[option]) {
            throw new Error(`Function/option ${option} does not exists!`);
        }
        return this[option];
    },

    getBlock(option?: Option) {
        if (! option) {
            option = this.option;
        }
        return {
            ...this._codeBlockMetadata[option],
            code: this._code ? this._code[option] : 'Loading...'
        };
    },

    getAllBlocks() {
        const ops = this.steps[this.step];
        return ops.map(op => this.getBlock(op));
    },

    canGoNext() {
        const ops = this.steps[this.step] as Array<string>;
        const currIndex = ops.indexOf(this.option);
        return currIndex < ops.length;
    },

    canGoPrev() {
        const ops = this.steps[this.step] as Array<string>;
        const currIndex = ops.indexOf(this.option);
        return currIndex > 0;
    },

    previousOption() {
        const ops = this.steps[this.step] as Array<string>;
        const currIndex = ops.indexOf(this.option);
        const newOption = ops[currIndex - 1 % ops.length] as Option;
        this.setNext(newOption);
    },

    nextOption() {
        const ops = this.steps[this.step] as Array<string>;
        const currIndex = ops.indexOf(this.option);
        const newOption = ops[(currIndex + 1) % ops.length] as Option;
        this.setNext(newOption);
    },

    setNext(option: Option) {
        const ops: Array<string> = this.steps[this.step];
        if (ops.indexOf(option) === -1) {
            throw new Error(
                `Invalid option '${option}' to set at step '${this.step}'`
            );
        }
        this.option = option;
        this.optionChanged = true;
    },

    get currentBlock() {
        return this.getBlock();
    },

    start() {
        if (this.started) {
            throw new Error('Cannot start again.');
        }
        this.started = true;
        this._goToStep(this.step);
    },

    continue() {
        this.loading = true;
        if (! this.started) {
            this.start();
            return;
        }
        if (! this._nextResolve) {
            throw new Error('next() error! _nextResolve is undefined!');
        }
        this._nextResolve();
        // this._call()
        //     .catch(err => console.error('An error has occurred:', err));
    },

    async _call() {
        if (this.optionChanged === false) {
            throw new Error('Cant call next again. Already called. This could be because continue() called before setNext().');
        }
        const currentFn = this._getFn().bind(this);
        const promise = currentFn();
        this.optionChanged = false;
        await promise;
    },

    async _goToStep(step: keyof typeof this.steps) {
        // might be a good idea to check for duped calls to _goToStep
        this.step = step;
        this.setNext(this.steps[this.step][0]);
        this.loading = false;
        return new Promise((resolve, reject) => {
            this._nextResolve = async () => {
                try {
                    await this._call();
                } catch (err) {
                    console.error('An error has occurred:', err);
                }
                resolve();
            };
        });
    },

    async register_save() {
        //! EXPORT_FN_IGNORE_START
        const channel = this.channel!;
        const actor = this.actor!;
        const operation = this.operation! as BerytusAccountCreationOperation;
        //! EXPORT_FN_IGNORE_END
        /*!
         * Here, we should send the account details
         * to the backend and upon a successful
         * account creation in the web app's backend,
         * we should call operation.save();
         * However, since we are still developing the
         * backend this is currently commentted out.
         * Please check back later.
         */
        //! opeation.save();
        //! EXPORT_FN_IGNORE_START
        this._finish();
        //! EXPORT_FN_IGNORE_END

    },

    async register_addUserPass() {
        //! EXPORT_FN_IGNORE_START
        const channel = this.channel!;
        const actor = this.actor!;
        const operation = this.operation! as BerytusAccountCreationOperation;
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
        await this._goToStep('register_save');
        //! EXPORT_FN_IGNORE_END
    },

    async auth_getIds() {
        //! EXPORT_FN_IGNORE_START
        const channel = this.channel!;
        const actor = this.actor!;
        const operation = this.operation! as BerytusAccountAuthenticationOperation;
        //! EXPORT_FN_IGNORE_END
    },

    async createChannel() {
        // add something like emit start loading event.
        /*! Domain-based credential mapping actor */
        const actor = new BerytusAnonymousWebAppActor();
        //!
        const channel = await BerytusChannel.create({
            webApp: actor
        });
        //!
        //! EXPORT_FN_IGNORE_START
        this.channel = channel;
        this.actor = actor;
        await this._goToStep('login');
        //! EXPORT_FN_IGNORE_END
    },

    async login() {
        //! EXPORT_FN_IGNORE_START
        const channel = this.channel!;
        const actor = this.actor!;
        //! EXPORT_FN_IGNORE_END
        const operation = await channel.login({
            requiredUserAttributes: {
                firstName: true,
                lastName: true
            }
        });
        //! EXPORT_FN_IGNORE_START
        this.operation = operation;
        //! EXPORT_FN_IGNORE_END
        if (operation.intent === 'Register') {
            /*! Handle registration operation */
            //! EXPORT_FN_IGNORE_START
            await this._goToStep('register_addFields');
            //! EXPORT_FN_IGNORE_END
        } else {
            /*! Handle authentication operation */
            //! EXPORT_FN_IGNORE_START
            await this._goToStep('auth_getIds');
            //! EXPORT_FN_IGNORE_END
        }
    }
};

Alpine.store('sampleLogin', store);