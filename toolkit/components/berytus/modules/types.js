/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
;
var EOperationStatus;
(function (EOperationStatus) {
    EOperationStatus[EOperationStatus["Pending"] = 0] = "Pending";
    EOperationStatus[EOperationStatus["Created"] = 1] = "Created";
    EOperationStatus[EOperationStatus["Aborted"] = 2] = "Aborted";
    EOperationStatus[EOperationStatus["Finished"] = 4] = "Finished";
})(EOperationStatus || (EOperationStatus = {}));
var EOpeationType;
(function (EOpeationType) {
    EOpeationType[EOpeationType["PendingDeclaration"] = 0] = "PendingDeclaration";
    EOpeationType[EOpeationType["Registration"] = 1] = "Registration";
    EOpeationType[EOpeationType["Authentication"] = 2] = "Authentication";
})(EOpeationType || (EOpeationType = {}));
export var ELoginUserIntent;
(function (ELoginUserIntent) {
    ELoginUserIntent[ELoginUserIntent["PendingDeclaration"] = 0] = "PendingDeclaration";
    ELoginUserIntent[ELoginUserIntent["Authenticate"] = 1] = "Authenticate";
    ELoginUserIntent[ELoginUserIntent["Register"] = 2] = "Register";
})(ELoginUserIntent || (ELoginUserIntent = {}));
var EChallengeType;
(function (EChallengeType) {
    EChallengeType[EChallengeType["Identification"] = 0] = "Identification";
    EChallengeType[EChallengeType["DigitalSignature"] = 1] = "DigitalSignature";
    EChallengeType[EChallengeType["Password"] = 2] = "Password";
    EChallengeType[EChallengeType["SecureRemotePassword"] = 3] = "SecureRemotePassword";
    EChallengeType[EChallengeType["ForeignIdentityOtp"] = 4] = "ForeignIdentityOtp";
})(EChallengeType || (EChallengeType = {}));
var EChallengeStatus;
(function (EChallengeStatus) {
    EChallengeStatus[EChallengeStatus["Invalid"] = 0] = "Invalid";
    EChallengeStatus[EChallengeStatus["Pending"] = 1] = "Pending";
    EChallengeStatus[EChallengeStatus["Active"] = 2] = "Active";
    EChallengeStatus[EChallengeStatus["Aborted"] = 3] = "Aborted";
    EChallengeStatus[EChallengeStatus["Sealed"] = 4] = "Sealed";
})(EChallengeStatus || (EChallengeStatus = {}));
;
;
var EFieldType;
(function (EFieldType) {
    EFieldType[EFieldType["Identity"] = 0] = "Identity";
    EFieldType[EFieldType["ForeignIdentity"] = 1] = "ForeignIdentity";
    EFieldType[EFieldType["Password"] = 2] = "Password";
    EFieldType[EFieldType["SecurePassword"] = 3] = "SecurePassword";
    EFieldType[EFieldType["Key"] = 4] = "Key";
})(EFieldType || (EFieldType = {}));
;
var EMetadataProperty;
(function (EMetadataProperty) {
    EMetadataProperty[EMetadataProperty["Version"] = 0] = "Version";
    EMetadataProperty[EMetadataProperty["Status"] = 1] = "Status";
    EMetadataProperty[EMetadataProperty["Category"] = 2] = "Category";
    EMetadataProperty[EMetadataProperty["ChangePassUrl"] = 4] = "ChangePassUrl";
})(EMetadataProperty || (EMetadataProperty = {}));
;
var EMetadataStatus;
(function (EMetadataStatus) {
    EMetadataStatus[EMetadataStatus["Pending"] = 0] = "Pending";
    EMetadataStatus[EMetadataStatus["Created"] = 1] = "Created";
    EMetadataStatus[EMetadataStatus["Retired"] = 2] = "Retired";
})(EMetadataStatus || (EMetadataStatus = {}));
;
