import type { APIContext } from "astro";
import objectPath from 'object-path';
import JWECompactCipherBox from "../crypto/JWECompactBox";
import { AesGcmKeyLoader } from "../logic/e2ee-orchestration/AesGcmKeyLoader";
import { Channel } from "../db/models/Channel";
import { releaseAssert } from "../utils/assert";
import { InvalidArgError } from "../errors/InvalidArgError";
import type { JSONValueWithBlob } from "@root/shared-types";
import { IllegalStateError } from "../errors/IllegalStateError";

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
    if (contentTypeHeader === "text/plain" || contentTypeHeader.startsWith("text/plain;")) {
        // cleatext string
        context.locals.requestBody = await context.request.text();
        return;
    }
    if (contentTypeHeader === "application/json" || contentTypeHeader.startsWith("application/json;")) {
        // cleartext json
        context.locals.requestBody = await context.request.json();
        return;
    }
    if (contentTypeHeader === "application/jose") {
        // ciphertext blob.
        // current Cipherbox implementation expects
        // ciphertypes as strings. Therefore,
        // simply read the JWE into string so that
        // it can be decrypted.
        context.locals.requestBody = await context.request.text();
    }
    if (contentTypeHeader.startsWith("multipart/form-data;")) {
        const formData = await context.request.formData();
        const obj: { $?: JSONValueWithBlob; } = {};
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
    if (channelIdHeader !== channelId) {
        throw new InvalidArgError(
            "X-Berytus-Channel-Id header must be consistent with the channelId URL parameter. "
            + `Path Parameter: ${channelId} -- X-Berytus-Channel-Id: ${channelIdHeader}`
        );
    }
    const keyMaterial = (await Channel.getChannel(channelId)).sessionKey;
    releaseAssert(typeof keyMaterial === 'string', "typeof keyMaterial === 'string'");
    const key = await (new AesGcmKeyLoader().importKey(keyMaterial));
    const transformDecrypted = async (value: Blob) => {
        if (value.type === "text/plain" || value.type?.startsWith("text/plain;")) {
            return await value.text();
        }
        return value;
    }
    const box = new JWECompactCipherBox({
        key,
        transformers: {
            postDecryption: transformDecrypted
        }
    });
    // context.locals.requestBody is either a stringified JWE
    // or a formdata with stringified JWEs.
    if (typeof context.locals.requestBody === "string") {
        const decrypted = await box.decrypt(context.locals.requestBody);
        if (decrypted !== null) {
            context.locals.requestBody = await transformDecrypted(decrypted);
        }
        return;
    }
    releaseAssert(typeof context.locals.requestBody === "object", 'typeof context.locals.requestBody === "object"');
    releaseAssert(context.locals.requestBody !== null, "context.locals.requestBody !== null");
    releaseAssert(!(context.locals.requestBody instanceof Blob), "!(context.locals.requestBody instanceof Blob)");
    const decryptedDict = await box.decryptDictionary(context.locals.requestBody);
    context.locals.requestBody = decryptedDict;
}

export enum ECipherBlueprintType {
    Plain,
    Dictionary,
}

export enum ECipherBlueprintTransformer {
    UTF8Encoder,
    Base64Decoder,
}

export type CipherBlueprintProperty =
    | string
    | { path: string; transformer?: ECipherBlueprintTransformer };

type CipherBlueprintBase = {
    type: ECipherBlueprintType;
    /**
     * Function that determines whether this blueprint should
     * be assumed for the given (cx, response). It
     * must not read the response body. However, if needed,
     * clone the Response first and read its body.
     */
    enabled: (
        cx: APIContext,
        response: Response
    ) => Promise<boolean>;
    transformer?: ECipherBlueprintTransformer;
}

type CipherBlueprintPlain = CipherBlueprintBase & {
    type: ECipherBlueprintType.Plain;
}

type CipherBlueprintDictionary = CipherBlueprintBase & {
    type: ECipherBlueprintType.Dictionary;
    props: readonly CipherBlueprintProperty[] | "*"
}

export type CipherBlueprint = CipherBlueprintPlain | CipherBlueprintDictionary;

function* blueprintProperties(blueprint: CipherBlueprintDictionary) {
    if (blueprint.props === "*") {
        throw new InvalidArgError("blueprintProperties() must not be called with '*'");
    }
    for (const prop of blueprint.props) {
        if (typeof prop === "string") {
            yield { path: prop, transformer: blueprint.transformer };
            continue;
        }
        const { path, transformer } = prop;
        yield { path, transformer: transformer || blueprint.transformer };
    }
}

const transformDatum = async (
    datum: unknown,
    transformer: ECipherBlueprintTransformer
) => {
    switch (transformer) {
        case ECipherBlueprintTransformer.UTF8Encoder: {
            if (typeof datum !== "string") {
                throw new InvalidArgError(
                    "Expected datum to be a string; got otherwise"
                );
            }
            return new Blob(
                [new TextEncoder().encode(datum).buffer],
                { type: "text/plain;charset=UTF-8" }
            );
        }
        case ECipherBlueprintTransformer.Base64Decoder: {
            if (typeof datum !== "string") {
                throw new InvalidArgError(
                    "Expected datum to be a string; got otherwise"
                );
            }
            return new Blob(
                [Uint8Array
                    // @ts-ignore: Node 25+
                    .fromBase64(datum)
                    .buffer],
                { type: "application/octet-stream" }
            );
        }
        default:
            throw new IllegalStateError(
                "Unrecognised transformer type " + transformer
            );
    }
}

const processDictionary = async (
    box: JWECompactCipherBox,
    blueprint: CipherBlueprintDictionary,
    dict: Record<string, unknown>,
): Promise<Record<string, unknown>> => {
    if (blueprint.props === "*") {
        dict = await box.encryptDictionary(dict, {
            async preEncryption(value) {
                if (! blueprint.transformer) {
                    return value;
                }
                return await transformDatum(value, blueprint.transformer);
            },
        });
        return dict;
    }
    for (const { path, transformer } of blueprintProperties(blueprint)) {
        let value = objectPath.get(dict, path);
        const inputDatum = transformer ? await transformDatum(value, transformer) : value;
        const encrypted = await box.encrypt(inputDatum);
        objectPath.set(dict, path, encrypted);
    }
    return dict;
}

const headersToJson = (headers: Headers): Record<string, string> => {
    const result: Record<string, string> = {};
    headers.entries().forEach(([key, value]) => {
        result[key] = value;
    });
    return result;
}

const selectBlueprint = async (
    blueprints: readonly CipherBlueprint[],
    cx: APIContext,
    response: Response
): Promise<CipherBlueprint | undefined> => {
    for (const blueprint of blueprints) {
        if (await blueprint.enabled(cx, response)) {
            return blueprint;
        }
    }
}

const requestUrlMatches = (request: Request, pathPattern: RegExp) => {
    const parsedUrl = new URL(request.url);
    return pathPattern.exec(decodeURI(parsedUrl.pathname));
}

const blueprints: CipherBlueprint[] = [
    {
        type: ECipherBlueprintType.Dictionary,
        async enabled(cx) {
            const match = requestUrlMatches(
                cx.request,
                /^\/channel\/(?<channelId>{[a-zA-Z0-9\-_]+})\/login\/[{}a-zA-Z0-9\-_]+\/(?<accountVersion>[{}a-zA-Z0-9\-_]+)\/(auth\/[{}a-zA-Z0-9\-_]+\/challenge\/(?<challengeId>[{}a-zA-Z0-9\-_]+)\/pending-message)$/
            )
            if (! match) {
                return false;
            }
            return true;
        },
        transformer: ECipherBlueprintTransformer.Base64Decoder,
        props: [
            "nextMessage.request"
        ]
    },
    {
        type: ECipherBlueprintType.Dictionary,
        async enabled(cx) {
            const match = requestUrlMatches(
                cx.request,
                /^\/channel\/(?<channelId>{[a-zA-Z0-9\-_]+})\/login\/[{}a-zA-Z0-9\-_]+\/(?<accountVersion>[{}a-zA-Z0-9\-_]+)\/constants$/
            )
            if (! match) {
                return false;
            }
            return true;
        },
        transformer: ECipherBlueprintTransformer.UTF8Encoder,
        props: "*"
    },
]

export const handleResponse = async (context: APIContext, resp: Response) => {
    if (resp.status !== 200) {
        return resp;
    }
    const { channelId } = context.params;
    const contentTypeHeader = resp.headers.get("Content-Type");
    const blueprint = await selectBlueprint(blueprints, context, resp);
    if (! contentTypeHeader || ! channelId || ! blueprint) {
        return resp;
    }
    const keyMaterial = (await Channel.getChannel(channelId)).sessionKey;
    if (keyMaterial === null) {
        return resp;
    }
    releaseAssert(typeof keyMaterial === 'string', "typeof keyMaterial === 'string'");
    const key = await (new AesGcmKeyLoader().importKey(keyMaterial));
    const box = new JWECompactCipherBox({ key });
    switch (blueprint.type) {
        case ECipherBlueprintType.Plain:
            if (
                contentTypeHeader === "application/octet-stream" ||
                contentTypeHeader === "text/plain" ||
                contentTypeHeader.startsWith("text/plain;")
            ) {
                const inputDatum = blueprint.transformer ? await transformDatum(
                    contentTypeHeader.startsWith("text/plain")
                        ? await resp.text()
                        : await resp.arrayBuffer(),
                    blueprint.transformer
                ) : await resp.blob();
                return new Response(await box.encrypt(inputDatum), {
                    status: 200,
                    headers: {
                        ...headersToJson(resp.headers),
                        'content-type': 'application/jose'
                    }
                });
            }
            throw new InvalidArgError(
                `Unsupported content-type '${contentTypeHeader}'`
            );
        case ECipherBlueprintType.Dictionary: {
            if (contentTypeHeader === 'application/json') {
                let dict = await resp.json();
                dict = await processDictionary(box, blueprint, dict);
                return new Response(JSON.stringify(dict), {
                    status: 200,
                    headers: resp.headers
                });
            }
            throw new InvalidArgError(
                `Unsupported content-type '${contentTypeHeader}'`
            );
        }
        default:
            throw new IllegalStateError("Unrecognised blueprint type");
    }
}


