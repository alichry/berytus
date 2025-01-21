import { AppPageContext } from "@root/pagecontext";

export const indexUrl =
    browser?.runtime?.getURL
        ? browser.runtime.getURL('build/index.html') + "#"
        : 'http://localhost:3000';

export const paths = { loading: `loading` };

export const url = (relativePath: string, appPageContext: AppPageContext) => {
    if (relativePath.startsWith('/')) {
        return `${indexUrl}/${appPageContext}${relativePath}`;
    }
    return `${indexUrl}/${appPageContext}/${relativePath}`;
}