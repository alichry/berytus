/// <reference types="astro/client" />

declare namespace App {
    interface Locals {
        /**
         * Only set for API routes that *CAN* accept ciphertext
         */
        requestBody?: import("./shared-types").JSONValueWithBlob;

        cipherBlueprint?: import("./backend/middlewares/e2ee").CipherBlueprint;
    }
}