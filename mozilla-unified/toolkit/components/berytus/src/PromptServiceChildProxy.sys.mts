/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { getChildActor } from "resource://gre/modules/BerytusChildProxyUtils.sys.mjs";
import type { PartialAccountIdentity, GetCredentialsMetadataArgs } from "./types";

export class XPPromptServiceChildProxy {

    async promptUsingPopupNotification(
        innerWindow: IDOMWindow,
        args_webAppCryptoActor: string,
        args_channelConstraints_secretManagerPublicKey: Array<string>,
        args_channelConstraints_enableEndToEndEncryption: boolean,
        args_accountConstraints_category: string,
        args_accountConstraints_schemaVersion: number,
        args_accountConstraints_identity: IPropertyBag,
    ): Promise<string> {
        const currentUri = innerWindow.document.documentURIObject;
        const originalUri = innerWindow.browsingContext.docShell.currentDocumentChannel.originalURI;

        const browsingContext = innerWindow.browsingContext;
        const actor = getChildActor(
            browsingContext,
            'BerytusPromptService'
        );

        //const identity: Record<string, string> = {};
        const identity: Array<PartialAccountIdentity> = [];
        for (const { name, value } of args_accountConstraints_identity.enumerator) {
            identity.push({ fieldId: name, fieldValue: value });
        }
        const args: GetCredentialsMetadataArgs = {
            webAppActor: args_webAppCryptoActor?.length > 0
                ? { ed25519Key: args_webAppCryptoActor }
                : {
                    currentUri: {
                        uri: currentUri.spec,
                        scheme: currentUri.scheme,
                        hostname: currentUri.host,
                        port: currentUri.port,
                        path: currentUri.filePath
                    },
                    originalUri: {
                        uri: originalUri.spec,
                        scheme: originalUri.scheme,
                        hostname: originalUri.host,
                        port: originalUri.port,
                        path: originalUri.filePath
                    }
                },
            channelConstraints: {
                secretManagerPublicKey: args_channelConstraints_secretManagerPublicKey,
                enableEndToEndEncryption: args_channelConstraints_enableEndToEndEncryption
            },
            accountConstraints: {
                category: args_accountConstraints_category?.length > 0
                    ? args_accountConstraints_category : undefined,
                schemaVersion: args_accountConstraints_schemaVersion !== 0
                    ? args_accountConstraints_schemaVersion : undefined,
                identity
            }
        };
        const selectedId = await actor.sendQuery(
            'BerytusPromptService:promptUsingPopupNotification',
            [
                browsingContext,
                args
            ]
        );
        return selectedId;
    }
}

// @ts-ignore
XPPromptServiceChildProxy.prototype.QueryInterface = ChromeUtils.generateQI([
    "mozIBerytusPromptService"
]);