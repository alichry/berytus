/*
 * Versions 1-4: Customer (Username)
 * Versions 1000-1001: Customer (Email)
 * Version 2000: Employee (PartyId, Username)
 * Version 3000: Citizen (First name, father's name, Surname, DOB, Country, SecurePassword)
 *
 * Version 1: Username, Password
 * Version 2: Username, SecurePassword
 * Version 3: Username, Key
 * Version 4: Username, SecurePassword, Key. E.g., password + hardware key, albeit Berytus does not specify any use of hardware keys at the moment but hardware keys can still be used in password managers. Perhaps a new field type, HardwareKey, might be worthwile.
 * Version 1000, Email, SecurePassword, EmailOTP
 * Version 1001: Email, EmailOTP,
 * Version 2000: PartyId, Username, SecurePassword
 * Version 3000: First name, Father's name, Surname, DOB, Country of birth, NationalID, SecurePassword
 */

INSERT INTO berytus_account_def
    (AccountVersion) VALUES (1),
                            (2),
                            (3),
                            (4),
                            (1000),
                            (1001),
                            (2000),
                            (3000)
    ;

INSERT INTO berytus_account_def_category
    (AccountCategory) VALUES ("Customer"),
                            ("Employee"),
                            ("Citizen")
    ;

INSERT INTO berytus_account_def_category_version
    (AccountCategory, AccountVersion, AllowNewRegistrations, Description)
    VALUES
    ("Customer", 1, 1, "Username Identification and Password Authentication"),
    ("Customer", 2, 1, "Username Identification and Secure Remote Password Authentication"),
    ("Customer", 3, 1, "Username Identification and Digital Signature Authentication"),
    ("Customer", 4, 1, "Username Identification, Secure Remote Password Authentication and Digital Signature Authentication"),
    ("Customer", 1000, 1, "Email Identification, Secure Remote Password Authentication and Email OTP Authentication"),
    ("Customer", 1001, 1, "(Passwordless/Keyless Authentication) Email Identification and Email OTP Authentication"),
    ("Employee", 2000, 1, "(Composite Login Handle) PartyId+Username Identification and Secure Remote Password Authentication"),
    ("Citizen", 3000, 1, "(Composite and Natural Login Handle) FirstName+FatherName+Surname+DOB+CountryOfBirth+NationalID Identification and Secure Remote Password Authentication")
    ;

INSERT INTO berytus_account_def_field
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
    (3000, 'nationalId', 'Identity', '{ "private": true,  "humanReadable": true, "maxLength": 256 }')
    ;

INSERT INTO berytus_account_def_key_field_id
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
    (3000, 'nationalId')
    ;

INSERT INTO berytus_account_def_auth_challenge
(AccountVersion, ChallengeID, ChallengeType, ChallengeParameters)
VALUES
(1, "password", "Password", '{ "passwordFieldIds": ["password"] }');

/*
INSERT INTO berytus_account_field
(AccountID, AccountVersion, FieldID, FieldValue)
 VALUES
(1, 1, "username", '"bob123"'),
(1, 1, "password", '"passbob"'),

(1, 2, "username", '"bob123"'),
(1, 2, "securePassword", '"securePassBob"'),

(2, 1, "username", '"john123"'),
(2, 1, "password", '"passjohn"'),

(2, 2, "username", '"john123"'),
(2, 2, "securePassword", '"securePassJohn"'),

(21, 2000, "partyId", '"jerry-and-sons"'),
(21, 2000, "username", '"jerry"'),
(21, 2000, "securePassword", '"securePassJerry"'),

(22, 2000, "partyId", '"jerry-and-sons"'),
(22, 2000, "username", '"caleb"'),
(22, 2000, "securePassword", '"securePassCaleb"'),

(23, 2000, "partyId", '"marks-and-sons"'),
(23, 2000, "username", '"mark"'),
(23, 2000, "securePassword", '"securePassMark"'),

(24, 2000, "partyId", '"mark-and-sons"'),
(24, 2000, "username", '"matty"'),
(24, 2000, "securePassword", '"securePassMatty"');
*/