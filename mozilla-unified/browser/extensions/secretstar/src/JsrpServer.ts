import jsrp from 'jsrp';

export default class JsrpServer extends jsrp.server {
    constructor() {
        super();
        this.checkClientProof = this.checkClientProof.bind(this);
        this.getPrivateKey = this.getPrivateKey.bind(this);
        this.getProof = this.getProof.bind(this);
        this.getPublicKey = this.getPublicKey.bind(this);
        this.getSalt = this.getSalt.bind(this);
        this.getSharedKey = this.getSharedKey.bind(this);
        this.setClientPublicKey = this.setClientPublicKey.bind(this);
    }
    public init(options: jsrp.ServerOptions): Promise<this> {
        return new Promise((resolve) => {
            super.init(options, () => {
                resolve(this);
            });
        });
    }
}