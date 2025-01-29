import type { PoolConnection, RowDataPacket } from "mysql2/promise";
import { useConnection } from "../pool";
import { EntityNotFoundError } from "../errors/EntityNotFoundError";
import { z } from "zod";

export enum EChallengeType {
    DigitalSignature = 'DigitalSignature',
    Password = 'Password',
    SecureRemotePassword = 'SecureRemotePassword',
    OffChannelOtp = 'OffChannelOtp'
}

interface PGetChallengeDef extends RowDataPacket {
    ChallengeID: string;
    AccountVersion: number;
    ChallengeType: EChallengeType;
    ChallengeParameters: string;
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
    public readonly challengeParameters: unknown;

    constructor(
        accountVersion: number,
        challengeId: string,
        challengeType: EChallengeType,
        challengeParameters: unknown
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
        const [res] = await conn.query<PGetChallengeDef[]>(
            'SELECT ChallengeID, AccountVersion, ChallengeType, ' +
            '       ChallengeParameters ' +
            'FROM berytus_account_def_auth_challenge ' +
            'WHERE ChallengeID = ? AND AccountVersion = ?',
            [challengeId, accountVersion]
        );
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
            res[0].ChallengeType,
            JSON.parse(res[0].ChallengeParameters)
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
        const [res] = await conn.query<PGetChallengeDef[]>(
            'SELECT ChallengeID, AccountVersion, ChallengeType, ' +
            '       ChallengeParameters ' +
            'FROM berytus_account_def_auth_challenge ' +
            'WHERE AccountVersion = ?',
            [accountVersion]
        );
        if (res.length === 0) {
            throw EntityNotFoundError.default(
                AccountDefAuthChallenge.name,
                "AccountVersion",
                `${accountVersion}`
            );
        }
        return res.map(r => new AccountDefAuthChallenge(
            accountVersion,
            r.ChallengeID,
            r.ChallengeType,
            JSON.parse(r.ChallengeParameters)
        ))
    }
}