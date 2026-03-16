import * as chai  from 'chai';
import chaiAsPromised from 'chai-as-promised';
import { E2EEHandler } from '@root/backend/logic/e2ee-orchestration/E2EEHandler.js';
import type { KeyAgreementParametersJson } from '@root/backend/db/models/Channel';
const { expect } = chai;
chai.use(chaiAsPromised);

describe("Berytus E2EE Handler", () => {
    const testCases = {
        base() {
            const scmKeys = {
                ed25519: {
                    public: "MCowBQYDK2VwAyEAwyMVPSekcbUpbBhbss2cQeTK5GVcmUD7P3plsG6bhW0=",
                    private: "MC4CAQAwBQYDK2VwBCIEIDb01uUD75zQeGW23rzcJ0v7sdPjTaWSBbVnCLd6R6Rq"
                },
                x25519: {
                    public: "MCowBQYDK2VuAyEAKWbLfrKRpN2RZmO8f0Hs7kBq8Ed6vKHU/g7CooYzcyg=",
                    private: "MC4CAQAwBQYDK2VuBCIEICgL2seqNdRmoPjA5Opwnh7x2dRkRWo9Okbf/kSiY7dQ"
                }
            };
            const webAppKeys = {
                ed25519: {
                    public: "MCowBQYDK2VwAyEAHXbhwZ9qvmHes1zVtVaDEtOYf/wkmBnqaI/7wQHW8I8=",
                    private: "MC4CAQAwBQYDK2VwBCIEIM92w28sWFUk6MmIZXjE8MGhcdA3dK7VRseMtmo+3mot"
                },
                x25519: {
                    public: "MCowBQYDK2VuAyEAKPHO3g7D8OHBebSMOpGOZwfUvU366LuIh3KT9GqMRHU=",
                    private: "MC4CAQAwBQYDK2VuBCIEIMjWeHrpHh4jCnS63Ff0gMMgJhLzHFfBlwBia7ybqnlD"
                }
            };
            const kap: KeyAgreementParametersJson = {
                authentication: {
                    name: "Ed25519",
                    public: {
                        scm: scmKeys.ed25519.public,
                        webApp: webAppKeys.ed25519.public,
                    }
                },
                derivation: {
                    hash: "SHA-256",
                    info: "Berytus E2EE",
                    name: "HKDF",
                    salt: "7YVSMdl0"
                },
                exchange: {
                    name: "X25519",
                    public: {
                        scm: scmKeys.x25519.public,
                        webApp: webAppKeys.x25519.public,
                    }
                },
                generation: {
                    length: 256,
                    name: "AES-GCM",
                },
                session: {
                    fingerprint: {
                        hash: "SHA-256",
                        salt: 'zQkowMWd',
                        value: "576376a6c9b0",
                        version: "1"
                    },
                    id: "1234",
                    timestamp: 10000,
                },
            };
            const signatureMessage = `{"authentication":{"name":"Ed25519","public":{"scm":"MCowBQYDK2VwAyEAwyMVPSekcbUpbBhbss2cQeTK5GVcmUD7P3plsG6bhW0=","webApp":"MCowBQYDK2VwAyEAHXbhwZ9qvmHes1zVtVaDEtOYf/wkmBnqaI/7wQHW8I8="}},"derivation":{"hash":"SHA-256","info":"Berytus E2EE","name":"HKDF","salt":"7YVSMdl0"},"exchange":{"name":"X25519","public":{"scm":"MCowBQYDK2VuAyEAKWbLfrKRpN2RZmO8f0Hs7kBq8Ed6vKHU/g7CooYzcyg=","webApp":"MCowBQYDK2VuAyEAKPHO3g7D8OHBebSMOpGOZwfUvU366LuIh3KT9GqMRHU="}},"generation":{"length":256,"name":"AES-GCM"},"session":{"fingerprint":{"hash":"SHA-256","salt":"zQkowMWd","value":"576376a6c9b0","version":"1"},"id":"1234","timestamp":10000}}`;
            const scmSignature = "8PrsdhV+jvA4pvfGj1XFsZoR6WD3s/xXrpfkD7njRPdJbbTiN80tOpOfwmBpN1WEyDXh6Wa9/9clD+I7k0ckCg==";
            const webAppSignature = "P3hvLUmfCaCH7pgVgg8CTUDz64tSFoEoVh8n9RNEd+GpdndZwHlJ+BdXj7heqFRtcr+l3ol3cM7x1MX/vuh2Aw==";
            const sessionKey = "DJm0cWjWbXAeGWLhZenKa28s2j0Oda4foA8+85YYaRQ=";
            return {
                kap,
                webAppKeys,
                scmKeys,
                signatureMessage,
                scmSignature,
                webAppSignature,
                sessionKey
            };
        }
    }

    const mockSigningKeyStore = (spki: string, pkcs8: string) => {
        return {
            getPrivateKeyMaterial: async () => pkcs8,
            getPublicKeyMaterial: async () => spki
        };
    }

    it("Should sign kap, authenticate peer sig, and derive session key", async () => {
        const testCase = testCases.base();
        const handler = new E2EEHandler(
            mockSigningKeyStore(testCase.webAppKeys.ed25519.public, testCase.webAppKeys.ed25519.private),
        );
        const signature = await handler.signKeyAgreementParameters(testCase.kap);
        expect(signature).to.equal(testCase.webAppSignature);
        const verificationResult = await handler.verifyPeerKapSignature(
            testCase.kap,
            testCase.scmSignature
        );
        expect(verificationResult).to.be.true;
        const sessionKey = await handler.deriveSessionKey(
            testCase.kap,
            testCase.webAppKeys.x25519.private
        );
        expect(sessionKey).to.equal(testCase.sessionKey);
    });
});