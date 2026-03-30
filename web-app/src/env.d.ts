/// <reference types="astro/client" />

type JSONValue = /* based on JSONValue from 'postgres' */
    | null
    | string
    | number
    | boolean
    | readonly JSONValue[]
    | {
      readonly [prop: string | number]:
      | undefined
      | JSONValue
    };

type RequestBody = JSONValue
    | Blob
    | {
      readonly [prop: string | number]:
      | undefined
      | RequestBody
    };

declare namespace App {
    interface Locals {
        /**
         * Only set for API routes that *CAN* accept ciphertext
         */
        requestBody?: RequestBody;
    }
}