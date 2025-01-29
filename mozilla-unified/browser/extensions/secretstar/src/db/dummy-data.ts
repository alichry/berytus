import { Account, Session, db } from "@root/db/db";
import { randomPassword } from "@root/utils";
import { EBerytusChallengeType, EBerytusFieldType, ELoginUserIntent, EMetadataStatus, EOperationStatus, EOperationType } from "@berytus/enums";
import { EBerytusDigitalSignatureChallengeMessageName, EBerytusIdentificationChallengeMessageName, EBerytusPasswordChallengeMessageName } from "@berytus/types-extd";

export const Accounts: Account[] = [
    {
        id: randomPassword(),
        fields: [
            {
                id: "username",
                type: EBerytusFieldType.Identity,
                options: {
                    humanReadable: true,
                    private: false,
                    allowedCharacters: undefined,
                    maxLength: 16
                },
                value: "bobisstrong"
            },
            {
                id: "email",
                type: EBerytusFieldType.ForeignIdentity,
                options: {
                    kind: "EmailAddress",
                    private: false
                },
                value: "bobby@soprano.com"
            },
            {
                id: "accountId",
                type: EBerytusFieldType.Identity,
                options: {
                    humanReadable: false,
                    private: true,
                    allowedCharacters: undefined,
                    maxLength: 30
                },
                value: "15632-2456"
            },
            {
                id: "password",
                type: EBerytusFieldType.Password,
                options: {},
                value: "6hhy-opau-9ro9-0fqq"
            },
            {
                id: "securePassword",
                type: EBerytusFieldType.SecurePassword,
                options: {
                    identityFieldId: "username"
                },
                value: "xvbh-opau-bbeq-00ax"
            },
            {
                id: "clientKey",
                type: EBerytusFieldType.Key,
                options: {
                    private: true,
                    alg: -44
                },
                value: "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQDniLalhZ4GLVSu/lY6gUh0Kyq+RaUzt2bc/qFtQtZgcfndtNVbWOjRP6i7HXtGeulJI7tH7phF6WaDkvC0+uWFfFzx5+aCd11MjpH2QSIrlO1Ls3IwXWRk9KbrBGLzWPoouVT37D5AZUZ3+NreZhHskWx2aDQk4BbdgI2TMlCE6kuZb4brOZyZ5Pe8CKxHAB9CdwhZaYfyflI7a4Twt3oZ1zs065XlluapCzPaSi8XhHf1tA6iSpssWAFhHZ1JU5JIKF7q5Vg+rK6pIv/vM2Kj7oYjUNhpUF4km+oLFS9bDT4fPKzu075xwhnold8QdW/D60X5s3bGKE3VaksMZZwPAgMED/8CggEADCJ2uYs8DcdSHDPC91Dxef5cwxtAC6AYw0LF7982vsUQhlU/lcVtYVBohNtrY/l4NiGJpuEywXh01wg4R6uCN1QOHZAZWkVMor5C3kmFR0Ljus4fjHPePuE2Hdbf3CAt4kr5GYnucXpuqhB0ViylEWGzpio41MUbzaLyOQMmYy3xOPipwJa9UZ8TtMIngBZmoCfwbVnNvEr3970mgAVNkHVdTziiApyAKS7Th8Mb+iNUCTvHWgalCegPtzh8x90FovS2jwAcwiFxzWXzLADcGrmIdNhDaZxD4ssZCVOLYLNeIeCLaTz29vveGRJH7ACvH4hukrm7WL11hX8p/s2KFwKBgQD2j0aIamZozV5UD9vgoiH6sVffPV6a2jnTA/DOkciY7KieebSMRLAzPhDJFwLlE+7iaq2uPM5ILPqS2ghzmu8wPu010OUxpW8vI0iDD9pq+9Kilo3bCdqNKQnP/PBeOuGN7yaoB2joPrpDMaNhqI7trhMiCr7O+b8AJOjTMkZPaQKBgQDwZijyEbTIgrDrn5tP0/RRYiuZIPPGCpyYnkISvP7gVzNtGpqF+TB9hN0wGtFSc7o3uV83wBQr2mSBaRlxbTRJXC0V0Klm2QeOM1B0/J92FrpRqemU4jM0oYpAPQyvDOauvi3qgN+HkToabrHtC6ynNhYMSor40kkORHhPqgkYtwKBgQCvwna/LjcZsQk9G+b0N2Ewy3kTv34wLStRKrjDvMtD0oUn1cHEGVpKF3jbdlVxV0C6s2rFtAkFMIeLPGleDGZqliDwSFpHvA5Hvh3ccLOIlOVb3ezxNfPHpvcXBSnEAvgibWJHyd/H1OBflXb5kHwGWyb3B/N+bfmcEdhue0lenwKBgG/6E+kmgxKaeE4ZM9zc8TyDUia+MOaAEf1dFw5v11TNOOZ/LQnpKInfzDddoxXSNnCz2USgSL/yS0+xgV18VpsmbXLr38OOfheFk5C9yFD4Qy1KI4H88Abz5qybbumMRHlR1HTbzd5Y5qeJJ6UwKMIq4fCeDow5H3pjcnZ4lHKBAoGAF4umXZVpBhw1GhAKxPQNdOi/IX1xtOxHfsy9xdu2UPIFCoFY2np3KXjsckKVGKOkHOCXi7loF6PG/8BWZFzZ4Z/Capkzy4Fa0ISE1AWi9xz58DBMb0+ahSX4KNaUf7HDz9b3jAgkAeuuTp+Nf/HvZ7vSPGFIQaLAPevAZaaBdoE="
            }
        ],
        userAttributes: {
            "address": {
                id: "address",
                value: "My College Flat 2 Block C"
            },
            "birthdate": {
                id: "birthdate",
                value: "01/01/1970"
            },
            "familyName": {
                id: "familyName",
                value: "Banks"
            },
            "gender": {
                id: "gender",
                value: "Male"
            },
            "givenName": {
                id: "givenName",
                value: "Bobby"
            },
            "locale": {
                id: "locale",
                value:  "en-US",
            },
            "middleName": {
                id: "middleName",
                value: ""
            },
            "name": {
                id: "name",
                value: "Bob"
            },
            "nickname": {
                id: "nickname",
                value: "Bobby"
            },
            "picture": {
                id: "picture",
                mimeType: "image/png",
                value: btoa('dummybuffer')
            },
            "profile": {
                id: "profile",
                value: "https://example.tld"
            },
            "website": {
                id: "website",
                value: "bobbanks.com"
            },
            "zoneinfo": {
                id: "zoneinfo",
                value:  "UTC+1"
            }
        },
        registrationUri: {
            hostname: "example.org",
            uri: "https://example.org/portal",
            port: -1,
            path: "/portal",
            scheme: "https"
        },
        webAppActor: {
            currentUri: {
                hostname: "example.org",
                uri: "https://example.org/portal",
                port: -1,
                path: "/portal",
                scheme: "https"
            },
            originalUri: {
                hostname: "example.org",
                uri: "https://example.org/portal",
                port: -1,
                path: "/portal",
                scheme: "https"
            }
        },
        date: new Date().toISOString(),
        metadata: {
            category: "",
            changePassUrl: "",
            status: EMetadataStatus.Pending,
            version: 1
        }
    }
]

interface PhaseEntry {
    phase: string;
    session: Session;
}

const _sessions: Array<PhaseEntry> = [
    {
        "phase":"Login_ApproveOperation",
        "session":{"id":"{ca451f5f-8416-4815-83c1-4ca31982340d}","requests":[{"id":"{8b51eb0a-70c9-4c10-b544-dda44df566a6}","type":"Login_ApproveOperation"}],"channel":{"id":"{984059d0-5685-4a3a-80f6-a015e0c230d2}","constraints":{"enableEndToEndEncryption":false,"account":{"category":"Customer","schemaVersion":1}},"webAppActor":{"originalUri":{"uri":"http://localhost:4321/","scheme":"http","hostname":"localhost","port":4321,"path":"/"},"currentUri":{"uri":"http://localhost:4321/","scheme":"http","hostname":"localhost","port":4321,"path":"/"}},"scmActor":{"ed25519Key":"BRTTODO:SCMEd25519PublicKey"}},"operation":{"intent":"PendingDeclaration" as ELoginUserIntent.PendingDeclaration,"requestedUserAttributes":[{"id":"name","required":true},{"id":"picture","required":false},{"id":"gender","required":true},{"id":"birthdate","required":true},{"id":"address","required":true}],"fields":{},"challenges":{},"id":"{ca451f5f-8416-4815-83c1-4ca31982340d}","type":"PendingDeclaration" as EOperationType.PendingDeclaration,"status":"Pending" as EOperationStatus.Pending,"state":{}},"context":{"channel":{"id":"{984059d0-5685-4a3a-80f6-a015e0c230d2}","constraints":{"enableEndToEndEncryption":false,"account":{"category":"Customer","schemaVersion":1}},"webAppActor":{"originalUri":{"uri":"http://localhost:4321/","scheme":"http","hostname":"localhost","port":4321,"path":"/"},"currentUri":{"uri":"http://localhost:4321/","scheme":"http","hostname":"localhost","port":4321,"path":"/"}},"scmActor":{"ed25519Key":"BRTTODO:SCMEd25519PublicKey"}},"document":{"id":10737418242,"uri":{"uri":"http://localhost:4321/","scheme":"http","hostname":"localhost","port":4321,"path":"/"}},"request":{"id":"{8b51eb0a-70c9-4c10-b544-dda44df566a6}","type":"Login_ApproveOperation"}},"metadata":{"version":1,"status":"Pending" as EMetadataStatus.Pending,"category":"Customer","changePassUrl":""},"version":1}
    },
    {
        "phase": "AccountCreation_GetUserAttributes",
        "session": {
          "id": "{c7253bf8-efe8-4ecd-879e-764c1c34c216}",
          "requests": [
            {
              "id": "{d07c346d-617e-44ed-aa74-1cb93c5210b2}",
              "type": "Login_ApproveOperation"
            },
            {
              "id": "{0f388ebf-3a74-46a8-ae12-f224be32b3f2}",
              "type": "AccountCreation_GetUserAttributes"
            }
          ],
          "channel": {
            "id": "{42ef95a8-9c23-41ed-a6c9-8f7ed651d1db}",
            "constraints": {
              "enableEndToEndEncryption": false,
              "account": {
                "category": "Customer",
                "schemaVersion": 1
              }
            },
            "webAppActor": {
              "originalUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              },
              "currentUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              }
            },
            "scmActor": {
              "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
            }
          },
          "operation": {
            "intent": "PendingDeclaration" as ELoginUserIntent,
            "requestedUserAttributes": [
              {
                "id": "name",
                "required": true
              },
              {
                "id": "picture",
                "required": false
              },
              {
                "id": "gender",
                "required": true
              },
              {
                "id": "birthdate",
                "required": true
              },
              {
                "id": "address",
                "required": true
              }
            ],
            "fields": {},
            "challenges": {},
            "id": "{c7253bf8-efe8-4ecd-879e-764c1c34c216}",
            "type": "PendingDeclaration"  as EOperationType,
            "status": "Pending" as EOperationStatus,
            "state": {}
          },
          "context": {
            "channel": {
              "id": "{42ef95a8-9c23-41ed-a6c9-8f7ed651d1db}",
              "constraints": {
                "enableEndToEndEncryption": false,
                "account": {
                  "category": "Customer",
                  "schemaVersion": 1
                }
              },
              "webAppActor": {
                "originalUri": {
                  "uri": "http://localhost:4321/",
                  "scheme": "http",
                  "hostname": "localhost",
                  "port": 4321,
                  "path": "/"
                },
                "currentUri": {
                  "uri": "http://localhost:4321/",
                  "scheme": "http",
                  "hostname": "localhost",
                  "port": 4321,
                  "path": "/"
                }
              },
              "scmActor": {
                "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
              }
            },
            "document": {
              "id": 2,
              "uri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              }
            },
            "request": {
              "id": "{d07c346d-617e-44ed-aa74-1cb93c5210b2}",
              "type": "Login_ApproveOperation"
            }
          },
          "metadata": {
            "version": 1,
            "status": "Pending" as EMetadataStatus,
            "category": "Customer",
            "changePassUrl": ""
          },
          "version": 2,
          "requiredUserAttributes": {
            "name": false,
            "givenName": false,
            "middleName": false,
            "familyName": false,
            "nickname": false,
            "gender": false,
            "birthdate": false,
            "locale": false,
            "address": false,
            "profile": false,
            "picture": false,
            "website": false,
            "zoneinfo": false
          }
        }
    },
    {
      "phase": "AccountCreation_AddField",
      "session": {
        "id": "{bdd28d35-cde1-497a-8a65-cc641ab1d073}",
        "requests": [
          {
            "id": "{a3ee72e0-d3ed-471f-bcd8-db2478a920e7}",
            "type": "Login_ApproveOperation"
          },
          {
            "id": "{282d341b-7ca7-462a-9e0e-bcd370729391}",
            "type": "AccountCreation_GetUserAttributes"
          },
          {
            "id": "{d2a82055-1b6a-4074-9d2e-0fbcfe4d8e63}",
            "type": "AccountCreation_AddField"
          }
        ],
        "channel": {
          "id": "{91fae47b-af96-4f91-86e6-36630d6ec626}",
          "constraints": {
            "enableEndToEndEncryption": false,
            "account": {
              "category": "Customer",
              "schemaVersion": 1
            }
          },
          "webAppActor": {
            "originalUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            },
            "currentUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "scmActor": {
            "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
          }
        },
        "operation": {
          "intent": "PendingDeclaration" as ELoginUserIntent,
          "requestedUserAttributes": [
            {
              "id": "name",
              "required": true
            },
            {
              "id": "picture",
              "required": false
            },
            {
              "id": "gender",
              "required": true
            },
            {
              "id": "birthdate",
              "required": true
            },
            {
              "id": "address",
              "required": true
            }
          ],
          "fields": {},
          "challenges": {},
          "id": "{bdd28d35-cde1-497a-8a65-cc641ab1d073}",
          "type": "PendingDeclaration" as EOperationType,
          "status": "Pending" as EOperationStatus,
          "state": {}
        },
        "context": {
          "channel": {
            "id": "{91fae47b-af96-4f91-86e6-36630d6ec626}",
            "constraints": {
              "enableEndToEndEncryption": false,
              "account": {
                "category": "Customer",
                "schemaVersion": 1
              }
            },
            "webAppActor": {
              "originalUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              },
              "currentUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              }
            },
            "scmActor": {
              "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
            }
          },
          "document": {
            "id": 2,
            "uri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "request": {
            "id": "{a3ee72e0-d3ed-471f-bcd8-db2478a920e7}",
            "type": "Login_ApproveOperation"
          }
        },
        "metadata": {
          "version": 1,
          "status": "Pending" as EMetadataStatus,
          "category": "Customer",
          "changePassUrl": ""
        },
        "version": 3,
        "requiredUserAttributes": {
          "name": false,
          "givenName": false,
          "middleName": false,
          "familyName": false,
          "nickname": false,
          "gender": false,
          "birthdate": false,
          "locale": false,
          "address": false,
          "profile": false,
          "picture": false,
          "website": false,
          "zoneinfo": false
        },
        "userAttributes": {
          "name": {
            "id": "name",
            "mimeType": "text/plain",
            "value": "John Bobby"
          },
          "givenName": {
            "id": "givenName",
            "mimeType": "text/plain",
            "value": "John"
          },
          "middleName": {
            "id": "middleName",
            "mimeType": "text/plain",
            "value": "A."
          },
          "familyName": {
            "id": "familyName",
            "mimeType": "text/plain",
            "value": "Bobby"
          },
          "nickname": {
            "id": "nickname",
            "mimeType": "text/plain",
            "value": "Bobby"
          },
          "gender": {
            "id": "gender",
            "mimeType": "text/plain",
            "value": "Male"
          },
          "birthdate": {
            "id": "birthdate",
            "mimeType": "text/plain",
            "value": "1970-12-12"
          },
          "locale": {
            "id": "locale",
            "mimeType": "text/plain",
            "value": "en_US"
          },
          "address": {
            "id": "address",
            "mimeType": "text/plain",
            "value": "12 Atlantic road"
          },
          "profile": {
            "id": "profile",
            "mimeType": "text/plain",
            "value": "https://bob.site/me"
          },
          "website": {
            "id": "website",
            "mimeType": "text/plain",
            "value": "https://blog.bob.site/"
          },
          "zoneinfo": {
            "id": "zoneinfo",
            "mimeType": "text/plain",
            "value": "Europe/London"
          }
        },
        "createFieldOptions": [
          {
            "id": "username",
            "type": "Identity" as EBerytusFieldType,
            "options": {
              "humanReadable": true,
              "private": false,
              "maxLength": 24
            }
          }
        ]
      }
    },
    {
      "phase": "AccountCreation_AddField",
      "session": {
        "id": "{bdd28d35-cde1-497a-8a65-cc641ab1d073}",
        "requests": [
          {
            "id": "{a3ee72e0-d3ed-471f-bcd8-db2478a920e7}",
            "type": "Login_ApproveOperation"
          },
          {
            "id": "{282d341b-7ca7-462a-9e0e-bcd370729391}",
            "type": "AccountCreation_GetUserAttributes"
          },
          {
            "id": "{d2a82055-1b6a-4074-9d2e-0fbcfe4d8e63}",
            "type": "AccountCreation_AddField"
          },
          {
            "id": "{942b94d7-c113-45ca-8cca-a562995f044e}",
            "type": "AccountCreation_AddField"
          }
        ],
        "channel": {
          "id": "{91fae47b-af96-4f91-86e6-36630d6ec626}",
          "constraints": {
            "enableEndToEndEncryption": false,
            "account": {
              "category": "Customer",
              "schemaVersion": 1
            }
          },
          "webAppActor": {
            "originalUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            },
            "currentUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "scmActor": {
            "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
          }
        },
        "operation": {
          "intent": "PendingDeclaration" as ELoginUserIntent,
          "requestedUserAttributes": [
            {
              "id": "name",
              "required": true
            },
            {
              "id": "picture",
              "required": false
            },
            {
              "id": "gender",
              "required": true
            },
            {
              "id": "birthdate",
              "required": true
            },
            {
              "id": "address",
              "required": true
            }
          ],
          "fields": {},
          "challenges": {},
          "id": "{bdd28d35-cde1-497a-8a65-cc641ab1d073}",
          "type": "PendingDeclaration" as EOperationType,
          "status": "Pending" as EOperationStatus,
          "state": {}
        },
        "context": {
          "channel": {
            "id": "{91fae47b-af96-4f91-86e6-36630d6ec626}",
            "constraints": {
              "enableEndToEndEncryption": false,
              "account": {
                "category": "Customer",
                "schemaVersion": 1
              }
            },
            "webAppActor": {
              "originalUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              },
              "currentUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              }
            },
            "scmActor": {
              "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
            }
          },
          "document": {
            "id": 2,
            "uri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "request": {
            "id": "{a3ee72e0-d3ed-471f-bcd8-db2478a920e7}",
            "type": "Login_ApproveOperation"
          }
        },
        "metadata": {
          "version": 1,
          "status": "Pending" as EMetadataStatus,
          "category": "Customer",
          "changePassUrl": ""
        },
        "version": 4,
        "requiredUserAttributes": {
          "name": false,
          "givenName": false,
          "middleName": false,
          "familyName": false,
          "nickname": false,
          "gender": false,
          "birthdate": false,
          "locale": false,
          "address": false,
          "profile": false,
          "picture": false,
          "website": false,
          "zoneinfo": false
        },
        "userAttributes": {
          "name": {
            "id": "name",
            "mimeType": "text/plain",
            "value": "John Bobby"
          },
          "givenName": {
            "id": "givenName",
            "mimeType": "text/plain",
            "value": "John"
          },
          "middleName": {
            "id": "middleName",
            "mimeType": "text/plain",
            "value": "A."
          },
          "familyName": {
            "id": "familyName",
            "mimeType": "text/plain",
            "value": "Bobby"
          },
          "nickname": {
            "id": "nickname",
            "mimeType": "text/plain",
            "value": "Bobby"
          },
          "gender": {
            "id": "gender",
            "mimeType": "text/plain",
            "value": "Male"
          },
          "birthdate": {
            "id": "birthdate",
            "mimeType": "text/plain",
            "value": "1970-12-12"
          },
          "locale": {
            "id": "locale",
            "mimeType": "text/plain",
            "value": "en_US"
          },
          "address": {
            "id": "address",
            "mimeType": "text/plain",
            "value": "12 Atlantic road"
          },
          "profile": {
            "id": "profile",
            "mimeType": "text/plain",
            "value": "https://bob.site/me"
          },
          "website": {
            "id": "website",
            "mimeType": "text/plain",
            "value": "https://blog.bob.site/"
          },
          "zoneinfo": {
            "id": "zoneinfo",
            "mimeType": "text/plain",
            "value": "Europe/London"
          }
        },
        "createFieldOptions": [
          {
            "id": "username",
            "type": "Identity" as EBerytusFieldType,
            "options": {
              "humanReadable": true,
              "private": false,
              "maxLength": 24
            }
          },
          {
            "id": "password",
            "type": "Password" as EBerytusFieldType,
            "options": {}
          }
        ],
        "fields": [
          {
            "id": "username",
            "type": "Identity" as EBerytusFieldType,
            "options": {
              "humanReadable": true,
              "private": false,
              "maxLength": 24
            },
            "value": "LargeEel18zz"
          }
        ]
      }
    },
    {
      "phase": "Login_CloseOperation",
      "session": {
        "id": "{f1b3cfa8-e553-4e06-9f47-86a88c627191}",
        "requests": [
          {
            "id": "{59a93dec-a9c2-4397-a150-506b0dfd0411}",
            "type": "Login_ApproveOperation"
          },
          {
            "id": "{5a05d7a6-fbad-412e-ac66-155ac817c9b1}",
            "type": "AccountCreation_GetUserAttributes"
          },
          {
            "id": "{8ef78e4c-3d8b-4497-9518-005765e08c04}",
            "type": "AccountCreation_AddField"
          },
          {
            "id": "{0a15a6ec-ae6e-4d65-892f-41959d47c86c}",
            "type": "AccountCreation_AddField"
          },
          {
            "id": "{ad9c018e-0d36-4d1c-a30a-60a1f348d7c7}",
            "type": "Login_CloseOperation"
          }
        ],
        "channel": {
          "id": "{c3e7723a-5cc8-4eee-ae42-c4821cc59209}",
          "constraints": {
            "enableEndToEndEncryption": false,
            "account": {
              "category": "Customer",
              "schemaVersion": 1
            }
          },
          "webAppActor": {
            "originalUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            },
            "currentUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "scmActor": {
            "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
          }
        },
        "operation": {
          "intent": "PendingDeclaration" as ELoginUserIntent,
          "requestedUserAttributes": [
            {
              "id": "name",
              "required": true
            },
            {
              "id": "picture",
              "required": false
            },
            {
              "id": "gender",
              "required": true
            },
            {
              "id": "birthdate",
              "required": true
            },
            {
              "id": "address",
              "required": true
            }
          ],
          "fields": {},
          "challenges": {},
          "id": "{f1b3cfa8-e553-4e06-9f47-86a88c627191}",
          "type": "PendingDeclaration" as EOperationType,
          "status": "Pending" as EOperationStatus,
          "state": {}
        },
        "context": {
          "channel": {
            "id": "{c3e7723a-5cc8-4eee-ae42-c4821cc59209}",
            "constraints": {
              "enableEndToEndEncryption": false,
              "account": {
                "category": "Customer",
                "schemaVersion": 1
              }
            },
            "webAppActor": {
              "originalUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              },
              "currentUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              }
            },
            "scmActor": {
              "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
            }
          },
          "document": {
            "id": 2,
            "uri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "request": {
            "id": "{59a93dec-a9c2-4397-a150-506b0dfd0411}",
            "type": "Login_ApproveOperation"
          }
        },
        "metadata": {
          "version": 1,
          "status": "Created" as EMetadataStatus,
          "category": "Customer",
          "changePassUrl": ""
        },
        "version": 8,
        "requiredUserAttributes": {
          "name": false,
          "givenName": false,
          "middleName": false,
          "familyName": false,
          "nickname": false,
          "gender": false,
          "birthdate": false,
          "locale": false,
          "address": false,
          "profile": false,
          "picture": false,
          "website": false,
          "zoneinfo": false
        },
        "userAttributes": {
          "name": {
            "id": "name",
            "mimeType": "text/plain",
            "value": "John Bobby"
          },
          "givenName": {
            "id": "givenName",
            "mimeType": "text/plain",
            "value": "John"
          },
          "middleName": {
            "id": "middleName",
            "mimeType": "text/plain",
            "value": "A."
          },
          "familyName": {
            "id": "familyName",
            "mimeType": "text/plain",
            "value": "Bobby"
          },
          "nickname": {
            "id": "nickname",
            "mimeType": "text/plain",
            "value": "Bobby"
          },
          "gender": {
            "id": "gender",
            "mimeType": "text/plain",
            "value": "Male"
          },
          "birthdate": {
            "id": "birthdate",
            "mimeType": "text/plain",
            "value": "1970-12-12"
          },
          "locale": {
            "id": "locale",
            "mimeType": "text/plain",
            "value": "en_US"
          },
          "address": {
            "id": "address",
            "mimeType": "text/plain",
            "value": "12 Atlantic road"
          },
          "profile": {
            "id": "profile",
            "mimeType": "text/plain",
            "value": "https://bob.site/me"
          },
          "website": {
            "id": "website",
            "mimeType": "text/plain",
            "value": "https://blog.bob.site/"
          },
          "zoneinfo": {
            "id": "zoneinfo",
            "mimeType": "text/plain",
            "value": "Europe/London"
          }
        },
        "createFieldOptions": [
          {
            "id": "username",
            "type": "Identity" as EBerytusFieldType,
            "options": {
              "humanReadable": true,
              "private": false,
              "maxLength": 24
            }
          },
          {
            "id": "password",
            "type": "Password" as EBerytusFieldType,
            "options": {}
          }
        ],
        "fields": [
          {
            "id": "username",
            "type": "Identity" as EBerytusFieldType,
            "options": {
              "humanReadable": true,
              "private": false,
              "maxLength": 24
            },
            "value": "Anchorrcwz"
          },
          {
            "id": "password",
            "type": "Password" as EBerytusFieldType,
            "options": {},
            "value": "7btw-n6jn-sx7p-e79u"
          }
        ],
        "createdAccountId": "7ef5-8pss-rbzh-6ld5",
        "closed": true
      }
    },
    {
      "phase": "AccountCreation_ApproveTransitionToAuthOp",
      "session": {
        "id": "{84029dfe-4a64-497f-815f-1f6fee123931}",
        "channel": {
          "id": "{91fae47b-af96-4f91-86e6-36630d6ec626}",
          "constraints": {
            "enableEndToEndEncryption": false,
            "account": {
              "category": "Customer",
              "schemaVersion": 1
            }
          },
          "webAppActor": {
            "originalUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            },
            "currentUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "scmActor": {
            "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
          }
        },
        "operation": {
          "intent": "Authenticate" as ELoginUserIntent,
          "requestedUserAttributes": [],
          "fields": {},
          "challenges": {},
          "id": "{84029dfe-4a64-497f-815f-1f6fee123931}",
          "type": "Authentication" as EOperationType,
          "status": "Pending" as EOperationStatus,
          "state": {}
        },
        "requests": [
          {
            "id": "{9d6f5428-3120-4e46-a1c6-b27815821c34}",
            "type": "AccountCreation_ApproveTransitionToAuthOp"
          }
        ],
        "transitionedFromSessionId": "{bdd28d35-cde1-497a-8a65-cc641ab1d073}",
        "context": {
          "operation": {
            "id": "{bdd28d35-cde1-497a-8a65-cc641ab1d073}",
            "type": "Registration" as EOperationType,
            "status": "Finished" as EOperationStatus,
            "state": {}
          },
          "channel": {
            "id": "{91fae47b-af96-4f91-86e6-36630d6ec626}",
            "constraints": {
              "enableEndToEndEncryption": false,
              "account": {
                "category": "Customer",
                "schemaVersion": 1
              }
            },
            "webAppActor": {
              "originalUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              },
              "currentUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              }
            },
            "scmActor": {
              "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
            }
          },
          "document": {
            "id": 2,
            "uri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "request": {
            "id": "{9d6f5428-3120-4e46-a1c6-b27815821c34}",
            "type": "AccountCreation_ApproveTransitionToAuthOp"
          }
        },
        "selectedAccountId": "lm8j-bx0h-6v3b-o232",
        "metadata": { 
          "version": 1,
          "status": "Created" as EMetadataStatus,
          "category": "Customer",
          "changePassUrl": ""
        },
        "version": 1
      }
    },
    {
      "phase": "AccountAuthentication_ApproveChallengeRequest",
      "session": {
        "id": "{358fe212-cc2f-492b-8a07-f252437ce56b}",
        "requests": [
          {
            "id": "{f4bb253f-368c-4724-8239-d3ccee038f8d}",
            "type": "Login_ApproveOperation"
          },
          {
            "id": "{b492b35a-a126-4f57-a50c-62ee4fffe51f}",
            "type": "AccountAuthentication_ApproveChallengeRequest"
          }
        ],
        "channel": {
          "id": "{30f67fc0-013c-4a08-9251-14d853cec3dc}",
          "constraints": {
            "enableEndToEndEncryption": false,
            "account": {
              "category": "Customer",
              "schemaVersion": 1
            }
          },
          "webAppActor": {
            "originalUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            },
            "currentUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "scmActor": {
            "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
          }
        },
        "operation": {
          "intent": "PendingDeclaration" as ELoginUserIntent,
          "requestedUserAttributes": [
            {
              "id": "name",
              "required": true
            },
            {
              "id": "picture",
              "required": false
            },
            {
              "id": "gender",
              "required": true
            },
            {
              "id": "birthdate",
              "required": true
            },
            {
              "id": "address",
              "required": true
            }
          ],
          "fields": {},
          "challenges": {},
          "id": "{358fe212-cc2f-492b-8a07-f252437ce56b}",
          "type": "PendingDeclaration" as EOperationType,
          "status": "Pending" as EOperationStatus,
          "state": {}
        },
        "context": {
          "channel": {
            "id": "{30f67fc0-013c-4a08-9251-14d853cec3dc}",
            "constraints": {
              "enableEndToEndEncryption": false,
              "account": {
                "category": "Customer",
                "schemaVersion": 1
              }
            },
            "webAppActor": {
              "originalUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              },
              "currentUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              }
            },
            "scmActor": {
              "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
            }
          },
          "document": {
            "id": 1,
            "uri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "request": {
            "id": "{f4bb253f-368c-4724-8239-d3ccee038f8d}",
            "type": "Login_ApproveOperation"
          }
        },
        "metadata": {
          "version": 1,
          "status": "Pending" as EMetadataStatus,
          "category": "Customer",
          "changePassUrl": ""
        },
        "version": 2,
        "selectedAccountId": "ic49-aewd-z9se-hgai",
        "challenges": {
          "id": {
            "id": "id",
            "type": "Identification" as EBerytusChallengeType.Identification,
            "parameters": null,
            "messages": {}
          }
        }
      }
    },
    {
      "phase": "AccountAuthentication_RespondToChallengeMessage",
      "session": {
        "id": "{358fe212-cc2f-492b-8a07-f252437ce56b}",
        "requests": [
          {
            "id": "{f4bb253f-368c-4724-8239-d3ccee038f8d}",
            "type": "Login_ApproveOperation"
          },
          {
            "id": "{b492b35a-a126-4f57-a50c-62ee4fffe51f}",
            "type": "AccountAuthentication_ApproveChallengeRequest"
          },
          {
            "id": "{1f8f1714-a050-498f-881e-af24f9545072}",
            "type": "AccountAuthentication_RespondToChallengeMessage"
          }
        ],
        "channel": {
          "id": "{30f67fc0-013c-4a08-9251-14d853cec3dc}",
          "constraints": {
            "enableEndToEndEncryption": false,
            "account": {
              "category": "Customer",
              "schemaVersion": 1
            }
          },
          "webAppActor": {
            "originalUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            },
            "currentUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "scmActor": {
            "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
          }
        },
        "operation": {
          "intent": "PendingDeclaration" as ELoginUserIntent,
          "requestedUserAttributes": [
            {
              "id": "name",
              "required": true
            },
            {
              "id": "picture",
              "required": false
            },
            {
              "id": "gender",
              "required": true
            },
            {
              "id": "birthdate",
              "required": true
            },
            {
              "id": "address",
              "required": true
            }
          ],
          "fields": {},
          "challenges": {},
          "id": "{358fe212-cc2f-492b-8a07-f252437ce56b}",
          "type": "PendingDeclaration" as EOperationType,
          "status": "Pending" as EOperationStatus,
          "state": {}
        },
        "context": {
          "channel": {
            "id": "{30f67fc0-013c-4a08-9251-14d853cec3dc}",
            "constraints": {
              "enableEndToEndEncryption": false,
              "account": {
                "category": "Customer",
                "schemaVersion": 1
              }
            },
            "webAppActor": {
              "originalUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              },
              "currentUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              }
            },
            "scmActor": {
              "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
            }
          },
          "document": {
            "id": 1,
            "uri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "request": {
            "id": "{f4bb253f-368c-4724-8239-d3ccee038f8d}",
            "type": "Login_ApproveOperation"
          }
        },
        "metadata": {
          "version": 1,
          "status": "Pending" as EMetadataStatus,
          "category": "Customer",
          "changePassUrl": ""
        },
        "version": 3,
        "selectedAccountId": "ic49-aewd-z9se-hgai",
        "challenges": {
          "id": {
            "id": "id",
            "type": "Identification" as EBerytusChallengeType.Identification,
            "parameters": null,
            "messages": {
              "GetIdentityFields": {
                "name": "GetIdentityFields" as EBerytusIdentificationChallengeMessageName.GetIdentityFields,
                "payload": [
                  "username"
                ]
              }
            }
          }
        }
      }
    },
    {
      "phase": "AccountAuthentication_CloseChallenge",
      "session": {
        "id": "{358fe212-cc2f-492b-8a07-f252437ce56b}",
        "requests": [
          {
            "id": "{f4bb253f-368c-4724-8239-d3ccee038f8d}",
            "type": "Login_ApproveOperation"
          },
          {
            "id": "{b492b35a-a126-4f57-a50c-62ee4fffe51f}",
            "type": "AccountAuthentication_ApproveChallengeRequest"
          },
          {
            "id": "{1f8f1714-a050-498f-881e-af24f9545072}",
            "type": "AccountAuthentication_RespondToChallengeMessage"
          },
          {
            "id": "{04a6669f-ed02-4407-9dbe-de9eab2e82e2}",
            "type": "AccountAuthentication_CloseChallenge"
          }
        ],
        "channel": {
          "id": "{30f67fc0-013c-4a08-9251-14d853cec3dc}",
          "constraints": {
            "enableEndToEndEncryption": false,
            "account": {
              "category": "Customer",
              "schemaVersion": 1
            }
          },
          "webAppActor": {
            "originalUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            },
            "currentUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "scmActor": {
            "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
          }
        },
        "operation": {
          "intent": "PendingDeclaration" as ELoginUserIntent,
          "requestedUserAttributes": [
            {
              "id": "name",
              "required": true
            },
            {
              "id": "picture",
              "required": false
            },
            {
              "id": "gender",
              "required": true
            },
            {
              "id": "birthdate",
              "required": true
            },
            {
              "id": "address",
              "required": true
            }
          ],
          "fields": {},
          "challenges": {},
          "id": "{358fe212-cc2f-492b-8a07-f252437ce56b}",
          "type": "PendingDeclaration" as EOperationType,
          "status": "Pending" as EOperationStatus,
          "state": {}
        },
        "context": {
          "channel": {
            "id": "{30f67fc0-013c-4a08-9251-14d853cec3dc}",
            "constraints": {
              "enableEndToEndEncryption": false,
              "account": {
                "category": "Customer",
                "schemaVersion": 1
              }
            },
            "webAppActor": {
              "originalUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              },
              "currentUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              }
            },
            "scmActor": {
              "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
            }
          },
          "document": {
            "id": 1,
            "uri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "request": {
            "id": "{f4bb253f-368c-4724-8239-d3ccee038f8d}",
            "type": "Login_ApproveOperation"
          }
        },
        "metadata": {
          "version": 1,
          "status": "Pending" as EMetadataStatus,
          "category": "Customer",
          "changePassUrl": ""
        },
        "version": 3,
        "selectedAccountId": "ic49-aewd-z9se-hgai",
        "challenges": {
          "id": {
            "id": "id",
            "type": "Identification" as EBerytusChallengeType.Identification,
            "parameters": null,
            "messages": {
              "GetIdentityFields": {
                "name": "GetIdentityFields" as EBerytusIdentificationChallengeMessageName.GetIdentityFields,
                "payload": [
                  "username"
                ]
              }
            },
            "closed": true
          }
        }
      }
    },
    {
      "phase": "AccountAuthentication_ApproveChallengeRequest",
      "session": {
        "id": "{358fe212-cc2f-492b-8a07-f252437ce56b}",
        "requests": [
          {
            "id": "{f4bb253f-368c-4724-8239-d3ccee038f8d}",
            "type": "Login_ApproveOperation"
          },
          {
            "id": "{b492b35a-a126-4f57-a50c-62ee4fffe51f}",
            "type": "AccountAuthentication_ApproveChallengeRequest"
          },
          {
            "id": "{1f8f1714-a050-498f-881e-af24f9545072}",
            "type": "AccountAuthentication_RespondToChallengeMessage"
          },
          {
            "id": "{04a6669f-ed02-4407-9dbe-de9eab2e82e2}",
            "type": "AccountAuthentication_CloseChallenge"
          },
          {
            "id": "{2e9e8fda-8341-4a19-9cc8-483678c38a25}",
            "type": "AccountAuthentication_ApproveChallengeRequest"
          }
        ],
        "channel": {
          "id": "{30f67fc0-013c-4a08-9251-14d853cec3dc}",
          "constraints": {
            "enableEndToEndEncryption": false,
            "account": {
              "category": "Customer",
              "schemaVersion": 1
            }
          },
          "webAppActor": {
            "originalUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            },
            "currentUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "scmActor": {
            "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
          }
        },
        "operation": {
          "intent": "PendingDeclaration" as ELoginUserIntent,
          "requestedUserAttributes": [
            {
              "id": "name",
              "required": true
            },
            {
              "id": "picture",
              "required": false
            },
            {
              "id": "gender",
              "required": true
            },
            {
              "id": "birthdate",
              "required": true
            },
            {
              "id": "address",
              "required": true
            }
          ],
          "fields": {},
          "challenges": {},
          "id": "{358fe212-cc2f-492b-8a07-f252437ce56b}",
          "type": "PendingDeclaration" as EOperationType,
          "status": "Pending" as EOperationStatus,
          "state": {}
        },
        "context": {
          "channel": {
            "id": "{30f67fc0-013c-4a08-9251-14d853cec3dc}",
            "constraints": {
              "enableEndToEndEncryption": false,
              "account": {
                "category": "Customer",
                "schemaVersion": 1
              }
            },
            "webAppActor": {
              "originalUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              },
              "currentUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              }
            },
            "scmActor": {
              "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
            }
          },
          "document": {
            "id": 1,
            "uri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "request": {
            "id": "{f4bb253f-368c-4724-8239-d3ccee038f8d}",
            "type": "Login_ApproveOperation"
          }
        },
        "metadata": {
          "version": 1,
          "status": "Pending" as EMetadataStatus,
          "category": "Customer",
          "changePassUrl": ""
        },
        "version": 4,
        "selectedAccountId": "ic49-aewd-z9se-hgai",
        "challenges": {
          "id": {
            "id": "id",
            "type": "Identification" as EBerytusChallengeType.Identification,
            "parameters": null,
            "messages": {
              "GetIdentityFields": {
                "name": "GetIdentityFields" as EBerytusIdentificationChallengeMessageName.GetIdentityFields,
                "payload": [
                  "username"
                ]
              }
            },
            "closed": true
          },
          "pass": {
            "id": "pass",
            "type": "Password" as EBerytusChallengeType.Password,
            "parameters": null,
            "messages": {}
          }
        }
      }
    },
    {
      "phase": "AccountAuthentication_RespondToChallengeMessage",
      "session": {
        "id": "{358fe212-cc2f-492b-8a07-f252437ce56b}",
        "requests": [
          {
            "id": "{f4bb253f-368c-4724-8239-d3ccee038f8d}",
            "type": "Login_ApproveOperation"
          },
          {
            "id": "{b492b35a-a126-4f57-a50c-62ee4fffe51f}",
            "type": "AccountAuthentication_ApproveChallengeRequest"
          },
          {
            "id": "{1f8f1714-a050-498f-881e-af24f9545072}",
            "type": "AccountAuthentication_RespondToChallengeMessage"
          },
          {
            "id": "{04a6669f-ed02-4407-9dbe-de9eab2e82e2}",
            "type": "AccountAuthentication_CloseChallenge"
          },
          {
            "id": "{2e9e8fda-8341-4a19-9cc8-483678c38a25}",
            "type": "AccountAuthentication_ApproveChallengeRequest"
          },
          {
            "id": "{f500327c-cc4f-456e-ae23-db9a6f2b59ac}",
            "type": "AccountAuthentication_RespondToChallengeMessage"
          }
        ],
        "channel": {
          "id": "{30f67fc0-013c-4a08-9251-14d853cec3dc}",
          "constraints": {
            "enableEndToEndEncryption": false,
            "account": {
              "category": "Customer",
              "schemaVersion": 1
            }
          },
          "webAppActor": {
            "originalUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            },
            "currentUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "scmActor": {
            "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
          }
        },
        "operation": {
          "intent": "PendingDeclaration" as ELoginUserIntent,
          "requestedUserAttributes": [
            {
              "id": "name",
              "required": true
            },
            {
              "id": "picture",
              "required": false
            },
            {
              "id": "gender",
              "required": true
            },
            {
              "id": "birthdate",
              "required": true
            },
            {
              "id": "address",
              "required": true
            }
          ],
          "fields": {},
          "challenges": {},
          "id": "{358fe212-cc2f-492b-8a07-f252437ce56b}",
          "type": "PendingDeclaration" as EOperationType,
          "status": "Pending" as EOperationStatus,
          "state": {}
        },
        "context": {
          "channel": {
            "id": "{30f67fc0-013c-4a08-9251-14d853cec3dc}",
            "constraints": {
              "enableEndToEndEncryption": false,
              "account": {
                "category": "Customer",
                "schemaVersion": 1
              }
            },
            "webAppActor": {
              "originalUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              },
              "currentUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              }
            },
            "scmActor": {
              "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
            }
          },
          "document": {
            "id": 1,
            "uri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "request": {
            "id": "{f4bb253f-368c-4724-8239-d3ccee038f8d}",
            "type": "Login_ApproveOperation"
          }
        },
        "metadata": {
          "version": 1,
          "status": "Pending" as EMetadataStatus,
          "category": "Customer",
          "changePassUrl": ""
        },
        "version": 5,
        "selectedAccountId": "ic49-aewd-z9se-hgai",
        "challenges": {
          "id": {
            "id": "id",
            "type": "Identification" as EBerytusChallengeType.Identification,
            "parameters": null,
            "messages": {
              "GetIdentityFields": {
                "name": "GetIdentityFields" as EBerytusIdentificationChallengeMessageName.GetIdentityFields,
                "payload": [
                  "username"
                ]
              }
            },
            "closed": true
          },
          "pass": {
            "id": "pass",
            "type": "Password" as EBerytusChallengeType.Password,
            "parameters": null,
            "messages": {
              "GetPasswordFields": {
                "name": "GetPasswordFields" as EBerytusPasswordChallengeMessageName.GetPasswordFields,
                "payload": [
                  "password"
                ]
              }
            }
          }
        }
      }
    },
    {
      "phase": "AccountAuthentication_CloseChallenge",
      "session": {
        "id": "{358fe212-cc2f-492b-8a07-f252437ce56b}",
        "requests": [
          {
            "id": "{f4bb253f-368c-4724-8239-d3ccee038f8d}",
            "type": "Login_ApproveOperation"
          },
          {
            "id": "{b492b35a-a126-4f57-a50c-62ee4fffe51f}",
            "type": "AccountAuthentication_ApproveChallengeRequest"
          },
          {
            "id": "{1f8f1714-a050-498f-881e-af24f9545072}",
            "type": "AccountAuthentication_RespondToChallengeMessage"
          },
          {
            "id": "{04a6669f-ed02-4407-9dbe-de9eab2e82e2}",
            "type": "AccountAuthentication_CloseChallenge"
          },
          {
            "id": "{2e9e8fda-8341-4a19-9cc8-483678c38a25}",
            "type": "AccountAuthentication_ApproveChallengeRequest"
          },
          {
            "id": "{f500327c-cc4f-456e-ae23-db9a6f2b59ac}",
            "type": "AccountAuthentication_RespondToChallengeMessage"
          },
          {
            "id": "{0b080c24-186d-4e9d-ad41-d7e5766b9e93}",
            "type": "AccountAuthentication_CloseChallenge"
          }
        ],
        "channel": {
          "id": "{30f67fc0-013c-4a08-9251-14d853cec3dc}",
          "constraints": {
            "enableEndToEndEncryption": false,
            "account": {
              "category": "Customer",
              "schemaVersion": 1
            }
          },
          "webAppActor": {
            "originalUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            },
            "currentUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "scmActor": {
            "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
          }
        },
        "operation": {
          "intent": "PendingDeclaration" as ELoginUserIntent,
          "requestedUserAttributes": [
            {
              "id": "name",
              "required": true
            },
            {
              "id": "picture",
              "required": false
            },
            {
              "id": "gender",
              "required": true
            },
            {
              "id": "birthdate",
              "required": true
            },
            {
              "id": "address",
              "required": true
            }
          ],
          "fields": {},
          "challenges": {},
          "id": "{358fe212-cc2f-492b-8a07-f252437ce56b}",
          "type": "PendingDeclaration" as EOperationType,
          "status": "Pending" as EOperationStatus,
          "state": {}
        },
        "context": {
          "channel": {
            "id": "{30f67fc0-013c-4a08-9251-14d853cec3dc}",
            "constraints": {
              "enableEndToEndEncryption": false,
              "account": {
                "category": "Customer",
                "schemaVersion": 1
              }
            },
            "webAppActor": {
              "originalUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              },
              "currentUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              }
            },
            "scmActor": {
              "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
            }
          },
          "document": {
            "id": 1,
            "uri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "request": {
            "id": "{f4bb253f-368c-4724-8239-d3ccee038f8d}",
            "type": "Login_ApproveOperation"
          }
        },
        "metadata": {
          "version": 1,
          "status": "Pending" as EMetadataStatus,
          "category": "Customer",
          "changePassUrl": ""
        },
        "version": 5,
        "selectedAccountId": "ic49-aewd-z9se-hgai",
        "challenges": {
          "id": {
            "id": "id",
            "type": "Identification" as EBerytusChallengeType.Identification,
            "parameters": null,
            "messages": {
              "GetIdentityFields": {
                "name": "GetIdentityFields" as EBerytusIdentificationChallengeMessageName.GetIdentityFields,
                "payload": [
                  "username"
                ]
              }
            },
            "closed": true
          },
          "pass": {
            "id": "pass",
            "type": "Password" as EBerytusChallengeType.Password,
            "parameters": null,
            "messages": {
              "GetPasswordFields": {
                "name": "GetPasswordFields" as EBerytusPasswordChallengeMessageName.GetPasswordFields,
                "payload": [
                  "password"
                ]
              }
            },
            "closed": true
          }
        }
      }
    },
    {
      "phase": "AccountAuthentication_AbortChallenge",
      "session": {
        "id": "{a8f3ebd2-abbc-40f6-8589-2fbbca172c5b}",
        "requests": [
          {
            "id": "{29e0c41c-635f-40ba-8caa-4e97b8872bb6}",
            "type": "Login_ApproveOperation"
          },
          {
            "id": "{8bf8c868-71ca-4c21-9f96-edae4d2fc3ca}",
            "type": "AccountAuthentication_ApproveChallengeRequest"
          },
          {
            "id": "{1201f2d5-7174-4efc-a5b5-f1011db28298}",
            "type": "AccountAuthentication_RespondToChallengeMessage"
          },
          {
            "id": "{6813d1ba-7cca-499d-99c1-d8b6041f9414}",
            "type": "AccountAuthentication_CloseChallenge"
          },
          {
            "id": "{e6b61ab7-863c-4301-8e36-fff002abe90c}",
            "type": "AccountAuthentication_ApproveChallengeRequest"
          },
          {
            "id": "{317af8ab-dc2b-4b2b-b1f1-4848cb214e04}",
            "type": "AccountAuthentication_RespondToChallengeMessage"
          },
          {
            "id": "{d73ea9af-7fef-487b-b6d8-75209e2a6946}",
            "type": "AccountAuthentication_RespondToChallengeMessage"
          },
          {
            "id": "{ddfcb487-f51f-48f5-b621-382e86c3215a}",
            "type": "AccountAuthentication_AbortChallenge"
          }
        ],
        "channel": {
          "id": "{c62e23e8-fe4d-4ead-a8a6-1d6dc660697a}",
          "constraints": {
            "enableEndToEndEncryption": false,
            "account": {
              "category": "Customer",
              "schemaVersion": 3
            }
          },
          "webAppActor": {
            "originalUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            },
            "currentUri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "scmActor": {
            "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
          }
        },
        "operation": {
          "intent": "PendingDeclaration" as ELoginUserIntent,
          "requestedUserAttributes": [
            {
              "id": "name",
              "required": true
            },
            {
              "id": "picture",
              "required": false
            },
            {
              "id": "gender",
              "required": true
            },
            {
              "id": "birthdate",
              "required": true
            },
            {
              "id": "address",
              "required": true
            }
          ],
          "fields": {},
          "challenges": {},
          "id": "{a8f3ebd2-abbc-40f6-8589-2fbbca172c5b}",
          "type": "PendingDeclaration" as EOperationType,
          "status": "Pending" as EOperationStatus,
          "state": {}
        },
        "context": {
          "channel": {
            "id": "{c62e23e8-fe4d-4ead-a8a6-1d6dc660697a}",
            "constraints": {
              "enableEndToEndEncryption": false,
              "account": {
                "category": "Customer",
                "schemaVersion": 3
              }
            },
            "webAppActor": {
              "originalUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              },
              "currentUri": {
                "uri": "http://localhost:4321/",
                "scheme": "http",
                "hostname": "localhost",
                "port": 4321,
                "path": "/"
              }
            },
            "scmActor": {
              "ed25519Key": "BRTTODO:SCMEd25519PublicKey"
            }
          },
          "document": {
            "id": 2,
            "uri": {
              "uri": "http://localhost:4321/",
              "scheme": "http",
              "hostname": "localhost",
              "port": 4321,
              "path": "/"
            }
          },
          "request": {
            "id": "{29e0c41c-635f-40ba-8caa-4e97b8872bb6}",
            "type": "Login_ApproveOperation"
          }
        },
        "metadata": {
          "version": 3,
          "status": "Pending" as EMetadataStatus,
          "category": "Customer",
          "changePassUrl": ""
        },
        "version": 6,
        "selectedAccountId": "h6rx-8u37-v4yt-htkk",
        "challenges": {
          "id": {
            "id": "id",
            "type": "Identification" as EBerytusChallengeType.Identification,
            "parameters": null,
            "messages": {
              "GetIdentityFields": {
                "name": "GetIdentityFields" as EBerytusIdentificationChallengeMessageName.GetIdentityFields,
                "payload": [
                  "username"
                ]
              }
            },
            "closed": true
          },
          "ds": {
            "id": "ds",
            "type": "DigitalSignature" as EBerytusChallengeType.DigitalSignature,
            "parameters": null,
            "messages": {
              "SelectKey": {
                "name": "SelectKey" as EBerytusDigitalSignatureChallengeMessageName.SelectKey,
                "payload": "key"
              },
              "SignNonce": {
                "name": "SignNonce" as EBerytusDigitalSignatureChallengeMessageName.SignNonce,
                "payload": "tXQxH5FHpjMagGXMM0sedKGnanhxgtcqTZy5qKrHRo4jU2R3sKd8/MJ8ozf4i2l4r3sAEa3lZitsu9scA2dpXQ=="
              }
            },
            "dsState": {
              "privateKey": "-----BEGIN PRIVATE KEY-----\n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\neo/1tGU42HKFSaLunC9rHP5cyfk4vW0D\n-----END PRIVATE KEY-----",
              "signedMessage": "-----BEGIN CUSTOM SIGNATURE FORMAT -----\nVNBTyOJETIfLR/3aLeUI8eRiwlqtmCZzxYwd+yaCkAWWO8cLa52MRNX7m5uPwONY\nKdcFhWuA006ZFYuXRbe16IuL4j9wpAr4b3uc5n8mCgHv5G46Qiva9XQP/oMKVSjQ\nwPGLEq9uDrsGvZwlTCsNBw5Pek9pyHrLODJOmCOlEKt3IaNMYQKf8SoZXjhrLK0d\nHXvYygH/kOXUL81fynF5c3I8bH9mGpNZ1dxQztA4rG0S6mmD21KkF1RcfA0l7zv6\nr3nht9/v0vXTblYhyebD02JMSsYes0VtNpKDbyZcKzf0JP4b6PDn6Lg36np1TWLl\ncqiRRa22PPkWnbilmQOGZESpKupdGDWrXuqiAvxip4v/pMAmfAO6zqHj7WfrulbG\ngDNFULmbwXdhsyUd4MkithtD2zZB7/NZfQEDZQ880U6qzErBYGWA75Sw6kH7wXGz\nahDOn8u5h9B/akz6tSLRGreAbEzgxAmcio2FdB1XsXKvBVgYlarXGUBtYrHlF0r1\nP+ZQwRk1aME873K9PvjqBDUdq8LDm35Y9M5Fu70dnojyUufEDKQJ33fGj4Wqveyb\nbz1GwoB5e8y+TLk0nYDea7nvUKJ+jzpd9HzxEzfNcEqiHCN4lG5bkOkIDkDRKyQ5\nqQRdqRQ3YZ9ndaKSRUViw2w67opWI3WBRWr03tBafe0=\n----END CUSTOM SIGNATURE FORMAT -----",
              "nonce": "tXQxH5FHpjMagGXMM0sedKGnanhxgtcqTZy5qKrHRo4jU2R3sKd8/MJ8ozf4i2l4r3sAEa3lZitsu9scA2dpXQ=="
            },
            "closed": true,
            "abortionReasonCode": "InvalidSignature"
          }
        }
      }
    }
]
//_sessions.save = _sessions.putPrivateKey;

export const Sessions: Array<PhaseEntry> = [];

_sessions.forEach(entry => {
    const sess = entry.session;
    const newId = '[' + randomPassword() + ']';
    const clone: Session = {
        ...sess,
        id: newId,
        operation: {
            ...sess.operation,
            id: newId
        }
    };
    Sessions.push({
        phase: entry.phase,
        session: clone
    });
});

// const approveTransitionRequest = {
//     requestId: "{batata}",
//     requestType: ERequestType.ApproveTransitionToAuthOp
// };
// _sessions.approveTransitionToAuthOp = {
//     id: "{x23bb678-093z-123a-7654-345234asd000}",
//     sessionInfo: {
//         ...Sessions.save.sessionInfo,
//         sessionId: "{x23bb678-093z-123a-7654-345234asd000}",
//     },
//     transitionedFromSessionId: Sessions.save.id,
//     requests: [approveTransitionRequest],
//     context: {
//         request: approveTransitionRequest,
//         tabId: _sessions.save.context.tabId
//     },
//     tabId: Sessions.save.tabId,
//     metadata: Sessions.save.metadata,
//     selectedAccountId: Accounts[0].id,
//     version: 1
// }
// Sessions.approveTransitionToAuthOp = _sessions.approveTransitionToAuthOp;

const phases = Object.keys(_sessions);

export const SimulationUrls = Sessions.map((entry, i) => {
    const { phase, session: sess } = entry;
    let url: string;
    if (phase === 'Login_ApproveOperation') {
        url = `/intent/${sess.id}/0`;
    } else if (phase === 'AccountCreation_AddField') {
        const { id } = sess.createFieldOptions![sess.createFieldOptions!.length - 1]
        url = `/create-field/${sess.id}/${id}`;
    } else if (phase === 'AccountCreation_AddField:Put') {
        const { id } = sess.putFields![sess.putFields!.length - 1]
        url = `/put-field/${sess.id}/${id}`;
    } else if (phase === 'AccountCreation_GetUserAttributes') {
        url = `/get-user-attributes/${sess.id}/1`;
    } else if (phase === 'Login_CloseOperation') {
        url = `/save/:${sess.id}/${Accounts[0].id}` // put a random account id as the newly saved account
    } else if (phase === 'AccountCreation_RejectFieldValue') {
        // use the last one for now. This is not correct
        // since the rejectedFieldId might not be the last one
        // in the array. But we let it pass for the sake of
        // simulation.
        const { fieldId } = sess.rejectedFieldValues![sess.rejectedFieldValues!.length - 1]
        url = `/create-field/${sess.id}/${fieldId}/rejected`;
    } else if (phase === 'AccountAuthentication_ApproveChallengeRequest') {
        const challengeIds = Object.keys(sess.challenges!);
        const challengeId = challengeIds[challengeIds.length - 1];

        url = `auth/${sess.id}/challenge/${challengeId}/approve`;
    } else if (phase === 'AccountAuthentication_RespondToChallengeMessage') {
        const challengeIds = Object.keys(sess.challenges!);
        const challengeId = challengeIds[challengeIds.length - 1];

        const messages = sess.challenges![challengeId].messages;
        const messageIds = Object.keys(messages);
        const messageId = messageIds[messageIds.length - 1];
        url = `auth/${sess.id}/challenge/${challengeId}/message/${messageId}`;
    } else if (phase === 'AccountCreation_ApproveTransitionToAuthOp') {
        url = `transition-to-auth-op/${sess.transitionedFromSessionId}/${sess.id}`;
    } else if (phase === 'AccountAuthentication_CloseChallenge') {
      // Note(berytus); use the last one for now. This is not correct
      // since the rejectedFieldId might not be the last one
      // in the array. But we let it pass..
      const challenges = Object.values(sess.challenges!);
      const lastChallenge = challenges[challenges.length - 1];
      url = `auth/${sess.id}/challenge/${lastChallenge.id}/sealed`;
    } else if (phase === 'AccountAuthentication_AbortChallenge') {
      // Note(berytus); use the last one for now. This is not correct
      // since the rejectedFieldId might not be the last one
      // in the array. But we let it pass..
      const challenges = Object.values(sess.challenges!);
      const lastChallenge = challenges[challenges.length - 1];
      url = `auth/${sess.id}/challenge/${lastChallenge.id}/aborted`;
    } else {
      throw new Error("unknown phase: " + phase);
    }
    return {
        phase,
        sessionId: sess.id,
        url
    };
})

if (process.env.NODE_ENV === "development") {
    // db.accounts.count()
    //     .then(c => {
    //         if (c !== 0) {
    //             return;
    //         }

    //     });
    // db.sessions.count()
    //     .then(c => {
    //         if (c !== 0) {
    //             return;
    //         }

    //     })
    db.accounts.bulkAdd(Accounts);
    db.sessions.bulkAdd(Sessions.map(e => e.session));
}
