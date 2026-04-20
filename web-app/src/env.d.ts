/// <reference types="astro/client" />
import type { CipherBlueprint } from "./backend/middlewares/e2ee";
import type { JSONValueWithBlob } from "./shared-types";


declare namespace App {
    interface Locals {
        /**
         * Only set for API routes that *CAN* accept ciphertext
         */
        requestBody?: JSONValueWithBlob;

        cipherBlueprint?: CipherBlueprint;
    }
}