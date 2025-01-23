import type {  AccountStageHandlerConstructor, IAccountStageHandler } from "@root/berytus/types";

export function createAccountHandler<A extends IAccountStageHandler>(
    ctor: AccountStageHandlerConstructor<A>
) {
    return new ctor();
}