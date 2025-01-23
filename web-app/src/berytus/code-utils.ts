import * as prettier from 'prettier';
import prettierPluginBabel from 'prettier/plugins/babel';
import prettierPluginEstree from 'prettier/plugins/estree';
import type { IBaseStageHandler, StageHandlerStepId } from './types';

function unindentOneLevel(code: string) {
    let whitespaces = "";
    for (let i = 0; i < code.length - 1; i++) {
        // get number of tabs or spaces till we reach the
        // first non-whitespace char.
        if (code.charAt(i).match(/[\s]/) === null) {
            break;
        }
        whitespaces += code.charAt(i);
    }
    const regexp = new window.RegExp(`^${whitespaces}`, 'gm');
    code = code.replaceAll(regexp, '');
    return code;
}

export function extractFunctionBody(fn: Function) {
    let code = fn.toString();
    for (let i = 0; i < code.length - 1; i++) {
        if (code.charAt(i) !== '\n') {
            continue;
        }
        code = code.slice(i + 1);
        break;
    }
    for (let i = code.length - 1; i >= 0; i--) {
        if (code.charAt(i) !== '\n') {
            continue;
        }
        code = code.slice(0, i);
        break;
    }

    return code;
}

export function extractFunctionCode(fn: Function) {
    let code = extractFunctionBody(fn);
    code = unindentOneLevel(code);
    code = code.trimEnd();
    return code;
}

export async function exportSampleCode(fn: Function) {
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

export type StageHandlerStepCode = {
    handler: Omit<IBaseStageHandler, "steps">;
    step: string;
    code: string;
};

type HandlerFn = { prototype: IBaseStageHandler };

const exportHandlerStepCode = async (
    top: HandlerFn,
    step: string,
    curr?: { prototype: object }
): Promise<StageHandlerStepCode> => {
    if (! curr) {
        curr = top;
    }
    const func = curr.prototype[step as keyof typeof curr.prototype];
    if (typeof func === "function") {
        const code = await exportSampleCode(func);
        return {
            handler: {
                label: top.prototype.label,
                type: top.prototype.type,
                description: top.prototype.description
            },
            step,
            code
        };
    }
    if ("prototype" in curr.prototype) {
        return exportHandlerStepCode(top, step, curr.prototype as { prototype: object});
    }
    throw new Error(
        `Could not export handler step ${step}. ` +
        `Handler did not define its method.`
    );
}

export const exportHandlerCode = async (
    handlers: Array<{ prototype: IBaseStageHandler }>
): Promise<Record<StageHandlerStepId, StageHandlerStepCode>> => {
    const promises: Array<Promise<void>> = [];
    const result: Record<StageHandlerStepId, StageHandlerStepCode> = {};
    handlers.forEach((handler) => {
        handler.prototype.steps.forEach(step => {
            const promise = exportHandlerStepCode(handler, step)
                .then(stepCode => {
                    result[`${handler.prototype.type}.${handler.prototype.label}->${step}`] = stepCode;
                });
            promises.push(promise);
        });

        // const { prototype }  = handler;
        // prototype.steps.forEach(step => {
        //     const func = prototype[step as keyof typeof prototype];
        //     if (typeof func !== "function") {
        //         throw new Error(
        //             `Bad Stage Handler ${prototype.type}.${prototype.label}: Invalid step ${step}`
        //         );
        //     }
        //     const promise = exportSampleCode(func)
        //         .then((code) => {
        //             result[`${prototype.type}.${prototype.label}->${step}`] = {
        //                 handler: {
        //                     label: prototype.label,
        //                     type: prototype.type,
        //                     description: prototype.description
        //                 },
        //                 step,
        //                 code
        //             };
        //         });
        //         promises.push(promise);
        // });
    });
    return Promise.all(promises).then(() => result);
}