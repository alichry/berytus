export type Keys<T, Prefix extends string | null = null> = {
    [key in string & keyof T]: T[key] extends object
    ? Keys<T[key], key & string>
    : Prefix extends string ? `${Prefix}.${key}` : key;
}[keyof T & string];

export type SingleChange<T, Prefix extends string | null = null> = {
    [key in string & keyof T]: T[key] extends object
    ? SingleChange<T[key], key & string>
    : {
        [C in (Prefix extends string ? `${Prefix}.${key}` : key)]: T[key]
    };
}[keyof T & string];