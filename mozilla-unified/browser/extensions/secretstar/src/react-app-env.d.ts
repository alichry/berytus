/// <reference types="react-scripts" />
import type { IUnderlyingRequestHandler } from "@berytus/types";


declare global {
    var browser: typeof chrome & {
        berytus: {
            //enum: IEnumDictionary,
            registerRequestHandler(requestHandler: IUnderlyingRequestHandler): void,
            unregisterRequestHandler(): void,
            resolveRequest(requestId: string, value: unknown): Promise<void>,
            rejectRequest(requestId: string, value: unknown): Promise<void>,
        }
    };
}