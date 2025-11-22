export interface FieldInput {
    id: string;
    value: JSONValue;
}

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