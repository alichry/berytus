import type { AccountStageHandlerConstructor, IAccountStageHandler } from '@root/berytus/types';
import * as AccountHandlers from './account-handlers';

export const getAccountHandlersList = () => {
    const array: Array<AccountStageHandlerConstructor<IAccountStageHandler>> = [];
    Object.values(AccountHandlers).forEach(category =>
        Object.values(category).forEach(handler => {
            array.push(handler);
        })
    );
    array.sort((a, b) => {
        const aV = a.name.split('V').pop()!;
        const bV = b.name.split('V').pop()!;
        if (! /^[0-9]+$/.test(aV) || ! /^[0-9]+$/.test(bV)) {
            return 0;
        }
        if (a.name.match(/E2EE/) && !b.name.match(/E2EE/)) {
            return 1;
        }
        if (!a.name.match(/E2EE/) && b.name.match(/E2EE/)) {
            return -1;
        }
        return Number(aV) - Number(bV);
    })
    return array;
}