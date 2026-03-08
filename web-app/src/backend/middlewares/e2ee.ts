import type { APIContext } from "astro";
import objectPath from 'object-path';

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
    if (
        !(/^\/login\/.*\/.*\/auth\/.*\/challenge\/.*\/respond-message$/
            .test(parsedUrl.pathname))
    ) {
        return;
    }
    if (context.request.method !== "POST") {
        return;
    }
    // TODO(berytus): Support blob in addition to multipart
    if (context.request.headers.get("Content-Type") !== "multipart/form-data") {
        return;
    }
    const channelId = context.request.headers.get("X-Berytus-Channel-Id");
    if (channelId === null) {
        return;
    }
    const formData = await context.request.formData();
    const obj = {};
    formData.forEach((value, key) => {
        objectPath.set(obj, key, value);
    });

    context.locals.requestBody = obj;
}

export const handleResponse = async (context: APIContext) => {
    // TODO(berytus): Here
}