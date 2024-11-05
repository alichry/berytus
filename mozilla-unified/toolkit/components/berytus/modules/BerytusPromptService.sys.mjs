/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
import { liaison, ESecretManagerType } from "resource://gre/modules/BerytusLiaison.sys.mjs";
import { XPCOMUtils } from "resource://gre/modules/XPCOMUtils.sys.mjs";
const lazy = {};
XPCOMUtils.defineLazyPreferenceGetter(lazy, "TEST_AUTO_SELECT_BUILTIN", "dom.security.berytus.test_auto_select_builtin", false);
const collectCredentialsMetadata = async (innerWindowId, args) => {
    const managers = liaison.managers;
    const entries = [];
    const keys = args.channelConstraints.secretManagerPublicKey;
    const context = { document: { id: innerWindowId } };
    const relevantManagers = keys && keys.length > 0
        ? (await Promise.all(managers.map(async (manager) => {
            const handler = liaison.getRequestHandler(manager.id);
            const key = await handler.manager.getSigningKey(context, { webAppActor: args.webAppActor });
            if (keys.indexOf(key) !== -1) {
                return manager;
            }
            return null;
        }))).filter(m => !!m)
        : managers;
    for (let i = 0; i < relevantManagers.length; i++) {
        const manager = relevantManagers[i];
        const handler = liaison.getRequestHandler(manager.id);
        entries.push({
            manager,
            credentialsMetadata: handler.manager.getCredentialsMetadata(context, args)
        });
    }
    return entries;
};
export class PromptService {
    async promptUsingPopupNotification(browsingContext, args) {
        const managerEntries = await collectCredentialsMetadata(browsingContext.currentWindowContext.innerWindowId, args);
        const selectedExtensionId = await Prompter.promptUsingPopupNotification(browsingContext, managerEntries);
        return selectedExtensionId;
    }
}
class Prompter {
    static async promptUsingPopupNotification(browsingContext, managerEntries) {
        //const browser = (browsingContext as any).topChromeWindow.gBrowser
        const browser = browsingContext.top.embedderElement;
        if (!browser) {
            throw new Error("Null browser provided");
        }
        if (managerEntries.length === 0) {
            throw new Error("No secret managers were found! " +
                "Cannot prompt for secret manager selection.");
        }
        // @ts-ignore: TS does not captures assertion here
        if (lazy.TEST_AUTO_SELECT_BUILTIN) {
            const builtIn = managerEntries.find(m => m.manager.type === ESecretManagerType.Native);
            if (!builtIn) {
                throw new Error('Built-in manager was not found!');
            }
            await builtIn.credentialsMetadata;
            return builtIn.manager.id;
        }
        return new Promise((resolve, reject) => {
            const { ownerDocument } = browser;
            const listBox = ownerDocument.getElementById('berytus-secret-manager-selector-container');
            while (listBox.firstChild) {
                listBox.removeChild(listBox.firstChild);
            }
            const itemTemplate = ownerDocument.getElementById("template-berytus-secret-manager-list-item");
            for (let i = 0; i < managerEntries.length; i++) {
                const { manager, credentialsMetadata } = managerEntries[i];
                //const extInfo = Prompter.#getExtensionInfo(manager.id);
                const newItem = itemTemplate.content.firstElementChild.cloneNode(true);
                // Add the new radio button, including pre-selection and the callback
                const newRadio = newItem.getElementsByClassName("berytus-secret-manager-list-item-radio")[0];
                newRadio.value = manager.id;
                newRadio.addEventListener("change", function (event) {
                    for (let item of listBox.children) {
                        item.classList.remove("checked");
                    }
                    if (event.target &&
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
                    newItem.getElementsByClassName("berytus-secret-manager-list-item-icon")[0].setAttribute('src', manager.icon);
                }
                newItem.getElementsByClassName("berytus-secret-manager-list-item-label-primary")[0].textContent = manager.label;
                newItem.getElementsByClassName("berytus-secret-manager-list-item-label-secondary")[0].textContent = manager.id;
                const spinner = newItem.getElementsByClassName("berytus-secret-manager-list-item-metadata-spinner-icon")[0];
                spinner.setAttribute('hidden', "false");
                Promise.resolve(credentialsMetadata)
                    .then((metadata) => {
                    const nbAccountElement = newItem.getElementsByClassName("berytus-secret-manager-list-item-metadata-nb-accounts")[0];
                    nbAccountElement.textContent = String(metadata);
                    spinner.setAttribute('hidden', "true");
                    newItem.getElementsByClassName("berytus-secret-manager-list-item-metadata-nb-accounts-container")[0].setAttribute("hidden", "false");
                })
                    .catch((err) => {
                    const warningIconElement = newItem.getElementsByClassName("berytus-secret-manager-list-item-metadata-error-icon")[0];
                    warningIconElement.setAttribute('alt', 'An error has occurred while retrieving credentials metadata from the secret manager.');
                    spinner.setAttribute('hidden', "true");
                    warningIconElement.setAttribute("hidden", "false");
                    newItem.classList.add('metadata-error');
                    // At the moment, the item can still be selected despite the error.
                    // not a huge concern for the proof of concept.
                });
                listBox.append(newItem);
            }
            const options = {
                hideClose: true,
                eventCallback: (topic, nextRemovalReason, isCancel) => {
                    if (topic == "removed" && isCancel) {
                        reject(new Components.Exception("", Cr.NS_ERROR_ABORT));
                    }
                },
            };
            const mainAction = {
                label: 'Select',
                accessKey: 'S',
                callback(event) {
                    const selectedId = listBox.querySelector(".berytus-secret-manager-list-item-radio:checked").value;
                    resolve(selectedId);
                },
            };
            const secondaryActions = [
                {
                    label: 'Cancel',
                    accessKey: 'C',
                    callback(event) {
                        reject(new Components.Exception("", Cr.NS_ERROR_ABORT));
                    },
                },
            ];
            ownerDocument.getElementById("berytus-header-text").textContent = browsingContext.currentURI.host +
                " would like to communicate with your secret manager to create a new " +
                "account or login with an existing one. Please select your preferred " +
                "manager to proceed.";
            ownerDocument.getElementById("berytus-header").hidden = false;
            browser.ownerGlobal.PopupNotifications.show(browser, "berytus", "", //"Berytus",
            "berytus-notification-icon", mainAction, secondaryActions, options);
        });
    }
}
// TODO: Make a PromptService Parent + Child
//       Make XPPromptService available in the Child
//              and XPPromptService calls PromptServiceChild
