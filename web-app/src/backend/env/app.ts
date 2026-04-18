import { env } from "./utils.js";

export const nodeEnv: string = env('NODE_ENV', 'development');

export const isDev = () => {
    return nodeEnv === 'development';
}

export const logDebugAssertions: boolean = env('LOG_DEBUG_ASSERTIONS', '0') === '1' ? true : false