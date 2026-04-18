/// <reference types="astro/client" />
import { JSONValueWithBlob } from "./shared-types";

declare namespace App {
    interface Locals {
        /**
         * Only set for API routes that *CAN* accept ciphertext
         */
        requestBody?: JSONValueWithBlob;
    }
}