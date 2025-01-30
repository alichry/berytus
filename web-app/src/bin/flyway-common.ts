import { databaseHost, databaseName, databaseUser, databasePassword, databasePort } from '../backend/env/db.ts';
import { promisify } from 'node:util';
import { exec as execCb } from 'node:child_process';
const exec = promisify(execCb);

export default async function Exec(print: boolean, ...args: Array<string>) {
    const flywayBin = '/usr/local/bin/flyway';

    const baseArgs = [
        // mysql:
        `-user=${databaseUser}`,
        `-password=${databasePassword}`,
        `-schemas=${databaseName}`,
        `-url=jdbc:mariadb://${databaseHost}:${databasePort}`,
        // sqlite:
        // `-url="jdbc:sqlite:${new URL(join('../..', databaseFile), import.meta.url).pathname}"`,
        `-locations="filesystem:${new URL('../../data/migrations', import.meta.url).pathname}"`,
        `-baselineOnMigrate=true`,
        `-executeInTransaction=true`,
    ];

    try {
        const { stderr, stdout } = await exec(
            `${flywayBin} ${baseArgs.join(' ')} ${args.join(' ')}`
        );
        if (print) {
            console.log(stdout);
            console.error(stderr);
        }

        return {
            success: true,
            code: 0,
            stdout: stdout,
            stderr: stderr
        };
    // @ts-ignore e should be an error.
    } catch (e: Error) {
        /*
         * "If this method is invoked as its util.promisify()ed version,
         * it returns a Promise for an Object with stdout and stderr properties.
         * The returned ChildProcess instance is attached to the Promise as a
         * child property. In case of an error (including any error resulting in
         * an exit code other than 0), a rejected promise is returned, with the
         * same error object given in the callback, but with two additional
         * propertiesstdout and stderr."
         * https://nodejs.org/api/child_process.html#child_processexeccommand-options-callback
         */
        if (print) {
            console.log(e.stdout);
            console.error(e.stderr);
        }
        return {
            success: false,
            code: e.code as number,
            stdout: e.stdout as string,
            stderr: e.stderr as string
        };
    }
}