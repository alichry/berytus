dictionary BerytusChallengeMessageRequestDefinition {
  required DOMString name;
  required any request;
};

dictionary BerytusChallengeMessageResponseDefinition {
  required any response;
};

[SecureContext, Exposed=(Window)]
interface BerytusChallengeMessage {
    readonly attribute DOMString name;
    readonly attribute any request;
    readonly attribute any response;
};