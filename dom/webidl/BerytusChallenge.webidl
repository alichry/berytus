enum BerytusChallengeType {
  "Identification",
  "DigitalSignature",
  "Password",
  "SecureRemotePassword",
  "ForeignIdentityOtp",
};

[SecureContext, Exposed=(Window)]
interface BerytusChallenge {};