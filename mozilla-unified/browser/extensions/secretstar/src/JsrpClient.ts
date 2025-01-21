import jsrp from 'jsrp';

export default class JsrpClient extends jsrp.client {

    public init(options: jsrp.ClientOptions): Promise<this> {
        return new Promise((resolve) => {
            super.init(options, () => {
                resolve(this);
            });
        });
    }

    public createVerifier(): Promise<jsrp.Verifier> {
        return new Promise((resolve, reject) => {
            super.createVerifier((err, result) => {
                if (err) {
                    reject(err);
                    return;
                }
                resolve(result);
            });
        });
    } 
}