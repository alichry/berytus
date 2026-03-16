import { env, int } from './utils.js';

export const signingKeySpki = env("SIGNING_KEY_SPKI");
export const signingKeyPkcs8 = env("SIGNING_KEY_PKCS8");