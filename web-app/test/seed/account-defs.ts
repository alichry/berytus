import { withSearchPath } from "@test/with-search-path.js";

const stmts = [
    `INSERT INTO berytus_account_def
    (AccountVersion) VALUES (1),
                            (2),
                            (3),
                            (4),
                            (1000),
                            (1001),
                            (2000),
                            (3000)`,

    `INSERT INTO berytus_account_def_category
    (AccountCategory) VALUES ('Customer'),
                            ('Employee'),
                            ('Citizen')`,

    `INSERT INTO berytus_account_def_category_version
    (AccountCategory, AccountVersion, AllowNewRegistrations, Description)
    VALUES
    ('Customer', 1, TRUE, 'Username Identification and Password Authentication'),
    ('Customer', 2, TRUE, 'Username Identification and Secure Remote Password Authentication'),
    ('Customer', 3, TRUE, 'Username Identification and Digital Signature Authentication'),
    ('Customer', 4, TRUE, 'Username Identification, Secure Remote Password Authentication and Digital Signature Authentication'),
    ('Customer', 1000, TRUE, 'Email Identification, Secure Remote Password Authentication and Email OTP Authentication'),
    ('Customer', 1001, TRUE, '(Passwordless/Keyless Authentication) Email Identification and Email OTP Authentication'),
    ('Employee', 2000, TRUE, '(Composite Login Handle) PartyId+Username Identification and Secure Remote Password Authentication'),
    ('Citizen', 3000, TRUE, '(Composite and Natural Login Handle) FirstName+FatherName+Surname+DOB+CountryOfBirth+NationalID Identification and Secure Remote Password Authentication')`,

    `INSERT INTO berytus_account_def_field
    (AccountVersion, FieldID, FieldType, FieldOptions) VALUES
    (1, 'username', 'Identity', '{ "private": false, "humanReadable": true, "maxLength": 24 }'),
    (1, 'password', 'Password',  '{ "passwordRules": "minlength: 16;" }'),

    (2, 'username', 'Identity', '{ "private": false, "humanReadable": true, "maxLength": 24 }'),
    (2, 'securePassword', 'SecurePassword', '{ "identityFieldId": "username" }'),

    (3, 'username', 'Identity', '{ "private": false, "humanReadable": true, "maxLength": 24 }'),
    (3, 'key', 'Key', NULL),

    (4, 'username', 'Identity', '{ "private": false, "humanReadable": true, "maxLength": 24 }'),
    (4, 'securePassword', 'SecurePassword',  '{ "identityFieldId": "username" }'),
    (4, 'key', 'Key', NULL),

    (1000, 'email', 'ForeignIdentity', '{ "private": false, "kind": "EmailAddress" }'),
    (1000, 'securePassword', 'SecurePassword',  '{ "identityFieldId": "email" }'),

    (1001, 'email', 'ForeignIdentity', '{ "private": false, "kind": "EmailAddress" }'),

    (2000, 'partyId', 'Identity', '{ "private": false,  "humanReadable": false, "maxLength": 24 }'),
    (2000, 'username', 'Identity', '{ "private": false,  "humanReadable": true, "maxLength": 24 }'),
    (2000, 'securePassword', 'SecurePassword',  '{ "identityFieldId": "partyId" }'),

    (3000, 'firstName', 'Identity', '{ "private": true,  "humanReadable": true, "maxLength": 256 }'),
    (3000, 'fatherName', 'Identity', '{ "private": true,  "humanReadable": true, "maxLength": 256 }'),
    (3000, 'lastName', 'Identity', '{ "private": true,  "humanReadable": true, "maxLength": 256 }'),
    (3000, 'dob', 'Identity', '{ "private": true,  "humanReadable": true, "maxLength": 256 }'),
    (3000, 'countryOfBirth', 'Identity', '{ "private": true,  "humanReadable": true, "maxLength": 256 }'),
    (3000, 'nationalId', 'Identity', '{ "private": true,  "humanReadable": true, "maxLength": 256 }')`,

    `INSERT INTO berytus_account_def_key_field_id
    (AccountVersion, FieldID) VALUES
    (1, 'username'),

    (2, 'username'),

    (3, 'username'),

    (4, 'username'),

    (1000, 'email'),

    (1001, 'email'),

    (2000, 'partyId'),
    (2000, 'username'),

    (3000, 'firstName'),
    (3000, 'fatherName'),
    (3000, 'lastName'),
    (3000, 'dob'),
    (3000, 'countryOfBirth'),
    (3000, 'nationalId')`,
]

export const createAccountDefs = async () => {
    return withSearchPath(async conn => {
        for (let i = 0; i < stmts.length; i++) {
            await conn.unsafe(stmts[i]);
        }
    });
}
