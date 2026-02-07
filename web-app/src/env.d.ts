/// <reference types="astro/client" />

declare namespace App {
    interface Locals {
        /**
         * Only set for API routes that can accept ciphertext
         */
        requestBody?: Record<string, unknown>;
    }
}