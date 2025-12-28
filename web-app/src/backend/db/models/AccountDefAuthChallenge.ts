import { table, useConnection } from "../pool.js";
import type { PoolConnection } from "../pool.js";
import type { JSONValue } from "../types.js";
import { EntityNotFoundError } from "../errors/EntityNotFoundError.js";
import { z } from "zod";

export enum EChallengeType {
    DigitalSignature = 'DigitalSignature',
    Password = 'Password',
    SecureRemotePassword = 'SecureRemotePassword',
    OffChannelOtp = 'OffChannelOtp'
}

interface PGetChallengeDef {
    challengeid: string;
    accountversion: number;
    challengetype: EChallengeType;
    challengeparameters: JSONValue;
}

/* Password Parameters, ideally this should be placed somewhere else */
export const PasswordChallengeParameters = z.object({
    passwordFieldIds: z.array(z.string())
});
export type PasswordChallengeParameters = z.infer<typeof PasswordChallengeParameters>;
/* */
export class AccountDefAuthChallenge {
    public readonly accountVersion: number;
    public readonly challengeId: string;
    public readonly challengeType: EChallengeType;
    public readonly challengeParameters: JSONValue;

    constructor(
        accountVersion: number,
        challengeId: string,
        challengeType: EChallengeType,
        challengeParameters: JSONValue
    ) {
        this.accountVersion = accountVersion;
        this.challengeId = challengeId;
        this.challengeType = challengeType;
        this.challengeParameters = challengeParameters;
    }

    static async challengeDefExists(
        challengeId: string,
        accountVersion: number,
        existingConn?: PoolConnection
    ) {
        try {
            await AccountDefAuthChallenge.getChallengeDef(
                challengeId,
                accountVersion,
                existingConn
            );
            return true;
        } catch (e) {
            if (e instanceof EntityNotFoundError) {
                return false;
            }
            throw e;
        }
    }

    static async getChallengeDef(
        challengeId: string,
        accountVersion: number,
        existingConn?: PoolConnection
    ): Promise<AccountDefAuthChallenge> {
        if (existingConn) {
            return AccountDefAuthChallenge.#getChallengeDef(
                existingConn,
                challengeId,
                accountVersion
            );
        }
        return useConnection((conn) =>
            AccountDefAuthChallenge.#getChallengeDef(
                conn,
                challengeId,
                accountVersion
            )
        );
    }

    static async #getChallengeDef(
        conn: PoolConnection,
        challengeId: string,
        accountVersion: number,
    ) {
        const res = await conn<PGetChallengeDef[]>`
            SELECT ChallengeID, AccountVersion, ChallengeType,
                   ChallengeParameters
            FROM ${table('berytus_account_def_auth_challenge')}
            WHERE ChallengeID = ${challengeId}
            AND AccountVersion = ${accountVersion}
        `;
        if (res.length === 0) {
            throw EntityNotFoundError.default(
                AccountDefAuthChallenge.name,
                "ChallengeID,AccountVersion",
                `${challengeId},${accountVersion}`
            );
        }
        return new AccountDefAuthChallenge(
            accountVersion,
            challengeId,
            res[0].challengetype,
            res[0].challengeparameters
        );
    }

    static async getAllChallengeDef(
        accountVersion: number,
        existingConn?: PoolConnection
    ): Promise<AccountDefAuthChallenge[]> {
        if (existingConn) {
            return AccountDefAuthChallenge.#getAllChallengeDef(
                existingConn,
                accountVersion
            );
        }
        return useConnection((conn) =>
            AccountDefAuthChallenge.#getAllChallengeDef(
                conn,
                accountVersion
            )
        );
    }

    static async #getAllChallengeDef(
        conn: PoolConnection,
        accountVersion: number,
    ) {
        const res = await conn<PGetChallengeDef[]>`
            SELECT ChallengeID, AccountVersion, ChallengeType,
                   ChallengeParameters
            FROM ${table('berytus_account_def_auth_challenge')}
            WHERE AccountVersion = ${accountVersion}
        `;
        if (res.length === 0) {
            throw EntityNotFoundError.default(
                AccountDefAuthChallenge.name,
                "AccountVersion",
                `${accountVersion}`
            );
        }
        return res.map(r => new AccountDefAuthChallenge(
            accountVersion,
            r.challengeid,
            r.challengetype,
            r.challengeparameters
        ))
    }
}