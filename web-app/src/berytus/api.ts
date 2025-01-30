interface VersionOnboardConfiguration {
    version: number | RegExp | string;
    fields: Array<BerytusField>;
    fieldValidators: Record<string, () => Promise<any>>;
    challengeValidators: Record<BerytusChallengeType, () => any>;
};


interface FastboardParams {
    channel: BerytusChannel;
    configs: Array<VersionOnboardConfiguration>;
    challengeHandler: any; // msg name as the function name. produce the necessary responses for interactive challenges
    migrationHandler: any;
};

//export const fastboard = function BerytusFastboard(params: FastboardParams);