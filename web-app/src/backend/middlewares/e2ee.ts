import type { APIContext } from "astro";
import objectPath from 'object-path';
import JWECompactCipherBox from "../crypto/JWECompactBox";
import { AesGcmKeyLoader } from "../logic/e2ee-orchestration/AesGcmKeyLoader";
import { Channel } from "../db/models/Channel";
import { releaseAssert } from "../utils/assert";
import { InvalidArgError } from "../errors/InvalidArgError";

/*
 * Goal: define an e2ee midleware such that:
 * - it decrypts incoming payloads to match their cleartext
 *   counterpart's datatype. Fortunately, the current
 *   implementation employ a uniform payload format for
 *   both cleartext and ciphertext payloads. Previously,
 *   when e2ee is disabled, the public key in the SelectKey
 *   message is sent by the web app as a base64-encoded string,
 *   but when e2ee is enabled, the web app sends an encrypted
 *   payload that decrypts to the raw bytes of the public key.
 *   Hence, if somehow the implementation is modified to alllow
 *   cleartext payload formats that are different from that of
 *   the plaintext, this middleware must handle it to ensure
 *   uniformity.
 * - it encrypts outgoing payloads, ensuring the ciphertext
 *   corresponds to what the secret manager expects; e.g.,
 *   don't encrypt base64 encoding of a signed nonce, encrypt
 *   the raw bytes.
 *
 * For both of these operations, the middleware should be
 * intuitive enough to infer the channel and retrieve the
 * shared secret (if any). If e2ee is disabled, i.e. no shared
 * key, the middleware leaves the payload intact.
 */

export const handleRequest = async (context: APIContext) => {
    const parsedUrl = new URL(context.request.url);
    const pathPattern = /^\/channel\/(?<channelId>{[a-zA-Z0-9\-_]+})\/login\/[{}a-zA-Z0-9\-_]+\/[{}a-zA-Z0-9\-_]+\/(create|id|exists|auth\/[{}a-zA-Z0-9\-_]+\/challenge\/[{}a-zA-Z0-9\-_]+\/respond-message)$/
    const matchRes = pathPattern.exec(decodeURI(parsedUrl.pathname));
    if (matchRes == null) {
        return;
    }
    const channelId = matchRes.groups!.channelId;
    if (context.request.method !== "POST") {
        return;
    }
    const contentTypeHeader = context.request.headers.get("Content-Type");
    if (! contentTypeHeader) {
        return;
    }
    if (contentTypeHeader.startsWith("application/octet-stream")) {
        // cleartext blob!
        context.locals.requestBody = await context.request.blob();
        return;
    }
    if (contentTypeHeader === "application/jose") {
        context.locals.requestBody = new Blob(
            [await context.request.arrayBuffer()],
            { type: contentTypeHeader }
        );
    }
    if (contentTypeHeader.startsWith("multipart/form-data;")) {
        const formData = await context.request.formData();
        const obj: { $?: RequestBody; } = {};
        for (let [key, value] of formData.entries()) {
            // current Cipherbox implementation expects
            // ciphertypes as strings. Check each blob's content-type,
            // if it's jose, then set value to text()
            if (value instanceof Blob && value.type === "application/jose") {
                value = await value.text();
            }
            objectPath.set(obj, `$.${key}`, value);
        }
        context.locals.requestBody = obj.$;
    }
    if (undefined === context.locals.requestBody) {
        // do nothing, unrecognised mimetype
        return;
    }
    const channelIdHeader = context.request.headers.get("X-Berytus-Channel-Id");
    if (channelIdHeader === null) {
        return;
    }
    // TODO(berytus): FIXME channelId in path parameter
    // is not consistent with X-Berytus-Channel-Id
    // if (channelIdHeader !== channelId) {
    //     throw new InvalidArgError(
    //         "X-Berytus-Channel-Id header must be consistent with the channelId URL parameter. "
    //         + `Path Parameter: ${channelId} -- X-Berytus-Channel-Id: ${channelIdHeader}`
    //     );
    // }
    const keyMaterial = (await Channel.getChannel(channelId)).sessionKey;
    releaseAssert(typeof keyMaterial === 'string');
    const key = await (new AesGcmKeyLoader().importKey(keyMaterial));
    const box = new JWECompactCipherBox({
        key,
        async transformDecrypted(value) {
            if (value.type === "text/plain" || value.type?.startsWith("text/plain;")) {
                return await value.text();
            }
            return value;
        }
    });
    // context.locals.requestBody is either a Blob (ciphertext)
    // or a formdata.
    if (context.locals.requestBody instanceof Blob) {
        const decrypted = await box.decrypt(await context.locals.requestBody.text());
        if (decrypted !== null) {
            context.locals.requestBody = decrypted;
        }
        return;
    }
    releaseAssert(typeof context.locals.requestBody === "object");
    releaseAssert(context.locals.requestBody !== null);
    const decryptedDict = await box.decryptDictionary(context.locals.requestBody);
    context.locals.requestBody = decryptedDict;
}

export const handleResponse = async (context: APIContext, resp: Response) => {
    const parsedUrl = new URL(context.request.url);
    const pathPattern = /^\/channel\/(?<channelId>{[a-zA-Z0-9\-_]+})\/login\/[{}a-zA-Z0-9\-_]+\/[{}a-zA-Z0-9\-_]+\/(constants|auth\/[{}a-zA-Z0-9\-_]+\/challenge\/[{}a-zA-Z0-9\-_]+\/pending-message)$/
    const matchRes = pathPattern.exec(decodeURI(parsedUrl.pathname));
    if (matchRes == null) {
        return resp;
    }
    if (context.request.method !== "POST") {
        return resp;
    }
    const channelId = matchRes.groups!.channelId;
    const contentTypeHeader = resp.headers.get("Content-Type");
    if (! contentTypeHeader) {
        return resp;
    }
    if (resp.status !== 200) {
        return resp;
    }
    const keyMaterial = (await Channel.getChannel(channelId)).sessionKey;
    releaseAssert(typeof keyMaterial === 'string');
    const key = await (new AesGcmKeyLoader().importKey(keyMaterial));
    const box = new JWECompactCipherBox({ key });
    const headers: Record<string, string> = {};
    resp.headers.entries().forEach(([key, value]) => {
        if (key.toLowerCase() === 'content-type') {
            return resp;
        }
        headers[key] = value;
    });
    if (
        contentTypeHeader === "application/octet-stream" ||
        contentTypeHeader.startsWith("text/plain")
    ) {
        // cleartext blob, encrypt and return application/jose
        return new Response(await box.encrypt(await resp.arrayBuffer()), {
            status: 200,
            headers: {
                ...headers,
                'Content-Type': 'application/jose'
            }
        });
    }
    if (contentTypeHeader === 'application/json') {
        const body = await box.encryptDictionary(await resp.json());
        return new Response(JSON.stringify(body), {
            status: 200,
            headers
        });
    }
    return resp;
}