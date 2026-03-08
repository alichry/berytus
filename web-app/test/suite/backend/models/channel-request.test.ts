import { ChannelRequest } from '@root/backend/db/models/ChannelRequest.js';
import * as chai  from 'chai';
import chaiAsPromised from 'chai-as-promised';
const { expect } = chai;
chai.use(chaiAsPromised);

describe('Berytus Channel Request', () => {
    it("Should create and retrieve a non-E2EE channel request", async () => {
        const request = await ChannelRequest.create(
            { origin: "https://example.tld" },
            null,
            null
        );
        expect(request.id).to.be.a('bigint');
        expect(request.webAppActor).to.deep.equal({ origin: "https://example.tld" });
        expect(request.webAppX25519).to.be.null;
        expect(request.unmaskAllowlist).to.be.null;

        const retrievedRequest = await ChannelRequest.getRequest(request.id);
        expect(retrievedRequest).to.deep.equal(request);
    });

    it("Should create and retrieve an E2EE channel request", async () => {
        const webAppActor = { ed25519Key: "abcdef" };
        const webAppX25519 = { private: "abc", public: "def" };
        const unmaskAllowlist = ["https://example.tld/login/*"]
        const request = await ChannelRequest.create(
            webAppActor,
            webAppX25519,
            unmaskAllowlist
        );
        expect(request.id).to.be.a('bigint');
        expect(request.webAppActor).to.deep.equal(webAppActor);
        expect(request.webAppX25519).to.deep.equal(webAppX25519);
        expect(request.unmaskAllowlist).to.deep.equal(unmaskAllowlist);

        const retrievedRequest = await ChannelRequest.getRequest(request.id);
        expect(retrievedRequest).to.deep.equal(request);
    });
});