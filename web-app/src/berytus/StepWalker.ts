import Alpine from "alpinejs";
import { exportHandlerCode, type StageHandlerStepCode } from "./code-utils";
import { EStageHandlerType, type IAccountStageHandler, type IAccountStageState, type IBaseStageHandler, type StageHandlerStepId, type StepResult } from "./types";
import { createAccountHandler } from './accounts/handlers/factory';
import { getAccountHandlersList } from './accounts/handlers/export-utils';
import { Selector, type ISelector } from "./Selector";

interface IStepWalker extends ISelector<Omit<IBaseStageHandler, "steps">> {
    error: undefined | Error;
    loading: boolean;
    started: boolean;
    finished: boolean;
    restarting: boolean;

    /**
     * Current stage.
     */
    stage: EStageHandlerType;
    /**
     * Current step name
     */
    step: string;
    /**
     * Current code for the current step
     */
    code: string;
    /**
     * Current state.
     */
    state: IAccountStageState | undefined;

    walk(): Promise<void>;

    restart(): Promise<void>;
};

async function getStepCodes() {
    let result: Record<StageHandlerStepId, StageHandlerStepCode> = {};
        await Promise.all([
            exportHandlerCode(getAccountHandlersList())
        ]).then(list => {
            list.forEach(steps => {
                result = {
                    ...result,
                    ...steps
                };
            });
        });
        return result;
}

type Handler = object & (IAccountStageHandler);

type HandlerInfo = Omit<IBaseStageHandler, "steps">;

export class StepWalker extends Selector<HandlerInfo> implements IStepWalker {
    error: Error | undefined = undefined;
    finished = false;
    started = false;
    loading = false;
    restarting = false;
    steps: Record<StageHandlerStepId, StageHandlerStepCode> = {};

    stage: EStageHandlerType = EStageHandlerType.Account;

    state: IAccountStageState | undefined = undefined;

    code!: string;
    step!: string;

    _handler!: Handler;
    _handlers: Array<IAccountStageHandler> = [];
    _canChangeHandler: boolean = true;

    protected constructor(steps: Record<StageHandlerStepId, StageHandlerStepCode>) {
        super();
        this.steps = steps;
        this._createHandlers();
        this._setHandler(this._handlers[0]);
        this.setItems(this._handlers);
    }

    static async create(): Promise<StepWalker> {
        return new StepWalker(await getStepCodes());
    }

    async restart() {
        this.restarting = true;
        this.error = undefined;
        this.finished = false;
        this.loading = false;
        this.stage = EStageHandlerType.Account;
        {
            if (this._handler?.getChannel()?.active) {
                await this._handler.getChannel()?.close();
            }
            this._createHandlers();
            this.setItems(this._handlers);
            this._setHandler(this._handlers[0]);
        }
        this.restarting = false;
    }

    select(index: number) {
        super.select(index);
        const selectedHandler = this._handlers.find(h => h.label === this.selected!.label);
        if (! selectedHandler) {
            throw new Error('Cannot find the selected handler.');
        }
        this._setHandler(selectedHandler);
    }

    _createHandlers() {
        switch (this.stage) {
            case EStageHandlerType.Account:
                this._handlers = getAccountHandlersList().map(ctor =>
                    createAccountHandler(ctor)
                );
        }
    }

    _stepExists(stepName: string) {
        return this._handler.steps.indexOf(stepName) !== -1;
    }

    _setNextStep(stepName: string) {
        if (this.loading) {
            throw new Error('A step is being executed. Cannot change next step yet.');
        }
        if (! this._stepExists(stepName)) {
            throw new Error('Invalid step name ' + stepName);
        }
        this.step = stepName;
        this._updateCode();
    }

    _updateCode() {
        const step = this.steps[`${this._handler.type}.${this._handler.label}->${this.step}`];
        if (! step) {
            this.code = "";
            return;
        }
        this.code = step.code;
    }

    _validateStepResult(result: unknown): asserts result is StepResult<string> {
        if (
            typeof result !== "object" ||
            result === null ||
            Object.keys(result).length > 1 || !(
                ("nextStep" in result && typeof result.nextStep === "string") ||
                ("finished" in result && result.finished === true)
            )
        ) {
            throw new Error('Malformed step result format: ' + JSON.stringify(result));
        }
    }

    async walk() {
        try {
            if (this.loading) {
                throw new Error('Cannot walk two steps at a time. Please wait for the previous walk() to finish.');
            }
            if (this.finished) {
                throw new Error('Cannot walk anymore as we have finished');
            }
            this.started = true;
            this.loading = true;
            this.pruneItems();
            const currentFn = this._getFn();
            const promise = currentFn();
            const result = await promise;
            this._validateStepResult(result);
            this.state = this._handler.getState();
            this.loading = false;
            if ("finished" in result) {
                this._finish();
            } else {
                this._setNextStep(result.nextStep);
            }
        } catch (e) {
            this.error = e as Error;
            this.loading = false;
            console.error('An error has occurred:', e);
        }
    }

    _getFn(): Function {
        const fn = (<Record<string, unknown>>this._handler)[this.step]
        if (typeof fn !== "function") {
            throw new Error(
                `${this._handler.label} handler does not implement ${this.step}.`
            );
        }
        return fn.bind(this._handler);
    }

    _finish(): void {
        /*
        if (.. weAreChangingStage ...) {
            this.stage = .. next stage ..;
            this._createHandlers();
            this._setHandler(this._handlers[0]);
            this.setItems(this._handlers);
            return;
        }
        */
        this.finished = true;
        return;
    }

    _setHandler(handler: Handler) {
        this._handler = handler;
        this._setNextStep(this._handler.steps[0]);
    }
}

class StepWalkerStore extends StepWalker {
    ready = false;
    _readyPromise: Promise<void>;

    constructor() {
        super({});
        this._readyPromise = getStepCodes()
            .then((steps) => {
                this.steps = steps;
                this._updateCode();
                this.ready = true;
            });
    }

    async init() {
        await this._readyPromise;
    }

    async keepWalking() {
        while (! this.finished && ! this.error) {
            await this.walk();
        }
    }
}

Alpine.store('stepWalker', new StepWalkerStore());
