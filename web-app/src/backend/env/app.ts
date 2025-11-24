import { env } from "./utils.js";

export const nodeEnv: string = env('NODE_ENV', 'development');

export const isDev = () => {
    return nodeEnv === 'development';
}