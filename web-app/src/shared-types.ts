export type JSONValue = /* based on JSONValue from 'postgres' */
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

export type JSONValueWithBlob = JSONValue
    | Blob
    | readonly JSONValueWithBlob[]
    | {
      readonly [prop: string | number]:
      | undefined
      | JSONValueWithBlob
    };