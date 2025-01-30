/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

export class BerytusAgentTargetChild extends JSWindowActorChild {
    /**
     * Copied from ASRouterChild.sys.mjs
     */
    wrapPromise(promise: Promise<unknown>) {
        return new this.contentWindow.Promise((resolve, reject) =>
          promise.then(resolve, reject)
        );
    }

    didDestroy() {
        console.debug(`BerytusAgentTargetChild::didDestroy()`);
    }

    sendQuery(aName: string, aData: unknown) {
        const { contentWindow } = this;
        return this.wrapPromise(
            new Promise((resolve, reject) => {
                super.sendQuery(aName, aData).then(
                    result => {
                        resolve(Cu.cloneInto(result, contentWindow));
                    },
                    err => {
                        let clonedErr;
                        try {
                            clonedErr = Cu.cloneInto(err, contentWindow);
                        } catch (cloneIntoErr) {
                            console.warn("BerytusAgentTargetChild::sendQuery()->reject(): Unable to clone rejection value; see error cloning error below.");
                            console.error(cloneIntoErr);
                            console.warn("BerytusAgentTargetChild::sendQuery()->reject(): Rejection value:");
                            console.error(err);
                            reject({
                                result: Cr.NS_BINDING_ABORTED,
                                message: "Exception occurred during request processing; unable to clone rejection value."
                            });
                            return;
                        }
                        reject(clonedErr);
                    }
                );
            })
        );
    }
}