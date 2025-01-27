import { Account, Session, db } from "@root/db/db";
import { randomPassword } from "@root/utils";
import { EBerytusChallengeType, EBerytusFieldType, ELoginUserIntent, EMetadataStatus, EOperationStatus, EOperationType } from "@berytus/enums";
import { EBerytusIdentificationChallengeMessageName, EBerytusPasswordChallengeMessageName } from "@berytus/types-extd";

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
        phase: phases[i],
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
