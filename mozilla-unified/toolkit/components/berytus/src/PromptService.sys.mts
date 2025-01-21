/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { ESecretManagerType } from "resource://gre/modules/BerytusLiaison.sys.mjs";
import type { GetCredentialsMetadataArgs } from "./types";
import { XPCOMUtils } from "resource://gre/modules/XPCOMUtils.sys.mjs";
import type { CollectCredentialsMetadataEntry } from "./Agent.sys.mjs";
import { Agent } from "resource://gre/modules/BerytusAgent.sys.mjs";

const lazy = {};
XPCOMUtils.defineLazyPreferenceGetter(
    lazy,
    "TEST_AUTO_SELECT_BUILTIN",
    "dom.security.berytus.test_auto_select_builtin",
    false
);

export interface ManangerSelectionEntry extends CollectCredentialsMetadataEntry  {
    manager: CollectCredentialsMetadataEntry['manager'] & {
        // TODO(berytus): Handle ext icons
        icon?: string;
    }
}

export class PromptService {

    async promptUsingPopupNotification(
        browsingContext: BrowsingContext,
        args: GetCredentialsMetadataArgs
    ): Promise<string> {
        const managerEntries = await Agent.collectCredentialsMetadata(
            browsingContext.currentWindowContext.innerWindowId,
            {
                uri: browsingContext.currentURI.spec,
                port: browsingContext.currentURI.port,
                scheme: browsingContext.currentURI.scheme,
                hostname: browsingContext.currentURI.host,
                path: browsingContext.currentURI.filePath
            },
            args
        );
        const selectedExtensionId = await Prompter.promptUsingPopupNotification(
            browsingContext,
            managerEntries
        );
        return selectedExtensionId;
    }
}

class Prompter {

    static async promptUsingPopupNotification(
        browsingContext: BrowsingContext,
        managerEntries: Array<ManangerSelectionEntry>
    ): Promise<string> {
        //const browser = (browsingContext as any).topChromeWindow.gBrowser
        const browser = (browsingContext.top as any).embedderElement;
        if (!browser) {
            throw new Error("Null browser provided");
        }
        if (managerEntries.length === 0) {
            throw new Error(
                "No secret managers were found! " +
                "Cannot prompt for secret manager selection."
            );
        }
        // @ts-ignore: TS does not captures assertion here
        if (lazy.TEST_AUTO_SELECT_BUILTIN) {
            const builtIn = managerEntries.find(m => m.manager.type === ESecretManagerType.Native);
            if (! builtIn) {
                throw new Error('Built-in manager was not found!');
            }
            await builtIn.credentialsMetadata;
            return builtIn.manager.id;
        }
        return new Promise((resolve, reject) => {
            const { ownerDocument } = browser as { ownerDocument: Document };

            const listBox = ownerDocument.getElementById(
                'berytus-secret-manager-selector-container'
            )!;
            while (listBox.firstChild) {
                listBox.removeChild(listBox.firstChild);
            }
            const itemTemplate = ownerDocument.getElementById(
                "template-berytus-secret-manager-list-item"
            ) as HTMLTemplateElement;

            let atLeastOneManagerCanBeSelected = false;
            const updateSelectButton = () => {
                const primaryButton = ownerDocument.getElementById('berytus-notification')!
                        .getElementsByClassName('popup-notification-primary-button')![0];
                if (! primaryButton) {
                    return; // popup is not shown yet.
                }
                if (atLeastOneManagerCanBeSelected) {
                    primaryButton.removeAttribute("disabled");
                } else {
                    primaryButton.setAttribute("disabled", "true");
                }
                primaryButton.classList.add("shown");
            }
            for (let i = 0; i < managerEntries.length; i++) {
                const { manager, credentialsMetadata } = managerEntries[i];

                //const extInfo = Prompter.#getExtensionInfo(manager.id);
                const newItem = itemTemplate.content.firstElementChild!.cloneNode(true) as HTMLElement;

                // Add the new radio button, including pre-selection and the callback
                const newRadio = newItem.getElementsByClassName(
                    "berytus-secret-manager-list-item-radio"
                )[0] as HTMLInputElement;
                newRadio.value = manager.id;
                newRadio.addEventListener("change", function (event) {
                    for (let item of listBox.children) {
                        item.classList.remove("checked");
                    }
                    if (
                        event.target &&
                        "checked" in event.target
                        && event.target.checked // typescript conundrum
                    ) {
                        newItem.classList.add("checked");
                    }
                });
                if (i === 0) {
                    newItem.classList.add('checked');
                    newRadio.checked = true;
                }
                if (manager.icon) {
                    newItem.getElementsByClassName(
                        "berytus-secret-manager-list-item-icon"
                    )[0].setAttribute('src', manager.icon);
                }
                newItem.getElementsByClassName(
                    "berytus-secret-manager-list-item-label-primary"
                )[0].textContent = manager.label;
                newItem.getElementsByClassName(
                    "berytus-secret-manager-list-item-label-secondary"
                )[0].textContent = manager.id;

                const spinner = newItem.getElementsByClassName(
                    "berytus-secret-manager-list-item-metadata-spinner-icon"
                )[0];
                spinner.removeAttribute('hidden');
                Promise.resolve(credentialsMetadata)
                    .then((metadata) => {
                        atLeastOneManagerCanBeSelected = true;
                        const nbAccountElement = newItem.getElementsByClassName(
                            "berytus-secret-manager-list-item-metadata-nb-accounts"
                        )[0];
                        nbAccountElement.textContent = String(metadata);
                        spinner.setAttribute('hidden', "true");
                        newItem.getElementsByClassName(
                            "berytus-secret-manager-list-item-metadata-nb-accounts-container"
                        )[0].removeAttribute("hidden");
                    })
                    .catch((err) => {
                        const warningIconElement = newItem.getElementsByClassName(
                            "berytus-secret-manager-list-item-metadata-error-icon"
                        )[0];
                        warningIconElement.setAttribute(
                            'alt', 'An error has occurred while retrieving credentials metadata from the secret manager.'
                        );
                        spinner.setAttribute('hidden', "true");
                        warningIconElement.removeAttribute("hidden");
                        newItem.classList.add('metadata-error');
                        newRadio.setAttribute('disabled', "true");
                    })
                    .finally(() => updateSelectButton())
                listBox.append(newItem);
            }

            const options = {
                hideClose: true,
                eventCallback: (topic: string, nextRemovalReason: unknown, isCancel: boolean) => {
                    if (topic === "removed" && isCancel) {
                        reject(new Components.Exception("", Cr.NS_ERROR_ABORT));
                        return;
                    }
                    if (topic === "shown") {
                        updateSelectButton();
                        return;
                    }
                },
            };

            // TODO(berytus): Throw an error if
            // zero managers can be selected due
            // to an error being thrown in getCredMetadata
            const mainAction = {
                label: 'Select',
                accessKey: 'S',
                callback(event: unknown) {
                    const selectedId = (listBox.querySelector(
                        ".berytus-secret-manager-list-item-radio:checked"
                    ) as HTMLInputElement).value;
                    resolve(selectedId);
                },
            };
            const secondaryActions = [
                {
                    label: 'Cancel',
                    accessKey: 'C',
                    callback(event: unknown) {
                        reject(new Components.Exception("", Cr.NS_ERROR_ABORT));
                    },
                },
            ];

            ownerDocument.getElementById(
                "berytus-header-text"
            )!.textContent = browsingContext.currentURI.host +
                " would like to communicate with your secret manager to create a new " +
                "account or login with an existing one. Please select your preferred " +
                "manager to proceed.";
            ownerDocument.getElementById(
                "berytus-header"
            )!.removeAttribute('hidden')

            browser.ownerGlobal.PopupNotifications.show(
                browser,
                "berytus",
                "", //"Berytus",
                "berytus-notification-icon",
                mainAction,
                secondaryActions,
                options
            );
        });
    }
}

// TODO: Make a PromptService Parent + Child
//       Make XPPromptService available in the Child
//              and XPPromptService calls PromptServiceChild
