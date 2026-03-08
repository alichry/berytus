import { Channel, EChannelType } from '@root/backend/db/models/Channel.js';
import { ChannelRequest } from '@root/backend/db/models/ChannelRequest.js';
import * as chai  from 'chai';
import chaiAsPromised from 'chai-as-promised';
const { expect } = chai;
chai.use(chaiAsPromised);

describe('Berytus Channel', () => {
    it("Should create and retrieve a non-E2EE channel", async () => {
        const request = await ChannelRequest.create(
            { origin: "https://example.tld" },
            null,
            null
        );
        const channelId = "abc";
        const scmActor = { ed25519Key: "hello" };
        const channel = await Channel.create(channelId, request.id, scmActor);
        expect(channel.id).to.equal(channelId);
        expect(channel.requestId).to.equal(request.id);
        expect(channel.type).to.equal(EChannelType.NonE2EE);
        expect(channel.scmActor).to.deep.equal(scmActor);
        expect(channel.keyAgreementParameters).to.be.null;
        expect(channel.sessionKey).to.be.null;
        const retrievedChannel = await Channel.getChannel(channel.id);
        expect(retrievedChannel).to.deep.equal(channel);
    });

    it("Should create and retrieve an E2EE channel", async () => {
        const webAppActor = { ed25519Key: "abcdef" };
        const webAppX25519 = { private: "abc", public: "def" };
        const unmaskAllowlist = ["https://example.tld/login/*"]
        const request = await ChannelRequest.create(
            webAppActor,
            webAppX25519,
            unmaskAllowlist
        );
        const channelId = "abc";
        const scmActor = { ed25519Key: "hello" };
        const channel = await Channel.create(channelId, request.id, scmActor);
        expect(channel.id).to.equal(channelId);
        expect(channel.requestId).to.equal(request.id);
        expect(channel.type).to.equal(EChannelType.E2EE);
        expect(channel.scmActor).to.deep.equal(scmActor);
        expect(channel.keyAgreementParameters).to.be.null;
        expect(channel.sessionKey).to.be.null;
        const retrievedChannel = await Channel.getChannel(channel.id);
        expect(retrievedChannel).to.deep.equal(channel);
    });
});