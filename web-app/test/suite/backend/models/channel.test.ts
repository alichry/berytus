import { Channel, EChannelType } from '@root/backend/db/models/Channel.js';
import { ChannelRequest } from '@root/backend/db/models/ChannelRequest.js';
import { pool, table } from '@root/backend/db/pool.js';
import * as chai  from 'chai';
import chaiAsPromised from 'chai-as-promised';
import { strict as assert } from 'node:assert/strict';
const { expect } = chai;
chai.use(chaiAsPromised);

describe('Berytus Channel', () => {
    const getExistingChannels = async () => {
        const rows = await pool`
            SELECT ChannelID, ChannelType, ChannelRequestID,
                   ScmActor, KeyAgreementParameters,
                   KeyAgreementSignatures, SessionKey
            FROM ${table('berytus_channel')}
        `;
        return rows.map(({
            channelid: id,
            channelrequestid: requestId,
            channeltype: type,
            scmactor: scmActor,
            keyagreementparameters: keyAgreementParameters,
            keyagreementsignatures: keyAgreementSignatures,
            sessionkey: sessionKey
        }) => {
            assert(typeof id === 'string');
            assert(typeof requestId === 'bigint');
            assert(typeof type === 'string');
            assert(type === EChannelType.E2EE || type === EChannelType.NonE2EE);
            assert(typeof scmActor === 'object' && scmActor !== null);
            assert(typeof keyAgreementParameters === 'object');
            assert(typeof keyAgreementSignatures === 'object');
            assert(typeof sessionKey === 'object');
            return {
                id,
                requestId,
                type,
                scmActor,
                keyAgreementParameters,
                keyAgreementSignatures,
                sessionKey
            }
        });
    }

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
        expect(await getExistingChannels()).to.deep.include(channel.toJSON());
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
        expect(await getExistingChannels()).to.deep.include(channel.toJSON());
        const retrievedChannel = await Channel.getChannel(channel.id);
        expect(retrievedChannel).to.deep.equal(channel);
    });

    it("Should create and establish an E2EE channel", async () => {
        // 1. create channel
        // 2. set kap
        // 3. set web app sig
        // 4. set scm sig
        // 5. set session key
        const webAppActor = { ed25519Key: "abcdef" };
        const webAppX25519 = { private: "abc", public: "def" };
        const unmaskAllowlist = ["https://example.tld/login/*"]
        const request = await ChannelRequest.create(
            webAppActor,
            webAppX25519,
            unmaskAllowlist
        );
        assert(request.supportsE2EE());
        const channelId = "abc";
        const scmActor = { ed25519Key: "hello" };
        const channel = await Channel.create(channelId, request.id, scmActor);
        expect(channel.scmActor).to.deep.equal(scmActor);
        const scmX25519 = "QAID"
        const kap = {
            session: {
                id: channel.id,
                fingerprint: {
                    hash: "SHA256",
                    salt: "AQID",
                    value: "AQID",
                    version: "0.1"
                },
                timestamp: 123,
                ...(request.unmaskAllowlist ? {
                    unmaskAllowlist: request.unmaskAllowlist
                } : undefined),
            },
            authentication: {
                name: "Ed25519",
                public: {
                    webApp: request.webAppActor.ed25519Key,
                    scm: scmActor.ed25519Key,
                }
            },
            exchange: {
                name: "X25519",
                public: {
                    webApp: request.webAppX25519.public,
                    scm: scmX25519
                }
            },
            derivation: {
                name: "HKDF",
                hash: "SHA-256",
                info: "Dummy",
                salt: "AQID"
            },
            generation: {
                name: "AES-GCM",
                length: 256
            }
        };
        await channel.setKeyAgreementParameters(kap);
        expect(channel.keyAgreementParameters).to.deep.equal(kap);
        expect(channel.keyAgreementSignatures).to.be.null;
        expect(await getExistingChannels()).to.deep.include(channel.toJSON());
        const webAppSig = "AQAQ";
        await channel.setWebAppKapSignature(webAppSig);
        expect(channel.keyAgreementSignatures).to.be.not.null;
        expect(channel.keyAgreementSignatures!.webApp).to.equal(webAppSig);
        expect(await getExistingChannels()).to.deep.include(channel.toJSON());
        const scmSig = "AQIB";
        await channel.setScmKapSignature(scmSig);
        expect(channel.keyAgreementSignatures).to.be.not.null;
        expect(channel.keyAgreementSignatures!.webApp).to.equal(webAppSig);
        expect(channel.keyAgreementSignatures!.scm).to.equal(scmSig);
        expect(await getExistingChannels()).to.deep.include(channel.toJSON());
        const sessionKey = { dummy: "AQIDBAUG" };
        await channel.setSessionKey(sessionKey);
        expect(channel.sessionKey).to.equal(sessionKey);
        expect(await getExistingChannels()).to.deep.include(channel.toJSON());
    });
});