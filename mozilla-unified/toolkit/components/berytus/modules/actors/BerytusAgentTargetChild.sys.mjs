/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
export class BerytusAgentTargetChild extends JSWindowActorChild {
    /**
     * Copied from ASRouterChild.sys.mjs
     */
    wrapPromise(promise) {
        // @ts-ignore: contentWindow is a global window object, I cannot be type it
        return new this.contentWindow.Promise((resolve, reject) => promise.then(resolve, reject));
    }
    sendQuery(aName, aData) {
        return this.wrapPromise(new Promise((resolve, reject) => {
            super.sendQuery(aName, aData).then(result => {
                resolve(Cu.cloneInto(result, this.contentWindow));
            }, err => {
                reject(Cu.cloneInto(err, this.contentWindow));
            });
        }));
    }
}
