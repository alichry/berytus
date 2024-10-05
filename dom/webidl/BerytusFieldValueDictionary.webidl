/**
 * Base interface for special field values that require
 * a dictioanry format to specify more than one values.
 * This is mainly used in the union type of BerytusField.value.
 * If we had specified 'object' instead of 'BerytusFieldValue'
 * an error will be thrown as the parser would have problem
 * distinguishing between ArrayBuffer and 'object'.
 * Any children of `BerytusFieldValue` should have the attribute
 * type `BerytusDataType` or a subset of its union members.
 */
[SecureContext, Exposed=(Window)]
interface BerytusFieldValueDictionary {};