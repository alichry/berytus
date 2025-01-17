dictionary BerytusChallengeMessageRequestDefinition {
  required DOMString name;
  required any request;
};

[GenerateConversionToJS]
dictionary BerytusChallengeMessageResponseDefinition {
  required any response;
};

dictionary BerytusChallengeMessage {
  required DOMString name;
  required any request;
  required any response;
};