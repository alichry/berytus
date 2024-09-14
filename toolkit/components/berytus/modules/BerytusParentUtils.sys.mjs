/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
class BaseParent extends JSWindowActorParent {
}
export class PassthroughParent extends BaseParent {
    async receiveMessage(msg) {
        return this.doCall(msg.name, msg.data);
    }
    doCall(messageName, args) {
        const method = this.extractMethod(messageName);
        if (!Array.isArray(args)) {
            throw new Error('Refusing to call Parent\'s instance; '
                + 'passed data is not an array.');
        }
        return this.getInstance()[method](...args);
    }
    isMethodNameValid(method) {
        const instance = this.getInstance();
        return method in instance
            && "function" === typeof instance[method];
    }
    extractMethod(messageName) {
        if (!messageName.startsWith(this.Actor + ":")) {
            throw new Error('Unable to extract method name; message '
                + 'name is malformed.');
        }
        const methodName = messageName.substring(this.Actor.length + 1);
        if (!this.isMethodNameValid(methodName)) {
            throw new Error('Error when extracting method name; message '
                + 'name contains an invalid method name '
                + `${methodName}`);
        }
        return methodName;
    }
}
