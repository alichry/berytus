import Dexie, { Table } from 'dexie';
import { SingleChange } from './ts-utils';
import { Identity, PRIMARY_IDENTITY_ID } from './Identity';
import { Picture } from './Picture';
import { Channel } from './Channel';
import { WebApp } from './WebApp';
import type { Request, FieldInfo, AddFieldArgs, FieldValueRejectionReason, RecordMetadata, BerytusSendMessageUnion, PreliminaryRequestContext, BerytusChallengeInfoUnion, OperationMetadata, UserAttributeKey, WebAppActor, ChannelMetadata, LoginOperationMetadata, UriParams } from '@berytus/types';
import { EBerytusChallengeType } from "@berytus/enums";
import { BerytusEncryptedPacket, BerytusUserAttributeKey } from '@berytus/types-extd';

export interface Field extends FieldInfo {
  value: string;
}

export interface FieldValueRejection extends FieldValueRejectionReason {
  fieldId: string;
  webAppDictatedValue?: Field['value'];
  previousValues: Array<Field['value']>;
}

export interface UserAttribute {
  id: BerytusUserAttributeKey;
  mimeType?: string;
  // TODO(berytus): REmove this once we remove ArrayBufferViews.
  value: string | ArrayBuffer | ArrayBufferView | BerytusEncryptedPacket;
}

export interface PutField extends FieldInfo {
  value: string;
}

export interface ChallengeMessage extends Omit<BerytusSendMessageUnion, 'challenge' | 'payload'> {
  payload: string | string[];
}

export type Challenge = BerytusChallengeInfoUnion & {
  messages: Record<string, ChallengeMessage>;
  closed?: boolean;
  abortionReasonCode?: string;
};

export type SrpChallenge = Challenge & {
  type: EBerytusChallengeType.SecureRemotePassword;
  srpState: {
    fields: { username: string; password: string };
    serverPublicKeyHexB?: string;
    clientPrivateKeyHexa?: string;
    clientPublicKeyHexA?: string; /* Not really needed in any computation, but good for debugging */
    salt?: string;
    clientProof?: string;
    serverProofValid?: boolean;
  }
}

export type DsChallenge = Challenge & {
  type: EBerytusChallengeType.DigitalSignature;
  dsState: {
    // PEM
    privateKey: string;
    nonce?: string;
    signedMessage?: string;
  }
}

export type RRequiredUserAttributes = Record<UserAttributeKey, boolean>;
export type RUserAttributes = Record<UserAttributeKey, UserAttribute>;

export interface Session {
  id: string;
  /**
   * This is the ExtensionRequestContext. Currently, it holds the active
   * request and the tabId where the Authentication or Registration session is.
   */
  context: PreliminaryRequestContext;
  /**
   * Only set in the MODE_EXTERNAL_WINDOW mode. If set, this is
   * the tabId of the created window. See session-utils.ts->openWindowOrRedirectTab
   * This is the tab id that corresponds to the Password Manager (us) -'s
   * created window for request processing (approve session, select user
   * attributes, etc). This is not the tab id where the Authentication or
   * Registration session is.
   */
  tabId?: number;
  channel: Omit<ChannelMetadata, 'scmActor'>;
  operation: LoginOperationMetadata;
  //sessionInfo: OperationMetadata;
  requests: Array<Request>;
  requiredUserAttributes?: RRequiredUserAttributes;
  userAttributes?: Partial<RUserAttributes>;
  createFieldOptions?: Array<FieldInfo>;
  putFields?: Array<PutField>;
  fields?: Array<Field>;
  rejectedFieldValues?: Array<FieldValueRejection>;
  metadata: RecordMetadata;
  createdAccountId?: string;
  transitionedFromSessionId?: string;
  /**
   * Auth session attributes.
   * TODO: Separate the registration session attributes
   * from the auth ones
   */
  selectedAccountId?: string;
  challenges?: Record<string, Challenge | SrpChallenge | DsChallenge>;
  /**
   * Numeric version of the record. This is used by React Components that expects
   * a newer version. This is helpful when the background script opens a popup,
   * but without waiting until the record has been updated. The React Component
   * expects a record after a specific 'version' where this attribute indicates.
   */
  version: number;
  saved?: boolean;
  closed?: boolean;
}

export interface Account {
    id: string;
    registrationUri: UriParams;
    webAppActor: WebAppActor;
    userAttributes: Partial<Record<UserAttributeKey, UserAttribute>>;
    fields: Array<Field>;
    date: string;
    metadata: RecordMetadata;
}

export interface Settings {
  seamless: { signup: boolean; login: boolean };
}

export const PRIMARY_SETTINGS_ID = "primary";

export class SecretDexie extends Dexie {
  // 'friends' is added by dexie when declaring the stores()
  // We just tell the typing system this is the case
  accounts!: Table<Account>;
  sessions!: Table<Session>;
  identity!: Table<Identity>;
  picture!: Table<Picture>;
  channel!: Table<Channel>;
  webApp!: Table<WebApp>;
  protected settings!: Table<Settings & { id: string }>;

  constructor() {
    super('secret*');
    this.version(1).stores({
      accounts: 'id',
      sessions: 'id',
      identity: 'id',
      settings: 'id',
      picture: 'filename',
      channel: 'id',
      webAppKey: '++id, ed25519Key',
    });
    this.on('ready', async (dexieDb: Dexie) => {
      const db = dexieDb as SecretDexie;
      if ((await db.settings.count()) === 0) {
        db.settings.add({
          id: PRIMARY_SETTINGS_ID,
          seamless: { signup: false, login: false }
        });
      }
      if ((await db.identity.count()) === 0) {
        db.identity.add({ id: PRIMARY_IDENTITY_ID });
      }
    });
  }

  async getSettings() {
    const settings = await this.settings.get(PRIMARY_SETTINGS_ID);
    if (! settings) {
      throw new Error("What?");
    }
    return settings;
  }

  async updateSettings(change: SingleChange<Settings>) {
    return this.settings.update(PRIMARY_SETTINGS_ID, {
      ...change
    });
  }

  async getIdentity(): Promise<Identity> {
    const identity = await this.identity.get(PRIMARY_IDENTITY_ID);
    if (! identity) {
      throw new Error('Expecting identity to be set!');
    }
    return identity;
  }

  async savePicture(f: File) {
    const data = btoa(String.fromCharCode.apply(null, new Uint8Array(await f.arrayBuffer()) as any));
    const picture = await this.picture.put({
      filename: f.name,
      type: f.type,
      data
    });
    return picture;
  }

  findCorrespondingAccounts(
    givenActor: WebAppActor,
    category?: string,
    version?: number
  ) {
    const matchActors = (actorA: WebAppActor, actorB: WebAppActor): boolean => {
      if ("ed25519Key" in actorA) {
        if (!("ed25519Key" in actorB)) {
          return false;
        }
        return actorA.ed25519Key === actorB.ed25519Key;
      }
      if ("ed25519Key" in actorB) {
        return false;
      }
      return actorB.originalUri.hostname === actorA.originalUri.hostname;
    }
    return this.accounts.filter(
      ({ webAppActor, metadata }) => {
        if (! matchActors(givenActor, webAppActor)) {
          return false;
        }
        if (
          category !== undefined &&
          category !== metadata.category
        ) {
          return false;
        }
        if (
          version !== undefined &&
          version !== metadata.version
        ) {
          return false;
        }
        return true;
      }
      );
  }

  pictureToDataUrl(picture: Picture) {
    return `data:${picture.type};base64,${picture.data}`;
  }
}

export const db = new SecretDexie();

// Just needed to distriibute T when it is a union.
type KeyOf<T> = T extends any ? keyof T : never;
const Algorithms: Record<number, string> = {
  [-42]: "RSAES-OAEP w/ SHA-256"
};

export const fieldOptionValueToString = (optionName: KeyOf<Field['options']>, value: Field['options'][keyof Field['options']]): string => {
  if (value === undefined) {
    return "";
  }
  if (optionName === 'alg') {
    return Algorithms[Number(value)] || `COSE:${value}`;
  }
  return String(value);
}

export const isSrpChallenge = (challenge: Challenge): challenge is SrpChallenge => {
  return "srpState" in challenge;
}
export const isDsChallenge = (challenge: Challenge): challenge is DsChallenge => {
  return "dsState" in challenge;
}

//import('./dummy-data');