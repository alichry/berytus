/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
;
var EOperationStatus;
(function (EOperationStatus) {
    EOperationStatus["Pending"] = "Pending";
    EOperationStatus["Created"] = "Created";
    EOperationStatus["Aborted"] = "Aborted";
    EOperationStatus["Finished"] = "Finished";
})(EOperationStatus || (EOperationStatus = {}));
var EOperationType;
(function (EOperationType) {
    EOperationType["PendingDeclaration"] = "PendingDeclaration";
    EOperationType["Registration"] = "Registration";
    EOperationType["Authentication"] = "Authentication";
})(EOperationType || (EOperationType = {}));
export var ELoginUserIntent;
(function (ELoginUserIntent) {
    ELoginUserIntent["PendingDeclaration"] = "PendingDeclaration";
    ELoginUserIntent["Authenticate"] = "Authenticate";
    ELoginUserIntent["Register"] = "Register";
})(ELoginUserIntent || (ELoginUserIntent = {}));
var EChallengeType;
(function (EChallengeType) {
    EChallengeType["Identification"] = "Identification";
    EChallengeType["DigitalSignature"] = "DigitalSignature";
    EChallengeType["Password"] = "Password";
    EChallengeType["SecureRemotePassword"] = "SecureRemotePassword";
    EChallengeType["ForeignIdentityOtp"] = "ForeignIdentityOtp";
})(EChallengeType || (EChallengeType = {}));
var EChallengeStatus;
(function (EChallengeStatus) {
    EChallengeStatus["Invalid"] = "Invalid";
    EChallengeStatus["Pending"] = "Pending";
    EChallengeStatus["Active"] = "Active";
    EChallengeStatus["Aborted"] = "Aborted";
    EChallengeStatus["Sealed"] = "Sealed";
})(EChallengeStatus || (EChallengeStatus = {}));
;
;
;
var EFieldType;
(function (EFieldType) {
    EFieldType["Identity"] = "Identity";
    EFieldType["ForeignIdentity"] = "ForeignIdentity";
    EFieldType["Password"] = "Password";
    EFieldType["SecurePassword"] = "SecurePassword";
    EFieldType["Key"] = "Key";
})(EFieldType || (EFieldType = {}));
;
var EMetadataProperty;
(function (EMetadataProperty) {
    EMetadataProperty["Version"] = "Version";
    EMetadataProperty["Status"] = "Status";
    EMetadataProperty["Category"] = "Category";
    EMetadataProperty["ChangePassUrl"] = "ChangePassUrl";
})(EMetadataProperty || (EMetadataProperty = {}));
;
var EMetadataStatus;
(function (EMetadataStatus) {
    EMetadataStatus["Pending"] = "Pending";
    EMetadataStatus["Created"] = "Created";
    EMetadataStatus["Retired"] = "Retired";
})(EMetadataStatus || (EMetadataStatus = {}));
;
;
