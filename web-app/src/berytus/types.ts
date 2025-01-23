export enum EStageHandlerType {
    Account = 'Account'
}

export interface IBaseStageHandler {
    type: EStageHandlerType;
    label: string;
    description: string;
    /**
     * steps[0] is the initial step.
     */
    steps: ReadonlyArray<string>;
}

export interface IAccountStageState {
    version: number;
    category: string;
    status: BerytusAccountStatus;
    userAttributes: Record<string, string>;
    identityFields: { id: string; value: string; }[];
    credentialFields: { id: string; value: string; }[];

};

export interface IAccountStageHandler extends IBaseStageHandler {
    type: EStageHandlerType.Account;
    version: number;
    category: string;

    getChannel(): BerytusChannel | undefined;
    getState(): IAccountStageState | undefined;
}

// export interface IChannelStageHandler extends IBaseStageHandler {
//     type: EStageHandlerType.Channel;
//     getChannel(): BerytusChannel | undefined;
// }

export type StepResult<S extends string> = { nextStep: S } | { finished: true };

export type TypedStageHandler<H extends IBaseStageHandler> = H & {
    [k in H['steps'][number]]: () => StepResult<H['steps'][number]> | Promise<StepResult<H['steps'][0]>>;
}

// export interface ChannelStageHandlerConstructor<C extends IChannelStageHandler> {
//     prototype: IChannelStageHandler;
//     new(): C;
// }

export interface AccountStageHandlerConstructor<A extends IAccountStageHandler> {
    prototype: IAccountStageHandler;
    new(): A;
}

// handlerType.handlerLabel.handlerStep
export type StageHandlerStepId = `${EStageHandlerType}.${string}->${string}`
