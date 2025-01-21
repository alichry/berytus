import { Account, Session, db } from "@root/db/db";
import { randomPassword } from "@root/utils";
import { EBerytusFieldType, ELoginUserIntent, EMetadataStatus, EOperationStatus, EOperationType, RequestType } from "@berytus/enums";

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

const _sessions: Record<string, Session> = {
    Login_ApproveOnboardingRequest: {
        id: "{f09aa365-957f-451e-8859-3b27640afc97}",
        metadata: {
            category: "",
            changePassUrl: "",
            status: EMetadataStatus.Pending,
            version: 1
        },
        channel: {
            id: "{f09aa365-957f-451e-8859-3b27640afc97}",
            constraints: {
                enableEndToEndEncryption: false
            },
            webAppActor: {
                "originalUri": {
                    "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
                    "scheme": "moz-extension",
                    "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
                    "port": -1,
                    "path": "/build/index.html"
                },
                "currentUri": {
                    "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
                    "scheme": "moz-extension",
                    "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
                    "port": -1,
                    "path": "/build/index.html"
                }
            },
        },
        operation: {
            id: "{f09aa365-957f-451e-8859-3b27640afc97}",
            state: {},
            status: EOperationStatus.Pending,
            type: EOperationType.PendingDeclaration,
            intent: ELoginUserIntent.PendingDeclaration,
            requestedUserAttributes: [],
            fields: {},
            challenges: {}
        },
        "requests": [
            {
                "id": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
                "type": "Login_ApproveOperation"
            }
        ],
        context: {
            "request": {
                "id": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
                "type": "Login_ApproveOperation"
            },
            document: {
                id: 25,
                uri: {
                    "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
                    "scheme": "moz-extension",
                    "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
                    "port": -1,
                    "path": "/build/index.html"
                }
            }
        },
        version: 1
    },
    // getUserAttributes: {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "channelId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967480,
    //             "originalUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionState": 0,
    //         "sessionType": 1
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         {
    //             "requestId": "{e9d68eba-56d0-40b4-96ce-c925c454d248}",
    //             "requestType": "GetUserAttributes" as ERequestType
    //         }
    //     ],
    //     "id": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //     "context": {
    //         "request": {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         "tabId": 41
    //     },
    //     "version": 2,
    //     "requiredUserAttributes": {
    //         "name": false,
    //         "givenName": false,
    //         "familyName": false,
    //         "middleName": false,
    //         "nickname": false,
    //         "profile": false,
    //         "picture": false,
    //         "website": false,
    //         "gender": false,
    //         "birthdate": false,
    //         "zoneinfo": false,
    //         "locale": false,

    //         "address": false
    //     }
    // },
    // getUserAttributesAllRequired: {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "channelId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967480,
    //             "originalUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionState": 0,
    //         "sessionType": 1
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         {
    //             "requestId": "{e9d68eba-56d0-40b4-96ce-c925c454d248}",
    //             "requestType": "GetUserAttributes" as ERequestType
    //         }
    //     ],
    //     "id": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //     "context": {
    //         "request": {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         "tabId": 41
    //     },
    //     "version": 2,
    //     "requiredUserAttributes": {
    //         "name": true,
    //         "givenName": true,
    //         "familyName": true,
    //         "middleName": true,
    //         "nickname": true,
    //         "profile": true,
    //         "picture": true,
    //         "website": true,
    //         "gender": true,
    //         "birthdate": true,
    //         "zoneinfo": true,
    //         "locale": true,
    //         "address": true
    //     }
    // },
    // createEmail: {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "channelId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967480,
    //             "originalUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionState": 0,
    //         "sessionType": 1
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         {
    //             "requestId": "{e9d68eba-56d0-40b4-96ce-c925c454d248}",
    //             "requestType": "GetUserAttributes" as ERequestType
    //         },
    //         {
    //             "requestId": "{16d63d45-30f8-4127-b6a4-96ab734c46df}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         }
    //     ],
    //     "id": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //     "context": {
    //         "request": {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         "tabId": 41
    //     },
    //     "version": 3,
    //     "requiredUserAttributes": {
    //         "name": false,
    //         "givenName": false,
    //         "familyName": false,
    //         "middleName": false,
    //         "nickname": false,
    //         "profile": false,
    //         "picture": false,
    //         "website": false,
    //         "gender": false,
    //         "birthdate": false,
    //         "zoneinfo": false,
    //         "locale": false,

    //         "address": false
    //     },
    //     "userAttributes": {
    //         "address": "My college",
    //         "birthdate": "1/1/1970",
    //         "familyName": "Doe",
    //         "gender": "Male",
    //         "givenName": "",
    //         "locale": "",
    //         "middleName": "Stewart",
    //         "name": "Bob",
    //         "nickname": "Bobby",

    //         "picture": "",
    //         "profile": "",
    //         "website": "",
    //         "zoneinfo": ""
    //     },
    //     "createFieldOptions": [
    //         {
    //             "fieldId": "email",
    //             "description": "Email field is needed",
    //             "foreignIdentityKind": "EmailAddress",
    //             "fieldType": 1
    //         }
    //     ]
    // },
    // createUsername: {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "channelId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967480,
    //             "originalUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionState": 0,
    //         "sessionType": 1
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         {
    //             "requestId": "{e9d68eba-56d0-40b4-96ce-c925c454d248}",
    //             "requestType": "GetUserAttributes" as ERequestType
    //         },
    //         {
    //             "requestId": "{16d63d45-30f8-4127-b6a4-96ab734c46df}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{cadd1400-b079-4924-a61c-382d083d1494}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         }
    //     ],
    //     "id": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //     "context": {
    //         "request": {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         "tabId": 41
    //     },
    //     "version": 4,
    //     "requiredUserAttributes": {
    //         "name": false,
    //         "givenName": false,
    //         "familyName": false,
    //         "middleName": false,
    //         "nickname": false,
    //         "profile": false,
    //         "picture": false,
    //         "website": false,
    //         "gender": false,
    //         "birthdate": false,
    //         "zoneinfo": false,
    //         "locale": false,

    //         "address": false
    //     },
    //     "userAttributes": {
    //         "address": "My college",
    //         "birthdate": "1/1/1970",
    //         "familyName": "Doe",
    //         "gender": "Male",
    //         "givenName": "",
    //         "locale": "",
    //         "middleName": "Stewart",
    //         "name": "Bob",
    //         "nickname": "Bobby",

    //         "picture": "",
    //         "profile": "",
    //         "website": "",
    //         "zoneinfo": ""
    //     },
    //     "createFieldOptions": [
    //         {
    //             "fieldId": "email",
    //             "description": "Email field is needed",
    //             "foreignIdentityKind": "EmailAddress",
    //             "fieldType": 1
    //         },
    //         {
    //             "fieldId": "username",
    //             "description": "",
    //             "humanReadable": false,
    //             "private": false,
    //             "allowedCharacters": "",
    //             "maxLength": 2,
    //             "fieldType": 0
    //         }
    //     ],
    //     "fields": [
    //         {
    //             "options": {
    //                 "fieldId": "email",
    //                 "description": "Email field is needed",
    //                 "foreignIdentityKind": "EmailAddress",
    //                 "fieldType": 1
    //             },
    //             "value": "m"
    //         }
    //     ]
    // },
    // createPassword: {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "channelId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967480,
    //             "originalUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionState": 0,
    //         "sessionType": 1
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         {
    //             "requestId": "{e9d68eba-56d0-40b4-96ce-c925c454d248}",
    //             "requestType": "GetUserAttributes" as ERequestType
    //         },
    //         {
    //             "requestId": "{16d63d45-30f8-4127-b6a4-96ab734c46df}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{cadd1400-b079-4924-a61c-382d083d1494}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{d0505955-d206-45b0-b6f3-98357aee855d}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         }
    //     ],
    //     "id": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //     "context": {
    //         "request": {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         "tabId": 41
    //     },
    //     "version": 5,
    //     "requiredUserAttributes": {
    //         "name": false,
    //         "givenName": false,
    //         "familyName": false,
    //         "middleName": false,
    //         "nickname": false,
    //         "profile": false,
    //         "picture": false,
    //         "website": false,
    //         "gender": false,
    //         "birthdate": false,
    //         "zoneinfo": false,
    //         "locale": false,

    //         "address": false
    //     },
    //     "userAttributes": {
    //         "address": "My college",
    //         "birthdate": "1/1/1970",
    //         "familyName": "Doe",
    //         "gender": "Male",
    //         "givenName": "",
    //         "locale": "",
    //         "middleName": "Stewart",
    //         "name": "Bob",
    //         "nickname": "Bobby",

    //         "picture": "",
    //         "profile": "",
    //         "website": "",
    //         "zoneinfo": ""
    //     },
    //     "createFieldOptions": [
    //         {
    //             "fieldId": "email",
    //             "description": "Email field is needed",
    //             "foreignIdentityKind": "EmailAddress",
    //             "fieldType": 1
    //         },
    //         {
    //             "fieldId": "username",
    //             "description": "",
    //             "humanReadable": false,
    //             "private": false,
    //             "allowedCharacters": "",
    //             "maxLength": 2,
    //             "fieldType": 0
    //         },
    //         {
    //             "fieldId": "password",
    //             "description": "Password field is needed",
    //             "fieldType": 2
    //         }
    //     ],
    //     "fields": [
    //         {
    //             "options": {
    //                 "fieldId": "email",
    //                 "description": "Email field is needed",
    //                 "foreignIdentityKind": "EmailAddress",
    //                 "fieldType": 1
    //             },
    //             "value": "m"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "username",
    //                 "description": "",
    //                 "humanReadable": false,
    //                 "private": false,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "Axolotl"
    //         }
    //     ]
    // },
    // putInternalId: {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "channelId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967480,
    //             "originalUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionState": 0,
    //         "sessionType": 1
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         {
    //             "requestId": "{e9d68eba-56d0-40b4-96ce-c925c454d248}",
    //             "requestType": "GetUserAttributes" as ERequestType
    //         },
    //         {
    //             "requestId": "{16d63d45-30f8-4127-b6a4-96ab734c46df}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{cadd1400-b079-4924-a61c-382d083d1494}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{d0505955-d206-45b0-b6f3-98357aee855d}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{573c3130-5b0f-4f5f-a473-58867d2786d1}",
    //             "requestType": "PutRegistrationField" as ERequestType
    //         }
    //     ],
    //     "id": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //     "context": {
    //         "request": {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         "tabId": 41
    //     },
    //     "version": 6,
    //     "requiredUserAttributes": {
    //         "name": false,
    //         "givenName": false,
    //         "familyName": false,
    //         "middleName": false,
    //         "nickname": false,
    //         "profile": false,
    //         "picture": false,
    //         "website": false,
    //         "gender": false,
    //         "birthdate": false,
    //         "zoneinfo": false,
    //         "locale": false,

    //         "address": false
    //     },
    //     "userAttributes": {
    //         "address": "My college",
    //         "birthdate": "1/1/1970",
    //         "familyName": "Doe",
    //         "gender": "Male",
    //         "givenName": "",
    //         "locale": "",
    //         "middleName": "Stewart",
    //         "name": "Bob",
    //         "nickname": "Bobby",

    //         "picture": "",
    //         "profile": "",
    //         "website": "",
    //         "zoneinfo": ""
    //     },
    //     "createFieldOptions": [
    //         {
    //             "fieldId": "email",
    //             "description": "Email field is needed",
    //             "foreignIdentityKind": "EmailAddress",
    //             "fieldType": 1
    //         },
    //         {
    //             "fieldId": "username",
    //             "description": "",
    //             "humanReadable": false,
    //             "private": false,
    //             "allowedCharacters": "",
    //             "maxLength": 2,
    //             "fieldType": 0
    //         },
    //         {
    //             "fieldId": "password",
    //             "description": "Password field is needed",
    //             "fieldType": 2
    //         }
    //     ],
    //     "fields": [
    //         {
    //             "options": {
    //                 "fieldId": "email",
    //                 "description": "Email field is needed",
    //                 "foreignIdentityKind": "EmailAddress",
    //                 "fieldType": 1
    //             },
    //             "value": "m"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "username",
    //                 "description": "",
    //                 "humanReadable": false,
    //                 "private": false,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "Axolotl"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "password",
    //                 "description": "Password field is needed",
    //                 "fieldType": 2
    //             },
    //             "value": "l90y-swcf-1vfl-nkl4"
    //         }
    //     ],
    //     "putFields": [
    //         {
    //             "options": {
    //                 "fieldId": "accountId",
    //                 "description": "internal account id",
    //                 "humanReadable": false,
    //                 "private": true,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "542466-88752"
    //         }
    //     ]
    // },
    // createSecurePassword: {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "channelId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967480,
    //             "originalUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionState": 0,
    //         "sessionType": 1
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         {
    //             "requestId": "{e9d68eba-56d0-40b4-96ce-c925c454d248}",
    //             "requestType": "GetUserAttributes" as ERequestType
    //         },
    //         {
    //             "requestId": "{16d63d45-30f8-4127-b6a4-96ab734c46df}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{cadd1400-b079-4924-a61c-382d083d1494}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{d0505955-d206-45b0-b6f3-98357aee855d}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{573c3130-5b0f-4f5f-a473-58867d2786d1}",
    //             "requestType": "PutRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{fc13e5b1-ca65-49c7-966e-b49cc384db0c}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         }
    //     ],
    //     "id": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //     "context": {
    //         "request": {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         "tabId": 41
    //     },
    //     "version": 7,
    //     "requiredUserAttributes": {
    //         "name": false,
    //         "givenName": false,
    //         "familyName": false,
    //         "middleName": false,
    //         "nickname": false,
    //         "profile": false,
    //         "picture": false,
    //         "website": false,
    //         "gender": false,
    //         "birthdate": false,
    //         "zoneinfo": false,
    //         "locale": false,

    //         "address": false
    //     },
    //     "userAttributes": {
    //         "address": "My college",
    //         "birthdate": "1/1/1970",
    //         "familyName": "Doe",
    //         "gender": "Male",
    //         "givenName": "",
    //         "locale": "",
    //         "middleName": "Stewart",
    //         "name": "Bob",
    //         "nickname": "Bobby",

    //         "picture": "",
    //         "profile": "",
    //         "website": "",
    //         "zoneinfo": ""
    //     },
    //     "createFieldOptions": [
    //         {
    //             "fieldId": "email",
    //             "description": "Email field is needed",
    //             "foreignIdentityKind": "EmailAddress",
    //             "fieldType": 1
    //         },
    //         {
    //             "fieldId": "username",
    //             "description": "",
    //             "humanReadable": false,
    //             "private": false,
    //             "allowedCharacters": "",
    //             "maxLength": 2,
    //             "fieldType": 0
    //         },
    //         {
    //             "fieldId": "password",
    //             "description": "Password field is needed",
    //             "fieldType": 2
    //         },
    //         {
    //             "fieldId": "securePassword",
    //             "description": "Secure Password :)",
    //             "identityFieldId": "email",
    //             "fieldType": 3
    //         }
    //     ],
    //     "fields": [
    //         {
    //             "options": {
    //                 "fieldId": "email",
    //                 "description": "Email field is needed",
    //                 "foreignIdentityKind": "EmailAddress",
    //                 "fieldType": 1
    //             },
    //             "value": "m"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "username",
    //                 "description": "",
    //                 "humanReadable": false,
    //                 "private": false,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "Axolotl"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "password",
    //                 "description": "Password field is needed",
    //                 "fieldType": 2
    //             },
    //             "value": "l90y-swcf-1vfl-nkl4"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "accountId",
    //                 "description": "internal account id",
    //                 "humanReadable": false,
    //                 "private": true,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "542466-88752"
    //         }
    //     ],
    //     "putFields": [
    //         {
    //             "options": {
    //                 "fieldId": "accountId",
    //                 "description": "internal account id",
    //                 "humanReadable": false,
    //                 "private": true,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "542466-88752"
    //         }
    //     ]
    // },
    // createPublicKey: {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "channelId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967480,
    //             "originalUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionState": 0,
    //         "sessionType": 1
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         {
    //             "requestId": "{e9d68eba-56d0-40b4-96ce-c925c454d248}",
    //             "requestType": "GetUserAttributes" as ERequestType
    //         },
    //         {
    //             "requestId": "{16d63d45-30f8-4127-b6a4-96ab734c46df}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{cadd1400-b079-4924-a61c-382d083d1494}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{d0505955-d206-45b0-b6f3-98357aee855d}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{573c3130-5b0f-4f5f-a473-58867d2786d1}",
    //             "requestType": "PutRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{fc13e5b1-ca65-49c7-966e-b49cc384db0c}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{9484b6ef-9433-4d06-82f7-08890b58a496}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         }
    //     ],
    //     "id": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //     "context": {
    //         "request": {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         "tabId": 41
    //     },
    //     "version": 8,
    //     "requiredUserAttributes": {
    //         "name": false,
    //         "givenName": false,
    //         "familyName": false,
    //         "middleName": false,
    //         "nickname": false,
    //         "profile": false,
    //         "picture": false,
    //         "website": false,
    //         "gender": false,
    //         "birthdate": false,
    //         "zoneinfo": false,
    //         "locale": false,

    //         "address": false
    //     },
    //     "userAttributes": {
    //         "address": "My college",
    //         "birthdate": "1/1/1970",
    //         "familyName": "Doe",
    //         "gender": "Male",
    //         "givenName": "",
    //         "locale": "",
    //         "middleName": "Stewart",
    //         "name": "Bob",
    //         "nickname": "Bobby",

    //         "picture": "",
    //         "profile": "",
    //         "website": "",
    //         "zoneinfo": ""
    //     },
    //     "createFieldOptions": [
    //         {
    //             "fieldId": "email",
    //             "description": "Email field is needed",
    //             "foreignIdentityKind": "EmailAddress",
    //             "fieldType": 1
    //         },
    //         {
    //             "fieldId": "username",
    //             "description": "",
    //             "humanReadable": false,
    //             "private": false,
    //             "allowedCharacters": "",
    //             "maxLength": 2,
    //             "fieldType": 0
    //         },
    //         {
    //             "fieldId": "password",
    //             "description": "Password field is needed",
    //             "fieldType": 2
    //         },
    //         {
    //             "fieldId": "securePassword",
    //             "description": "Secure Password :)",
    //             "identityFieldId": "email",
    //             "fieldType": 3
    //         },
    //         {
    //             "fieldId": "clientKey",
    //             "description": "Client Key",
    //             "alg": -42,
    //             "fieldType": 4
    //         }
    //     ],
    //     "fields": [
    //         {
    //             "options": {
    //                 "fieldId": "email",
    //                 "description": "Email field is needed",
    //                 "foreignIdentityKind": "EmailAddress",
    //                 "fieldType": 1
    //             },
    //             "value": "m"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "username",
    //                 "description": "",
    //                 "humanReadable": false,
    //                 "private": false,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "Axolotl"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "password",
    //                 "description": "Password field is needed",
    //                 "fieldType": 2
    //             },
    //             "value": "l90y-swcf-1vfl-nkl4"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "accountId",
    //                 "description": "internal account id",
    //                 "humanReadable": false,
    //                 "private": true,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "542466-88752"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "securePassword",
    //                 "description": "Secure Password :)",
    //                 "identityFieldId": "email",
    //                 "fieldType": 3
    //             },
    //             "value": "ohtv-5bh3-0ha4-10hu"
    //         }
    //     ],
    //     "putFields": [
    //         {
    //             "options": {
    //                 "fieldId": "accountId",
    //                 "description": "internal account id",
    //                 "humanReadable": false,
    //                 "private": true,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "542466-88752"
    //         }
    //     ]
    // },
    // putPrivateKey: {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "channelId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967480,
    //             "originalUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionState": 0,
    //         "sessionType": 1
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         {
    //             "requestId": "{e9d68eba-56d0-40b4-96ce-c925c454d248}",
    //             "requestType": "GetUserAttributes" as ERequestType
    //         },
    //         {
    //             "requestId": "{16d63d45-30f8-4127-b6a4-96ab734c46df}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{cadd1400-b079-4924-a61c-382d083d1494}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{d0505955-d206-45b0-b6f3-98357aee855d}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{573c3130-5b0f-4f5f-a473-58867d2786d1}",
    //             "requestType": "PutRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{fc13e5b1-ca65-49c7-966e-b49cc384db0c}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{9484b6ef-9433-4d06-82f7-08890b58a496}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{1750f3b9-290e-4dc0-b8ab-449cc5801540}",
    //             "requestType": "PutRegistrationField" as ERequestType
    //         }
    //     ],
    //     "id": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //     "context": {
    //         "request": {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         "tabId": 41
    //     },
    //     "version": 9,
    //     "requiredUserAttributes": {
    //         "name": false,
    //         "givenName": false,
    //         "familyName": false,
    //         "middleName": false,
    //         "nickname": false,
    //         "profile": false,
    //         "picture": false,
    //         "website": false,
    //         "gender": false,
    //         "birthdate": false,
    //         "zoneinfo": false,
    //         "locale": false,

    //         "address": false
    //     },
    //     "userAttributes": {
    //         "address": "My college",
    //         "birthdate": "1/1/1970",
    //         "familyName": "Doe",
    //         "gender": "Male",
    //         "givenName": "",
    //         "locale": "",
    //         "middleName": "Stewart",
    //         "name": "Bob",
    //         "nickname": "Bobby",

    //         "picture": "",
    //         "profile": "",
    //         "website": "",
    //         "zoneinfo": ""
    //     },
    //     "createFieldOptions": [
    //         {
    //             "fieldId": "email",
    //             "description": "Email field is needed",
    //             "foreignIdentityKind": "EmailAddress",
    //             "fieldType": 1
    //         },
    //         {
    //             "fieldId": "username",
    //             "description": "",
    //             "humanReadable": false,
    //             "private": false,
    //             "allowedCharacters": "",
    //             "maxLength": 2,
    //             "fieldType": 0
    //         },
    //         {
    //             "fieldId": "password",
    //             "description": "Password field is needed",
    //             "fieldType": 2
    //         },
    //         {
    //             "fieldId": "securePassword",
    //             "description": "Secure Password :)",
    //             "identityFieldId": "email",
    //             "fieldType": 3
    //         },
    //         {
    //             "fieldId": "clientKey",
    //             "description": "Client Key",
    //             "alg": -42,
    //             "fieldType": 4
    //         }
    //     ],
    //     "fields": [
    //         {
    //             "options": {
    //                 "fieldId": "email",
    //                 "description": "Email field is needed",
    //                 "foreignIdentityKind": "EmailAddress",
    //                 "fieldType": 1
    //             },
    //             "value": "m"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "username",
    //                 "description": "",
    //                 "humanReadable": false,
    //                 "private": false,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "Axolotl"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "password",
    //                 "description": "Password field is needed",
    //                 "fieldType": 2
    //             },
    //             "value": "l90y-swcf-1vfl-nkl4"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "accountId",
    //                 "description": "internal account id",
    //                 "humanReadable": false,
    //                 "private": true,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "542466-88752"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "securePassword",
    //                 "description": "Secure Password :)",
    //                 "identityFieldId": "email",
    //                 "fieldType": 3
    //             },
    //             "value": "ohtv-5bh3-0ha4-10hu"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "clientKey",
    //                 "description": "Client Key",
    //                 "alg": -42,
    //                 "fieldType": 4,
    //                 "private": true
    //             },
    //             "value": "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAp4TnY8a+sgvPhChqLvibViVHKfcKbXieLS6C1uyTnlvtfJ1Yv7McLUcxPQstupio5UE0mVdYwZlC1nvbYTMF9aLEYZoVWiSqYM5BwJowZS+zhvARZH09fps36e+CtQPk5OWdJdp2mgwnGnf8OGBtB/4O60WD8ST+4nArSdST93BX1D3Zc0qWytJSbVGVi0NCTFIC4earUEAZcf8XZuio+ee74bwolV3OOve/bcuNblUIn0D9F5dI4z9Od0QFbneCmzthpbXy1ueQxhmAnxXhkqHL+x9DHQNIGd/PV7+7cKWFSWmNVKDveeBgAGPywGeJ9d+Dg7OQPl1UEob7/Lr7uMm1VOTFTp6HEwu8KCi4dTKujDKaxpGkyeV5xS1cfBzw1l5hWkYC/pCzqi5ggA+NYrlaCpCUYYolQlRKHqmOROBrBy4RgUin5YvLK97S+J0SajTRvWR+miSrSQLM/azh/7gRxFuhfJtJFWGij3WbLvNcQDRHL4ExDSc+6cIQdiJJS8CBw+WCKtr2ZNcjUQiLQWtQfDw0POogYxl8PkEH5GQW0N9QqoUWyWHqaIdMlzqG8CggC7KlZl7Ka0wp+jUtl8+JtoXKXYl5H07aC0Y9xtb3va8JeoX0G1FdZyN8Wzx7useS9ZPvgXmv4Ft1k4CxGWQrdkYspubskZqd2j9Y1dsCAwEAAQ=="
    //         }
    //     ],
    //     "putFields": [
    //         {
    //             "options": {
    //                 "fieldId": "accountId",
    //                 "description": "internal account id",
    //                 "humanReadable": false,
    //                 "private": true,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "542466-88752"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "webAppGeneratedKey",
    //                 "description": "WebApp-Generated Key",
    //                 "alg": -42,
    //                 "fieldType": 4
    //             },
    //             "value": "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"
    //         }
    //     ]
    // },
    // "rejectUsername": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "channelId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967480,
    //             "originalUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionState": 0,
    //         "sessionType": 1
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         {
    //             "requestId": "{e9d68eba-56d0-40b4-96ce-c925c454d248}",
    //             "requestType": "GetUserAttributes" as ERequestType
    //         },
    //         {
    //             "requestId": "{16d63d45-30f8-4127-b6a4-96ab734c46df}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{cadd1400-b079-4924-a61c-382d083d1494}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{d0505955-d206-45b0-b6f3-98357aee855d}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{573c3130-5b0f-4f5f-a473-58867d2786d1}",
    //             "requestType": "PutRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{fc13e5b1-ca65-49c7-966e-b49cc384db0c}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{9484b6ef-9433-4d06-82f7-08890b58a496}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{1750f3b9-290e-4dc0-b8ab-449cc5801540}",
    //             "requestType": "PutRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{375603f0-c5ff-4728-8615-28e651c2af1d}",
    //             "requestType": "RejectRegistrationFieldValue" as ERequestType
    //         }
    //     ],
    //     "id": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //     "context": {
    //         "request": {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         "tabId": 41
    //     },
    //     "version": 10,
    //     "requiredUserAttributes": {
    //         "name": false,
    //         "givenName": false,
    //         "familyName": false,
    //         "middleName": false,
    //         "nickname": false,
    //         "profile": false,
    //         "picture": false,
    //         "website": false,
    //         "gender": false,
    //         "birthdate": false,
    //         "zoneinfo": false,
    //         "locale": false,

    //         "address": false
    //     },
    //     "userAttributes": {
    //         "address": "My college",
    //         "birthdate": "1/1/1970",
    //         "familyName": "Doe",
    //         "gender": "Male",
    //         "givenName": "",
    //         "locale": "",
    //         "middleName": "Stewart",
    //         "name": "Bob",
    //         "nickname": "Bobby",

    //         "picture": "",
    //         "profile": "",
    //         "website": "",
    //         "zoneinfo": ""
    //     },
    //     "createFieldOptions": [
    //         {
    //             "fieldId": "email",
    //             "description": "Email field is needed",
    //             "foreignIdentityKind": "EmailAddress",
    //             "fieldType": 1
    //         },
    //         {
    //             "fieldId": "username",
    //             "description": "",
    //             "humanReadable": false,
    //             "private": false,
    //             "allowedCharacters": "",
    //             "maxLength": 2,
    //             "fieldType": 0
    //         },
    //         {
    //             "fieldId": "password",
    //             "description": "Password field is needed",
    //             "fieldType": 2
    //         },
    //         {
    //             "fieldId": "securePassword",
    //             "description": "Secure Password :)",
    //             "identityFieldId": "email",
    //             "fieldType": 3
    //         },
    //         {
    //             "fieldId": "clientKey",
    //             "description": "Client Key",
    //             "alg": -42,
    //             "fieldType": 4
    //         }
    //     ],
    //     "fields": [
    //         {
    //             "options": {
    //                 "fieldId": "email",
    //                 "description": "Email field is needed",
    //                 "foreignIdentityKind": "EmailAddress",
    //                 "fieldType": 1
    //             },
    //             "value": "m"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "username",
    //                 "description": "",
    //                 "humanReadable": false,
    //                 "private": false,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "Axolotl"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "password",
    //                 "description": "Password field is needed",
    //                 "fieldType": 2
    //             },
    //             "value": "l90y-swcf-1vfl-nkl4"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "accountId",
    //                 "description": "internal account id",
    //                 "humanReadable": false,
    //                 "private": true,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "542466-88752"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "securePassword",
    //                 "description": "Secure Password :)",
    //                 "identityFieldId": "email",
    //                 "fieldType": 3
    //             },
    //             "value": "ohtv-5bh3-0ha4-10hu"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "clientKey",
    //                 "description": "Client Key",
    //                 "alg": -42,
    //                 "fieldType": 4,
    //                 "private": true
    //             },
    //             "value": "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAp4TnY8a+sgvPhChqLvibViVHKfcKbXieLS6C1uyTnlvtfJ1Yv7McLUcxPQstupio5UE0mVdYwZlC1nvbYTMF9aLEYZoVWiSqYM5BwJowZS+zhvARZH09fps36e+CtQPk5OWdJdp2mgwnGnf8OGBtB/4O60WD8ST+4nArSdST93BX1D3Zc0qWytJSbVGVi0NCTFIC4earUEAZcf8XZuio+ee74bwolV3OOve/bcuNblUIn0D9F5dI4z9Od0QFbneCmzthpbXy1ueQxhmAnxXhkqHL+x9DHQNIGd/PV7+7cKWFSWmNVKDveeBgAGPywGeJ9d+Dg7OQPl1UEob7/Lr7uMm1VOTFTp6HEwu8KCi4dTKujDKaxpGkyeV5xS1cfBzw1l5hWkYC/pCzqi5ggA+NYrlaCpCUYYolQlRKHqmOROBrBy4RgUin5YvLK97S+J0SajTRvWR+miSrSQLM/azh/7gRxFuhfJtJFWGij3WbLvNcQDRHL4ExDSc+6cIQdiJJS8CBw+WCKtr2ZNcjUQiLQWtQfDw0POogYxl8PkEH5GQW0N9QqoUWyWHqaIdMlzqG8CggC7KlZl7Ka0wp+jUtl8+JtoXKXYl5H07aC0Y9xtb3va8JeoX0G1FdZyN8Wzx7useS9ZPvgXmv4Ft1k4CxGWQrdkYspubskZqd2j9Y1dsCAwEAAQ=="
    //         }
    //     ],
    //     "putFields": [
    //         {
    //             "options": {
    //                 "fieldId": "accountId",
    //                 "description": "internal account id",
    //                 "humanReadable": false,
    //                 "private": true,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "542466-88752"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "webAppGeneratedKey",
    //                 "description": "WebApp-Generated Key",
    //                 "alg": -42,
    //                 "fieldType": 4
    //             },
    //             "value": "MIIJQgIBADANBgkqhkiG9w0BAQEFAASCCSwwggkoAgEAAoICAQDVAcjPrmO/hwG9Va0jQJFsDI+db12bNBrlDkZWodQhdq6Je2N4ugL1CNorAflbIqzWYrjEB5nmH4lOUYtW6pxUdjZaX2K4XTDyLjL979jfq0TYIEtcaiDWDp0m0NaazJ4euWNZC4g1dVT7NNiZqR7yuIud6bUU0Cq5RrzvKJnfkIXotGZY6QTA18bfHwYiWRwvvNQP51m+VcZFJZeb1a7774sSUycYw4X1E4I7lp2+mkSv+HMH1P1T3FWFcfFxHS31T6Ri6yqJXFB1KtGC79Fs8R2lYfK/TwPQKue0AXyZuGUZvUEWqHt6RCNHsMSJNIRpXVcTns2H4XASyQL9+pKwUgLGXHgoxSnN7JnkFbqitjvZaoABfUXwF8T/7fPQ4mmFFEdvjTAhhYdBghMUP+ObEH4Uq6yPcTO1UyiaTSbFc15IPfEajW2xEi3CJcDvaHQ/wxkuPFqr5w+ABUNa1daEweGt9bh6mHQciJfP5xl2rWYjzVldYprrhv8CtmOgw9FtcY8dw3bV1Vy2RiC9nrAc34EOemxkK6VucBac067hS/T7jeFn67KRJGWBS7HnaPS6dUOhItWd5/QKOBNxvH9smHaVFKLtgl2JZLWI6KY40TzZJBfO6p72eeFsfPP30iSF5yw5KKKEl4ywepRNFdsY9TFYKT5V7LpNj8Ytscx1aQIDAQABAoICAADfhe2QAoLOsIJ+PzM6JnVPkd7aIBW/JADL767gOjsFvinXrf80VbUybZeoQOoPXWYmt5a+sYRy5jdm5gbSihbL+PfHcHXMlJDat8TvbEHN4v6SYHXZTc5wiVMXMaICYf/U9NrMOxbrS2v0egA/0d3e++5ikO+HjFi2y02dGFmFLzYx6fiheQnfU/1LJp6ktd4VFbbg+YpFseGU0svFzmR2aft0Ual+ifiItXArhYcDarsXW9ejcqfRtC6cyCwVdOTXvxb4//NHw/M8t8RtIs6Pli1PKLDKfqBK5LtAqgWXASA7YnC0Q/S9IlXjZUCPlgzLFvE3SjcbBNKpLP4JGddA1/INAB0iENvPnXjrlLdEncZMO/cPIcXSAmRI9gG1qvpcspS1/0fdR+VamudPYUKXapSM0MPtxn3BiyW2eWr5xnaXo642kRjQVZeDNyqK8En3csbLMkmdPRrW73sd9kXXfgqszGPXi7LEt1kn/nWxE3OMnO8+E+lg5Kl/CxHynC2X+1u+QIrjAor+8WL8VBKWX1t/r7EalyTBynxkLB8kmzHYgj/QcKDnhLlNXXN2wcbUkDiuPpo2uEJHOCzNp4LFw8bXQHvpz7FobJlUmTG9ttqmjwQsJ1+ijYa+ejgbTYjWL/X3w3GYE3rKxyBmyoEfGjwarTv0l7+xrOrAB2unAoIBAQDwLDw09dwhefsXjsowBKR1jmNM753qCGF7HmNAkV1DdY/WlbW3lIbbJ1iPP0uZBMnrWxXQxhNkUIUt9MI/JDWttOY6+pIq7O2mQK4iHuSTMMWn1cRNXFFRkvk5Mds8y7W6RGn602uKObPyVXUmZBHPqQxNu095jMoak29iEOqrOhs91nYAPoWIGsZnFdIaQif8b9O0wyBR+LW37VKb5HDqfVe5I6rys9fYLstMdFgAU9eM81To+f3GYvYJpSunyB1vuTATGfhlgh/vqt+7JxZlm7VkmO01PTJ6pALV62IPul4ASLDmQYo5Wxd69+euuxUKO2BHSauOT+iL54NMyVJvAoIBAQDjC0GUDh4uwW8jmE52XBWflyYyRzWscwj/9nJGmQPTHiGFfb3pfcleb6QhwJoHL7U0oOIpfUrd/1aAgSDFQgw4ARx8RUppjwj7xgR5bDo+c431zIrV733RUBKAEomXCX+MDDEBb3D/SHiqBghWe0rDMTsasICGAci2x135L6paXC5QfcRhrhLJi/0Fi1zHsTgNqWn4DajddHB+ng4rDMatPpPYdl5QuElkQV04lXYpb+5b6QMEszvEACqHKqlZXljnAhwUN1LmXsbNLo/OCwdquNHPd+w7olu4bsApfCaVw6rJm9iRoAH4+dcaPAvnuahm6WIVXToE2Ve/BCxizsGnAoIBADZSWv/DiBmORGuMoVCBDE2GehFBtR7k7F9zfRFyj3ydj34CeIpgLqxM1uv3bfrbKWxxOprw4hvzVlwRmwm6/+UdIGHAoOTSl+HdiUUwZZShGGOgpvi0Y7QmoPX9v8+dZ6oMpUfVH+KwsHGkQI4BXkHww1alghFE9cVTCYNNvjYiO10eL6xYd81T+QIB3naP9glTpbeh1qKTtU43gFLANYXj+b5nQdgJM02zL8pIGktgq0U3GEJloK+JXIrL7/bP/VByu1vmqNcUunJz3IyxB5DNyhZJvIUpJUy14epOhex2w6IeztwVgNT+NG5cues8Z0LcMSeOkx3qFvfl0j3A30MCggEACrHKCf5MP6mO4Z9X+s1Fm3nUK8Qc7B2TC/OA2IB2pKgjrhUJMAljxdX+ZjSbTMFbiLoXTRsAvso7OdxbPc9poe2zeMsOwUAijTF7nniRmcV5p11WINOiGozqAixdXMbuwhsE5+tm6C9yt5eHJwOxttYH0a8ErstAuBNWAtnS7WSwLcxuxqcbbXDxnyQ8X4+ic0pKmouSK7RDZENvqWl1pk1vS1/v+19i3Z0Slgq8QSFrmdq08LC8nPPhDPLefT4rc94ukmk0t3M5bArIbU0psUY5Y8tjTj/MkDhvRsT5UE6iKdL0dY+S6DZkgnDXorelDgvqOdVlWTsfnfjFWnYguwKCAQEA7QgPdjg+C3vSrxfSAGjfnfdbd2Jy+L3F6oddvD/0JBndHTjrI63V+iKu0Wz8VZ8YlpY9+RkbFGYZZyz9gWBv3XEwro1e6awh+NPoV0N+kVMfp1vx050t8sgOsf50KRuLL9+n60JaSdGHmfLjc2wqZuq0rGvJ3KQ//XYWn/fZFqf7W5dUf+pMMt5I2HitdwuTUNZmUFUf5NB/qsqZwfb1DFHGe1uLjDKy1TC+oEXnoHTjjv2NPK/cWU/yDWvM9dqoS+FS6I4qQGCLMfVvzLsuN8yShp3p7P1E7K/RZARHEZ9OUvE57OBFGRLjN2OnaehitcoKoMv+nthl7e2ftaJLaA=="
    //         }
    //     ],
    //     "rejectedFieldValues": [
    //         {
    //             "fieldId": "username",
    //             "fieldType": 0,
    //             "code": "Other:Dummy",
    //             "description": "The value you have entered is invalid",
    //             "previousValues": [
    //                 "Axolotl"
    //             ]
    //         }
    //     ]
    // },
    // "rejectUsernameDictated": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "channelId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967480,
    //             "originalUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionState": 0,
    //         "sessionType": 1
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         {
    //             "requestId": "{e9d68eba-56d0-40b4-96ce-c925c454d248}",
    //             "requestType": "GetUserAttributes" as ERequestType
    //         },
    //         {
    //             "requestId": "{16d63d45-30f8-4127-b6a4-96ab734c46df}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{cadd1400-b079-4924-a61c-382d083d1494}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{d0505955-d206-45b0-b6f3-98357aee855d}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{573c3130-5b0f-4f5f-a473-58867d2786d1}",
    //             "requestType": "PutRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{fc13e5b1-ca65-49c7-966e-b49cc384db0c}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{9484b6ef-9433-4d06-82f7-08890b58a496}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{1750f3b9-290e-4dc0-b8ab-449cc5801540}",
    //             "requestType": "PutRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{375603f0-c5ff-4728-8615-28e651c2af1d}",
    //             "requestType": "RejectRegistrationFieldValue" as ERequestType
    //         },
    //         {
    //             "requestId": "{9179f40f-fca1-4e79-acaf-efb57b4d49c6}",
    //             "requestType": "RejectRegistrationFieldValue" as ERequestType
    //         }
    //     ],
    //     "id": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //     "context": {
    //         "request": {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         "tabId": 41
    //     },
    //     "version": 10,
    //     "requiredUserAttributes": {
    //         "name": false,
    //         "givenName": false,
    //         "familyName": false,
    //         "middleName": false,
    //         "nickname": false,
    //         "profile": false,
    //         "picture": false,
    //         "website": false,
    //         "gender": false,
    //         "birthdate": false,
    //         "zoneinfo": false,
    //         "locale": false,

    //         "address": false
    //     },
    //     "userAttributes": {
    //         "address": "My college",
    //         "birthdate": "1/1/1970",
    //         "familyName": "Doe",
    //         "gender": "Male",
    //         "givenName": "",
    //         "locale": "",
    //         "middleName": "Stewart",
    //         "name": "Bob",
    //         "nickname": "Bobby",

    //         "picture": "",
    //         "profile": "",
    //         "website": "",
    //         "zoneinfo": ""
    //     },
    //     "createFieldOptions": [
    //         {
    //             "fieldId": "email",
    //             "description": "Email field is needed",
    //             "foreignIdentityKind": "EmailAddress",
    //             "fieldType": 1
    //         },
    //         {
    //             "fieldId": "username",
    //             "description": "",
    //             "humanReadable": false,
    //             "private": false,
    //             "allowedCharacters": "",
    //             "maxLength": 2,
    //             "fieldType": 0
    //         },
    //         {
    //             "fieldId": "password",
    //             "description": "Password field is needed",
    //             "fieldType": 2
    //         },
    //         {
    //             "fieldId": "securePassword",
    //             "description": "Secure Password :)",
    //             "identityFieldId": "email",
    //             "fieldType": 3
    //         },
    //         {
    //             "fieldId": "clientKey",
    //             "description": "Client Key",
    //             "alg": -42,
    //             "fieldType": 4
    //         }
    //     ],
    //     "fields": [
    //         {
    //             "options": {
    //                 "fieldId": "email",
    //                 "description": "Email field is needed",
    //                 "foreignIdentityKind": "EmailAddress",
    //                 "fieldType": 1
    //             },
    //             "value": "m"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "username",
    //                 "description": "",
    //                 "humanReadable": false,
    //                 "private": false,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "Axolotl"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "password",
    //                 "description": "Password field is needed",
    //                 "fieldType": 2
    //             },
    //             "value": "l90y-swcf-1vfl-nkl4"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "accountId",
    //                 "description": "internal account id",
    //                 "humanReadable": false,
    //                 "private": true,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "542466-88752"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "securePassword",
    //                 "description": "Secure Password :)",
    //                 "identityFieldId": "email",
    //                 "fieldType": 3
    //             },
    //             "value": "ohtv-5bh3-0ha4-10hu"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "clientKey",
    //                 "description": "Client Key",
    //                 "alg": -42,
    //                 "fieldType": 4,
    //                 "private": true
    //             },
    //             "value": "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAp4TnY8a+sgvPhChqLvibViVHKfcKbXieLS6C1uyTnlvtfJ1Yv7McLUcxPQstupio5UE0mVdYwZlC1nvbYTMF9aLEYZoVWiSqYM5BwJowZS+zhvARZH09fps36e+CtQPk5OWdJdp2mgwnGnf8OGBtB/4O60WD8ST+4nArSdST93BX1D3Zc0qWytJSbVGVi0NCTFIC4earUEAZcf8XZuio+ee74bwolV3OOve/bcuNblUIn0D9F5dI4z9Od0QFbneCmzthpbXy1ueQxhmAnxXhkqHL+x9DHQNIGd/PV7+7cKWFSWmNVKDveeBgAGPywGeJ9d+Dg7OQPl1UEob7/Lr7uMm1VOTFTp6HEwu8KCi4dTKujDKaxpGkyeV5xS1cfBzw1l5hWkYC/pCzqi5ggA+NYrlaCpCUYYolQlRKHqmOROBrBy4RgUin5YvLK97S+J0SajTRvWR+miSrSQLM/azh/7gRxFuhfJtJFWGij3WbLvNcQDRHL4ExDSc+6cIQdiJJS8CBw+WCKtr2ZNcjUQiLQWtQfDw0POogYxl8PkEH5GQW0N9QqoUWyWHqaIdMlzqG8CggC7KlZl7Ka0wp+jUtl8+JtoXKXYl5H07aC0Y9xtb3va8JeoX0G1FdZyN8Wzx7useS9ZPvgXmv4Ft1k4CxGWQrdkYspubskZqd2j9Y1dsCAwEAAQ=="
    //         }
    //     ],
    //     "putFields": [
    //         {
    //             "options": {
    //                 "fieldId": "accountId",
    //                 "description": "internal account id",
    //                 "humanReadable": false,
    //                 "private": true,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "542466-88752"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "webAppGeneratedKey",
    //                 "description": "WebApp-Generated Key",
    //                 "alg": -42,
    //                 "fieldType": 4
    //             },
    //             "value": "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"
    //         }
    //     ],
    //     "rejectedFieldValues": [
    //         {
    //             "fieldId": "username",
    //             "fieldType": 0,
    //             "code": "Other:Dummy",
    //             "description": "The value you have entered is invalid",
    //             "previousValues": [
    //                 "Axolotl",
    //                 "Swag"
    //             ],
    //             "webAppDictatedValue": "Potatoe"
    //         }
    //     ]
    // },
    // "rejectEmail": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "channelId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967480,
    //             "originalUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionState": 0,
    //         "sessionType": 1
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         {
    //             "requestId": "{e9d68eba-56d0-40b4-96ce-c925c454d248}",
    //             "requestType": "GetUserAttributes" as ERequestType
    //         },
    //         {
    //             "requestId": "{16d63d45-30f8-4127-b6a4-96ab734c46df}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{cadd1400-b079-4924-a61c-382d083d1494}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{d0505955-d206-45b0-b6f3-98357aee855d}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{573c3130-5b0f-4f5f-a473-58867d2786d1}",
    //             "requestType": "PutRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{fc13e5b1-ca65-49c7-966e-b49cc384db0c}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{9484b6ef-9433-4d06-82f7-08890b58a496}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{1750f3b9-290e-4dc0-b8ab-449cc5801540}",
    //             "requestType": "PutRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{375603f0-c5ff-4728-8615-28e651c2af1d}",
    //             "requestType": "RejectRegistrationFieldValue" as ERequestType
    //         }
    //     ],
    //     "id": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //     "context": {
    //         "request": {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         "tabId": 41
    //     },
    //     "version": 10,
    //     "requiredUserAttributes": {
    //         "name": false,
    //         "givenName": false,
    //         "familyName": false,
    //         "middleName": false,
    //         "nickname": false,
    //         "profile": false,
    //         "picture": false,
    //         "website": false,
    //         "gender": false,
    //         "birthdate": false,
    //         "zoneinfo": false,
    //         "locale": false,

    //         "address": false
    //     },
    //     "userAttributes": {
    //         "address": "My college",
    //         "birthdate": "1/1/1970",
    //         "familyName": "Doe",
    //         "gender": "Male",
    //         "givenName": "",
    //         "locale": "",
    //         "middleName": "Stewart",
    //         "name": "Bob",
    //         "nickname": "Bobby",

    //         "picture": "",
    //         "profile": "",
    //         "website": "",
    //         "zoneinfo": ""
    //     },
    //     "createFieldOptions": [
    //         {
    //             "fieldId": "email",
    //             "description": "Email field is needed",
    //             "foreignIdentityKind": "EmailAddress",
    //             "fieldType": 1
    //         },
    //         {
    //             "fieldId": "username",
    //             "description": "",
    //             "humanReadable": false,
    //             "private": false,
    //             "allowedCharacters": "",
    //             "maxLength": 2,
    //             "fieldType": 0
    //         },
    //         {
    //             "fieldId": "password",
    //             "description": "Password field is needed",
    //             "fieldType": 2
    //         },
    //         {
    //             "fieldId": "securePassword",
    //             "description": "Secure Password :)",
    //             "identityFieldId": "email",
    //             "fieldType": 3
    //         },
    //         {
    //             "fieldId": "clientKey",
    //             "description": "Client Key",
    //             "alg": -42,
    //             "fieldType": 4
    //         }
    //     ],
    //     "fields": [
    //         {
    //             "options": {
    //                 "fieldId": "email",
    //                 "description": "Email field is needed",
    //                 "foreignIdentityKind": "EmailAddress",
    //                 "fieldType": 1
    //             },
    //             "value": "bob.bobby@example.org"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "username",
    //                 "description": "",
    //                 "humanReadable": false,
    //                 "private": false,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "Axolotl"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "password",
    //                 "description": "Password field is needed",
    //                 "fieldType": 2
    //             },
    //             "value": "l90y-swcf-1vfl-nkl4"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "accountId",
    //                 "description": "internal account id",
    //                 "humanReadable": false,
    //                 "private": true,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "542466-88752"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "securePassword",
    //                 "description": "Secure Password :)",
    //                 "identityFieldId": "email",
    //                 "fieldType": 3
    //             },
    //             "value": "ohtv-5bh3-0ha4-10hu"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "clientKey",
    //                 "description": "Client Key",
    //                 "alg": -42,
    //                 "fieldType": 4,
    //                 "private": true
    //             },
    //             "value": "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAp4TnY8a+sgvPhChqLvibViVHKfcKbXieLS6C1uyTnlvtfJ1Yv7McLUcxPQstupio5UE0mVdYwZlC1nvbYTMF9aLEYZoVWiSqYM5BwJowZS+zhvARZH09fps36e+CtQPk5OWdJdp2mgwnGnf8OGBtB/4O60WD8ST+4nArSdST93BX1D3Zc0qWytJSbVGVi0NCTFIC4earUEAZcf8XZuio+ee74bwolV3OOve/bcuNblUIn0D9F5dI4z9Od0QFbneCmzthpbXy1ueQxhmAnxXhkqHL+x9DHQNIGd/PV7+7cKWFSWmNVKDveeBgAGPywGeJ9d+Dg7OQPl1UEob7/Lr7uMm1VOTFTp6HEwu8KCi4dTKujDKaxpGkyeV5xS1cfBzw1l5hWkYC/pCzqi5ggA+NYrlaCpCUYYolQlRKHqmOROBrBy4RgUin5YvLK97S+J0SajTRvWR+miSrSQLM/azh/7gRxFuhfJtJFWGij3WbLvNcQDRHL4ExDSc+6cIQdiJJS8CBw+WCKtr2ZNcjUQiLQWtQfDw0POogYxl8PkEH5GQW0N9QqoUWyWHqaIdMlzqG8CggC7KlZl7Ka0wp+jUtl8+JtoXKXYl5H07aC0Y9xtb3va8JeoX0G1FdZyN8Wzx7useS9ZPvgXmv4Ft1k4CxGWQrdkYspubskZqd2j9Y1dsCAwEAAQ=="
    //         }
    //     ],
    //     "putFields": [
    //         {
    //             "options": {
    //                 "fieldId": "accountId",
    //                 "description": "internal account id",
    //                 "humanReadable": false,
    //                 "private": true,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "542466-88752"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "webAppGeneratedKey",
    //                 "description": "WebApp-Generated Key",
    //                 "alg": -42,
    //                 "fieldType": 4
    //             },
    //             "value": "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"
    //         }
    //     ],
    //     "rejectedFieldValues": [
    //         {
    //             "fieldId": "email",
    //             "fieldType": 0,
    //             "code": "IdentityAlreadyExists",
    //             "description": "The identity value you have provided already exists!",
    //             "previousValues": [
    //                 "bob.bobby@example.org"
    //             ]
    //         }
    //     ]
    // },
    // "rejectClientKey": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "channelId": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967480,
    //             "originalUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96/build/index.html#/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "cc3f8ea1-ed03-46f4-a91b-5f1648b3fc96",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionState": 0,
    //         "sessionType": 1
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         {
    //             "requestId": "{e9d68eba-56d0-40b4-96ce-c925c454d248}",
    //             "requestType": "GetUserAttributes" as ERequestType
    //         },
    //         {
    //             "requestId": "{16d63d45-30f8-4127-b6a4-96ab734c46df}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{cadd1400-b079-4924-a61c-382d083d1494}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{d0505955-d206-45b0-b6f3-98357aee855d}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{573c3130-5b0f-4f5f-a473-58867d2786d1}",
    //             "requestType": "PutRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{fc13e5b1-ca65-49c7-966e-b49cc384db0c}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{9484b6ef-9433-4d06-82f7-08890b58a496}",
    //             "requestType": "CreateRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{1750f3b9-290e-4dc0-b8ab-449cc5801540}",
    //             "requestType": "PutRegistrationField" as ERequestType
    //         },
    //         {
    //             "requestId": "{375603f0-c5ff-4728-8615-28e651c2af1d}",
    //             "requestType": "RejectRegistrationFieldValue" as ERequestType
    //         },
    //         {
    //             "requestId": "{9179f40f-fca1-4e79-acaf-efb57b4d49c6}",
    //             "requestType": "RejectRegistrationFieldValue" as ERequestType
    //         }
    //     ],
    //     "id": "{f09aa365-957f-451e-8859-3b27640afc97}",
    //     "context": {
    //         "request": {
    //             "requestId": "{ff98337c-8fc3-4143-b0dc-cbe5e3333399}",
    //             "requestType": "ApproveSession" as ERequestType
    //         },
    //         "tabId": 41
    //     },
    //     "version": 10,
    //     "requiredUserAttributes": {
    //         "name": false,
    //         "givenName": false,
    //         "familyName": false,
    //         "middleName": false,
    //         "nickname": false,
    //         "profile": false,
    //         "picture": false,
    //         "website": false,
    //         "gender": false,
    //         "birthdate": false,
    //         "zoneinfo": false,
    //         "locale": false,

    //         "address": false
    //     },
    //     "userAttributes": {
    //         "address": "My college",
    //         "birthdate": "1/1/1970",
    //         "familyName": "Doe",
    //         "gender": "Male",
    //         "givenName": "",
    //         "locale": "",
    //         "middleName": "Stewart",
    //         "name": "Bob",
    //         "nickname": "Bobby",

    //         "picture": "",
    //         "profile": "",
    //         "website": "",
    //         "zoneinfo": ""
    //     },
    //     "createFieldOptions": [
    //         {
    //             "fieldId": "email",
    //             "description": "Email field is needed",
    //             "foreignIdentityKind": "EmailAddress",
    //             "fieldType": 1
    //         },
    //         {
    //             "fieldId": "username",
    //             "description": "",
    //             "humanReadable": false,
    //             "private": false,
    //             "allowedCharacters": "",
    //             "maxLength": 2,
    //             "fieldType": 0
    //         },
    //         {
    //             "fieldId": "password",
    //             "description": "Password field is needed",
    //             "fieldType": 2
    //         },
    //         {
    //             "fieldId": "securePassword",
    //             "description": "Secure Password :)",
    //             "identityFieldId": "email",
    //             "fieldType": 3
    //         },
    //         {
    //             "fieldId": "clientKey",
    //             "description": "Client Key",
    //             "alg": -42,
    //             "fieldType": 4
    //         }
    //     ],
    //     "fields": [
    //         {
    //             "options": {
    //                 "fieldId": "email",
    //                 "description": "Email field is needed",
    //                 "foreignIdentityKind": "EmailAddress",
    //                 "fieldType": 1
    //             },
    //             "value": "m"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "username",
    //                 "description": "",
    //                 "humanReadable": false,
    //                 "private": false,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "Axolotl"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "password",
    //                 "description": "Password field is needed",
    //                 "fieldType": 2
    //             },
    //             "value": "l90y-swcf-1vfl-nkl4"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "accountId",
    //                 "description": "internal account id",
    //                 "humanReadable": false,
    //                 "private": true,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "542466-88752"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "securePassword",
    //                 "description": "Secure Password :)",
    //                 "identityFieldId": "email",
    //                 "fieldType": 3
    //             },
    //             "value": "ohtv-5bh3-0ha4-10hu"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "clientKey",
    //                 "description": "Client Key",
    //                 "alg": -42,
    //                 "fieldType": 4,
    //                 "private": true
    //             },
    //             "value": "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAp4TnY8a+sgvPhChqLvibViVHKfcKbXieLS6C1uyTnlvtfJ1Yv7McLUcxPQstupio5UE0mVdYwZlC1nvbYTMF9aLEYZoVWiSqYM5BwJowZS+zhvARZH09fps36e+CtQPk5OWdJdp2mgwnGnf8OGBtB/4O60WD8ST+4nArSdST93BX1D3Zc0qWytJSbVGVi0NCTFIC4earUEAZcf8XZuio+ee74bwolV3OOve/bcuNblUIn0D9F5dI4z9Od0QFbneCmzthpbXy1ueQxhmAnxXhkqHL+x9DHQNIGd/PV7+7cKWFSWmNVKDveeBgAGPywGeJ9d+Dg7OQPl1UEob7/Lr7uMm1VOTFTp6HEwu8KCi4dTKujDKaxpGkyeV5xS1cfBzw1l5hWkYC/pCzqi5ggA+NYrlaCpCUYYolQlRKHqmOROBrBy4RgUin5YvLK97S+J0SajTRvWR+miSrSQLM/azh/7gRxFuhfJtJFWGij3WbLvNcQDRHL4ExDSc+6cIQdiJJS8CBw+WCKtr2ZNcjUQiLQWtQfDw0POogYxl8PkEH5GQW0N9QqoUWyWHqaIdMlzqG8CggC7KlZl7Ka0wp+jUtl8+JtoXKXYl5H07aC0Y9xtb3va8JeoX0G1FdZyN8Wzx7useS9ZPvgXmv4Ft1k4CxGWQrdkYspubskZqd2j9Y1dsCAwEAAQ=="
    //         }
    //     ],
    //     "putFields": [
    //         {
    //             "options": {
    //                 "fieldId": "accountId",
    //                 "description": "internal account id",
    //                 "humanReadable": false,
    //                 "private": true,
    //                 "allowedCharacters": "",
    //                 "maxLength": 2,
    //                 "fieldType": 0
    //             },
    //             "value": "542466-88752"
    //         },
    //         {
    //             "options": {
    //                 "fieldId": "webAppGeneratedKey",
    //                 "description": "WebApp-Generated Key",
    //                 "alg": -42,
    //                 "fieldType": 4
    //             },
    //             "value": "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"
    //         }
    //     ],
    //     "rejectedFieldValues": [
    //         {
    //             "fieldId": "clientKey",
    //             "fieldType": 0,
    //             "code": "Other:Dummy",
    //             "description": "The value you have entered is invalid",
    //             "previousValues": []
    //         }
    //     ]
    // },
    // "approveChallengeIdentification": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{5f4a917d-741b-4a3f-b8e7-d6cec7edb159}",
    //         "channelId": "{5f4a917d-741b-4a3f-b8e7-d6cec7edb159}",
    //         "sessionState": 0,
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967308,
    //             "originalUri": {
    //                 "uri": "moz-extension://e8ae7536-6748-4b46-a8bc-c249b7aec474/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "e8ae7536-6748-4b46-a8bc-c249b7aec474",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://e8ae7536-6748-4b46-a8bc-c249b7aec474/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "e8ae7536-6748-4b46-a8bc-c249b7aec474",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionType": 0
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{d78628af-5c5c-4b57-8e6e-3b4be51fca07}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{49b6d8a0-80cf-45a7-be87-bbf2ab0a79fb}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         }
    //     ],
    //     "id": "{5f4a917d-741b-4a3f-b8e7-d6cec7edb159}",
    //     "context": {
    //         "request": {
    //             "requestId": "{d78628af-5c5c-4b57-8e6e-3b4be51fca07}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType,
    //         },
    //         "tabId": 3
    //     },
    //     "version": 2,
    //     "selectedAccountId": "1xj4-a67y-vrk8-v2c5",
    //     "challenges": {
    //         "identification": {
    //             "id": "identification",
    //             "type": 0,
    //             "parameters": {},
    //             "state": 1,
    //             "messages": {}
    //         }
    //     }
    // },
    // "respondGetIdentityFields": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{bbf0535d-2515-4a31-be9e-ce5418a5be9c}",
    //         "channelId": "{bbf0535d-2515-4a31-be9e-ce5418a5be9c}",
    //         "sessionState": 0,
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967307,
    //             "originalUri": {
    //                 "uri": "moz-extension://8088dbba-1fe4-4dc1-864e-7c83fc210e2f/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "8088dbba-1fe4-4dc1-864e-7c83fc210e2f",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://8088dbba-1fe4-4dc1-864e-7c83fc210e2f/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "8088dbba-1fe4-4dc1-864e-7c83fc210e2f",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionType": 0
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{38fabdc1-047f-40f8-935d-61517c27dcb3}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{0d651a1d-92de-4769-aa59-d26ca22e8a73}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{09bd67de-5aba-41ee-829b-799e6884b8b6}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         }
    //     ],
    //     "id": "{bbf0535d-2515-4a31-be9e-ce5418a5be9c}",
    //     "context": {
    //         "request": {
    //             "requestId": "{38fabdc1-047f-40f8-935d-61517c27dcb3}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         "tabId": 3
    //     },
    //     "version": 3,
    //     "selectedAccountId": Accounts[0].id,
    //     "challenges": {
    //         "identification": {
    //             "id": "identification",
    //             "type": 0,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetIdentityFields": {
    //                     "name": "GetIdentityFields",
    //                     "payload": [
    //                         "accountId",
    //                         "username"
    //                     ]
    //                 }
    //             }
    //         }
    //     }
    // },
    // "respondGetIdentityFieldsWithUnknownFieldId": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{bbf0535d-2515-4a31-be9e-ce5418a5be9c}",
    //         "channelId": "{bbf0535d-2515-4a31-be9e-ce5418a5be9c}",
    //         "sessionState": 0,
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967307,
    //             "originalUri": {
    //                 "uri": "moz-extension://8088dbba-1fe4-4dc1-864e-7c83fc210e2f/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "8088dbba-1fe4-4dc1-864e-7c83fc210e2f",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://8088dbba-1fe4-4dc1-864e-7c83fc210e2f/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "8088dbba-1fe4-4dc1-864e-7c83fc210e2f",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionType": 0
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{38fabdc1-047f-40f8-935d-61517c27dcb3}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{0d651a1d-92de-4769-aa59-d26ca22e8a73}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{09bd67de-5aba-41ee-829b-799e6884b8b6}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         }
    //     ],
    //     "id": "{bbf0535d-2515-4a31-be9e-ce5418a5be9c}",
    //     "context": {
    //         "request": {
    //             "requestId": "{38fabdc1-047f-40f8-935d-61517c27dcb3}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         "tabId": 3
    //     },
    //     "version": 3,
    //     "selectedAccountId": Accounts[0].id,
    //     "challenges": {
    //         "identification": {
    //             "id": "identification",
    //             "type": 0,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetIdentityFields": {
    //                     "name": "GetIdentityFields",
    //                     "payload": [
    //                         "accountId",
    //                         "username",
    //                         "email"
    //                     ]
    //                 }
    //             }
    //         }
    //     }
    // },
    // "respondGetIdentityFieldsWithInvalidAccountId": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{bbf0535d-2515-4a31-be9e-ce5418a5be9c}",
    //         "channelId": "{bbf0535d-2515-4a31-be9e-ce5418a5be9c}",
    //         "sessionState": 0,
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967307,
    //             "originalUri": {
    //                 "uri": "moz-extension://8088dbba-1fe4-4dc1-864e-7c83fc210e2f/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "8088dbba-1fe4-4dc1-864e-7c83fc210e2f",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://8088dbba-1fe4-4dc1-864e-7c83fc210e2f/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "8088dbba-1fe4-4dc1-864e-7c83fc210e2f",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionType": 0
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{38fabdc1-047f-40f8-935d-61517c27dcb3}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{0d651a1d-92de-4769-aa59-d26ca22e8a73}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{09bd67de-5aba-41ee-829b-799e6884b8b6}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         }
    //     ],
    //     "id": "{bbf0535d-2515-4a31-be9e-ce5418a5be9c}",
    //     "context": {
    //         "request": {
    //             "requestId": "{38fabdc1-047f-40f8-935d-61517c27dcb3}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         "tabId": 3
    //     },
    //     "version": 3,
    //     "selectedAccountId": "1x9n-jj9y-wjh3-0tj3",
    //     "challenges": {
    //         "identification": {
    //             "id": "identification",
    //             "type": 0,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetIdentityFields": {
    //                     "name": "GetIdentityFields",
    //                     "payload": [
    //                         "accountId",
    //                         "username",
    //                         "email"
    //                     ]
    //                 }
    //             }
    //         }
    //     }
    // },
    // "approveChallengePassword": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{22b73f0b-1005-4684-944c-88c571b22405}",
    //         "channelId": "{22b73f0b-1005-4684-944c-88c571b22405}",
    //         "sessionState": 0,
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967322,
    //             "originalUri": {
    //                 "uri": "moz-extension://4e50b9ca-d4a4-4c5d-a5a8-f6798f0c5def/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "4e50b9ca-d4a4-4c5d-a5a8-f6798f0c5def",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://4e50b9ca-d4a4-4c5d-a5a8-f6798f0c5def/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "4e50b9ca-d4a4-4c5d-a5a8-f6798f0c5def",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionType": 0
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{64931d2d-10a8-4623-a581-14b9cb2c277c}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{b86a8aba-0cfc-4944-92e8-b495f638e30f}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{ea45ab03-6e24-4318-ba7f-674b4c69585f}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{45cd94b4-22d7-4144-a1c6-6ef910ff72c6}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         }
    //     ],
    //     "id": "{22b73f0b-1005-4684-944c-88c571b22405}",
    //     "context": {
    //         "request": {
    //             "requestId": "{64931d2d-10a8-4623-a581-14b9cb2c277c}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         "tabId": 6
    //     },
    //     "version": 4,
    //     "selectedAccountId": "a7zd-bs1y-gdxs-o0c5",
    //     "challenges": {
    //         "identification": {
    //             "id": "identification",
    //             "type": 0,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetIdentityFields": {
    //                     "name": "GetIdentityFields",
    //                     "payload": [
    //                         "accountId",
    //                         "username",
    //                         "email"
    //                     ]
    //                 }
    //             }
    //         },
    //         "password": {
    //             "id": "password",
    //             "type": 2,
    //             "parameters": {},
    //             "state": 1,
    //             "messages": {}
    //         }
    //     }
    // },
    // "respondGetPasswordFields": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{22b73f0b-1005-4684-944c-88c571b22405}",
    //         "channelId": "{22b73f0b-1005-4684-944c-88c571b22405}",
    //         "sessionState": 0,
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967322,
    //             "originalUri": {
    //                 "uri": "moz-extension://4e50b9ca-d4a4-4c5d-a5a8-f6798f0c5def/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "4e50b9ca-d4a4-4c5d-a5a8-f6798f0c5def",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://4e50b9ca-d4a4-4c5d-a5a8-f6798f0c5def/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "4e50b9ca-d4a4-4c5d-a5a8-f6798f0c5def",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionType": 0
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{64931d2d-10a8-4623-a581-14b9cb2c277c}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{b86a8aba-0cfc-4944-92e8-b495f638e30f}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{ea45ab03-6e24-4318-ba7f-674b4c69585f}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{45cd94b4-22d7-4144-a1c6-6ef910ff72c6}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{976a8a8c-2caf-4708-86b3-759105f2add3}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         }
    //     ],
    //     "id": "{22b73f0b-1005-4684-944c-88c571b22405}",
    //     "context": {
    //         "request": {
    //             "requestId": "{64931d2d-10a8-4623-a581-14b9cb2c277c}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         "tabId": 6
    //     },
    //     "version": 5,
    //     "selectedAccountId": Accounts[0].id,
    //     "challenges": {
    //         "identification": {
    //             "id": "identification",
    //             "type": 0,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetIdentityFields": {
    //                     "name": "GetIdentityFields",
    //                     "payload": [
    //                         "accountId",
    //                         "username",
    //                         "email"
    //                     ]
    //                 }
    //             }
    //         },
    //         "password": {
    //             "id": "password",
    //             "type": 2,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetPasswordFields": {
    //                     "name": "GetPasswordFields",
    //                     "payload": [
    //                         "password"
    //                     ]
    //                 }
    //             }
    //         }
    //     }
    // },
    // "approveChallengeSrp": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{db983750-db10-4cf8-bf97-635f27ceaf0a}",
    //         "channelId": "{db983750-db10-4cf8-bf97-635f27ceaf0a}",
    //         "sessionState": 0,
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967308,
    //             "originalUri": {
    //                 "uri": "moz-extension://4e50b9ca-d4a4-4c5d-a5a8-f6798f0c5def/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "4e50b9ca-d4a4-4c5d-a5a8-f6798f0c5def",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://4e50b9ca-d4a4-4c5d-a5a8-f6798f0c5def/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "4e50b9ca-d4a4-4c5d-a5a8-f6798f0c5def",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionType": 0
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{3204827c-eb98-4b55-bc3f-b11c2775bd5b}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{7da5fc37-7b1e-4622-88d0-ecadca469373}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{48769aee-9c1c-4a67-b005-d76808b28530}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{489926f4-3797-4456-a701-911de313e851}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{6d13638a-1995-4bf3-8ac1-ce5560a2313f}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{06c317c9-5492-4183-83c1-474a6142a842}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         }
    //     ],
    //     "id": "{db983750-db10-4cf8-bf97-635f27ceaf0a}",
    //     "context": {
    //         "request": {
    //             "requestId": "{3204827c-eb98-4b55-bc3f-b11c2775bd5b}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         "tabId": 3
    //     },
    //     "version": 6,
    //     "selectedAccountId": Accounts[0].id,
    //     "challenges": {
    //         "identification": {
    //             "id": "identification",
    //             "type": 0,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetIdentityFields": {
    //                     "name": "GetIdentityFields",
    //                     "payload": [
    //                         "accountId",
    //                         "username",
    //                         "email"
    //                     ]
    //                 }
    //             }
    //         },
    //         "password": {
    //             "id": "password",
    //             "type": 2,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetPasswordFields": {
    //                     "name": "GetPasswordFields",
    //                     "payload": [
    //                         "password"
    //                     ]
    //                 }
    //             }
    //         },
    //         "srp": {
    //             "id": "srp",
    //             "type": 3,
    //             "parameters": {},
    //             "state": 1,
    //             "messages": {}
    //         }
    //     }
    // },
    // "respondSelectSecurePasswordField": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{db983750-db10-4cf8-bf97-635f27ceaf0a}",
    //         "channelId": "{db983750-db10-4cf8-bf97-635f27ceaf0a}",
    //         "sessionState": 0,
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967308,
    //             "originalUri": {
    //                 "uri": "moz-extension://4e50b9ca-d4a4-4c5d-a5a8-f6798f0c5def/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "4e50b9ca-d4a4-4c5d-a5a8-f6798f0c5def",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://4e50b9ca-d4a4-4c5d-a5a8-f6798f0c5def/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "4e50b9ca-d4a4-4c5d-a5a8-f6798f0c5def",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionType": 0
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{3204827c-eb98-4b55-bc3f-b11c2775bd5b}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{7da5fc37-7b1e-4622-88d0-ecadca469373}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{48769aee-9c1c-4a67-b005-d76808b28530}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{489926f4-3797-4456-a701-911de313e851}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{6d13638a-1995-4bf3-8ac1-ce5560a2313f}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{06c317c9-5492-4183-83c1-474a6142a842}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{984aaadb-37ca-44d6-950f-4f450ad8d750}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         }
    //     ],
    //     "id": "{db983750-db10-4cf8-bf97-635f27ceaf0a}",
    //     "context": {
    //         "request": {
    //             "requestId": "{3204827c-eb98-4b55-bc3f-b11c2775bd5b}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         "tabId": 3
    //     },
    //     "version": 7,
    //     "selectedAccountId": Accounts[0].id,
    //     "challenges": {
    //         "identification": {
    //             "id": "identification",
    //             "type": 0,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetIdentityFields": {
    //                     "name": "GetIdentityFields",
    //                     "payload": [
    //                         "accountId",
    //                         "username",
    //                         "email"
    //                     ]
    //                 }
    //             }
    //         },
    //         "password": {
    //             "id": "password",
    //             "type": 2,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetPasswordFields": {
    //                     "name": "GetPasswordFields",
    //                     "payload": [
    //                         "password"
    //                     ]
    //                 }
    //             }
    //         },
    //         "srp": {
    //             "id": "srp",
    //             "type": 3,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "SelectSecurePasswordField": {
    //                     "name": "SelectSecurePasswordField",
    //                     "payload": "securePassword"
    //                 }
    //             }
    //         }
    //     }
    // },
    // "respondExchangePublicKeys": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{9edc496a-5dbd-48b8-bff8-f5c3e798f9d7}",
    //         "channelId": "{9edc496a-5dbd-48b8-bff8-f5c3e798f9d7}",
    //         "sessionState": 0,
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967363,
    //             "originalUri": {
    //                 "uri": "moz-extension://c7be6a26-15be-4e21-b2dc-abc76108f258/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "c7be6a26-15be-4e21-b2dc-abc76108f258",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://c7be6a26-15be-4e21-b2dc-abc76108f258/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "c7be6a26-15be-4e21-b2dc-abc76108f258",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionType": 0
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{c31cb287-9240-4426-a2bb-f3a8519f4474}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{6c499f9a-2eac-406a-a839-e84e1ef2556a}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{883ac50a-74ce-48ff-b248-2f04b2a24859}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{22851964-1d68-4ee6-9296-b7659958d000}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{58f80464-7aed-4f8a-a6d9-7a5b5f700dbd}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{70d5904b-3b0b-45bb-a9dd-dcfda6f85a02}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{6c4ebb36-bd2d-4f76-814a-134425f1f899}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{3ad3573b-f3bd-42ca-bc15-c8d990be4f2a}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         }
    //     ],
    //     "id": "{9edc496a-5dbd-48b8-bff8-f5c3e798f9d7}",
    //     "context": {
    //         "request": {
    //             "requestId": "{c31cb287-9240-4426-a2bb-f3a8519f4474}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         "tabId": 8
    //     },
    //     "version": 8,
    //     "selectedAccountId": "d52m-19tm-a8ug-h61a",
    //     "challenges": {
    //         "identification": {
    //             "id": "identification",
    //             "type": 0,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetIdentityFields": {
    //                     "name": "GetIdentityFields",
    //                     "payload": [
    //                         "accountId",
    //                         "username",
    //                         "email"
    //                     ]
    //                 }
    //             }
    //         },
    //         "password": {
    //             "id": "password",
    //             "type": 2,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetPasswordFields": {
    //                     "name": "GetPasswordFields",
    //                     "payload": [
    //                         "password"
    //                     ]
    //                 }
    //             }
    //         },
    //         "srp": {
    //             "id": "srp",
    //             "type": 3,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "SelectSecurePasswordField": {
    //                     "name": "SelectSecurePasswordField",
    //                     "payload": "securePassword"
    //                 },
    //                 "ExchangePublicKeys": {
    //                     "name": "ExchangePublicKeys",
    //                     "payload": "11c6025c2e7c35c4dc02baf7cc0208c0dd0a140941734de91cb7590446544d943d0e11b617a12f7924859ae4d02c69bbdfda167bea8e0ba052794fc8d7ef81edd6bc15aa0c44859ac67b2e65811dbd0f08404b29ecbdf09246008177d25e60352b566159bc909a2f6b42d6113cb32fd97e479c0ba45daf1d72683383343b49bcb030b6b3399e51a564fd20a3e35b8cc24d90c3de260052b3475a64b1f7df9aebac88533a21a3ff986049eb349f358b4f74a8930fc2d343c80a879a73bd6a46bebcedd5b8b6cf710796450fc99186c8ec490c516a2d0c873963680c32d86ae43b623eab0f8e64ea4a443aee322f1b27c530928ae81d9ffd30823f3c68eb1373d9c7a9f3a64972f4325070e31d11a33d2e66ba40f51ad9af7fb76320905058fff4b77e6d28207f419f1ccea27b436f712e2d366d167cfbec1536971a818b2a168a03dd4433e3030cbddadb919b535ac4353e345e95b7b94a24a857ce6877615ffac3624c44a80543c2244967431d32c87d2658f7f2be03cf097c80cd1894cad5159a421aadf4f14343b8c6af6dbc01a54fdfa5c11f2743d08177f5eede5b293a63da193511319a352f8957e71a93b2163f2b4d92118b902ddf578969c0316d804d76db879947a67059dcb3fcf4f4111f96ebaf50858d51caed7161c55c1a5cf244a147463c0462b33f7eca624004a77822591baf1d0115c611799d46c9f6ef1386"
    //                 }
    //             },
    //             "srpState": {
    //                 "fields": {
    //                     "username": "bob.bobby@example.org",
    //                     "password": "y8lt-yj2l-e1sa-qs8n"
    //                 }
    //             }
    //         }
    //     }
    // },
    // "respondComputeClientProof": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{9edc496a-5dbd-48b8-bff8-f5c3e798f9d7}",
    //         "channelId": "{9edc496a-5dbd-48b8-bff8-f5c3e798f9d7}",
    //         "sessionState": 0,
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967363,
    //             "originalUri": {
    //                 "uri": "moz-extension://c7be6a26-15be-4e21-b2dc-abc76108f258/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "c7be6a26-15be-4e21-b2dc-abc76108f258",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://c7be6a26-15be-4e21-b2dc-abc76108f258/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "c7be6a26-15be-4e21-b2dc-abc76108f258",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionType": 0
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{c31cb287-9240-4426-a2bb-f3a8519f4474}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{6c499f9a-2eac-406a-a839-e84e1ef2556a}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{883ac50a-74ce-48ff-b248-2f04b2a24859}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{22851964-1d68-4ee6-9296-b7659958d000}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{58f80464-7aed-4f8a-a6d9-7a5b5f700dbd}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{70d5904b-3b0b-45bb-a9dd-dcfda6f85a02}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{6c4ebb36-bd2d-4f76-814a-134425f1f899}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{3ad3573b-f3bd-42ca-bc15-c8d990be4f2a}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{f7f70a7e-7f01-4deb-8d19-9fefc5efdcbb}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         }
    //     ],
    //     "id": "{9edc496a-5dbd-48b8-bff8-f5c3e798f9d7}",
    //     "context": {
    //         "request": {
    //             "requestId": "{c31cb287-9240-4426-a2bb-f3a8519f4474}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         "tabId": 8
    //     },
    //     "version": 9,
    //     "selectedAccountId": "d52m-19tm-a8ug-h61a",
    //     "challenges": {
    //         "identification": {
    //             "id": "identification",
    //             "type": 0,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetIdentityFields": {
    //                     "name": "GetIdentityFields",
    //                     "payload": [
    //                         "accountId",
    //                         "username",
    //                         "email"
    //                     ]
    //                 }
    //             }
    //         },
    //         "password": {
    //             "id": "password",
    //             "type": 2,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetPasswordFields": {
    //                     "name": "GetPasswordFields",
    //                     "payload": [
    //                         "password"
    //                     ]
    //                 }
    //             }
    //         },
    //         "srp": {
    //             "id": "srp",
    //             "type": 3,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "SelectSecurePasswordField": {
    //                     "name": "SelectSecurePasswordField",
    //                     "payload": "securePassword"
    //                 },
    //                 "ExchangePublicKeys": {
    //                     "name": "ExchangePublicKeys",
    //                     "payload": "11c6025c2e7c35c4dc02baf7cc0208c0dd0a140941734de91cb7590446544d943d0e11b617a12f7924859ae4d02c69bbdfda167bea8e0ba052794fc8d7ef81edd6bc15aa0c44859ac67b2e65811dbd0f08404b29ecbdf09246008177d25e60352b566159bc909a2f6b42d6113cb32fd97e479c0ba45daf1d72683383343b49bcb030b6b3399e51a564fd20a3e35b8cc24d90c3de260052b3475a64b1f7df9aebac88533a21a3ff986049eb349f358b4f74a8930fc2d343c80a879a73bd6a46bebcedd5b8b6cf710796450fc99186c8ec490c516a2d0c873963680c32d86ae43b623eab0f8e64ea4a443aee322f1b27c530928ae81d9ffd30823f3c68eb1373d9c7a9f3a64972f4325070e31d11a33d2e66ba40f51ad9af7fb76320905058fff4b77e6d28207f419f1ccea27b436f712e2d366d167cfbec1536971a818b2a168a03dd4433e3030cbddadb919b535ac4353e345e95b7b94a24a857ce6877615ffac3624c44a80543c2244967431d32c87d2658f7f2be03cf097c80cd1894cad5159a421aadf4f14343b8c6af6dbc01a54fdfa5c11f2743d08177f5eede5b293a63da193511319a352f8957e71a93b2163f2b4d92118b902ddf578969c0316d804d76db879947a67059dcb3fcf4f4111f96ebaf50858d51caed7161c55c1a5cf244a147463c0462b33f7eca624004a77822591baf1d0115c611799d46c9f6ef1386"
    //                 },
    //                 "ComputeClientProof": {
    //                     "name": "ComputeClientProof",
    //                     "payload": "7841fd014c8773236d0701419622105126dfa46c709bd2c1ae03ba546b4b9b8d"
    //                 }
    //             },
    //             "srpState": {
    //                 "fields": {
    //                     "username": "bob.bobby@example.org",
    //                     "password": "y8lt-yj2l-e1sa-qs8n"
    //                 },
    //                 "clientPrivateKeyHexa": "9ea1e3e204de968d6fca9a86db22cdf8336ebfb7387c881f1393af9a7cca028a2e3077e893dd1232d04b1d7011c128c20180ebf7c16696138bfc2a16a78a8c44",
    //                 "serverPublicKeyHexB": "11c6025c2e7c35c4dc02baf7cc0208c0dd0a140941734de91cb7590446544d943d0e11b617a12f7924859ae4d02c69bbdfda167bea8e0ba052794fc8d7ef81edd6bc15aa0c44859ac67b2e65811dbd0f08404b29ecbdf09246008177d25e60352b566159bc909a2f6b42d6113cb32fd97e479c0ba45daf1d72683383343b49bcb030b6b3399e51a564fd20a3e35b8cc24d90c3de260052b3475a64b1f7df9aebac88533a21a3ff986049eb349f358b4f74a8930fc2d343c80a879a73bd6a46bebcedd5b8b6cf710796450fc99186c8ec490c516a2d0c873963680c32d86ae43b623eab0f8e64ea4a443aee322f1b27c530928ae81d9ffd30823f3c68eb1373d9c7a9f3a64972f4325070e31d11a33d2e66ba40f51ad9af7fb76320905058fff4b77e6d28207f419f1ccea27b436f712e2d366d167cfbec1536971a818b2a168a03dd4433e3030cbddadb919b535ac4353e345e95b7b94a24a857ce6877615ffac3624c44a80543c2244967431d32c87d2658f7f2be03cf097c80cd1894cad5159a421aadf4f14343b8c6af6dbc01a54fdfa5c11f2743d08177f5eede5b293a63da193511319a352f8957e71a93b2163f2b4d92118b902ddf578969c0316d804d76db879947a67059dcb3fcf4f4111f96ebaf50858d51caed7161c55c1a5cf244a147463c0462b33f7eca624004a77822591baf1d0115c611799d46c9f6ef1386"
    //             }
    //         }
    //     }
    // },
    // "approveChallengeDigitalSignature": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{d8fef61b-58f1-43b2-bbc1-a07ca99cd0dc}",
    //         "channelId": "{d8fef61b-58f1-43b2-bbc1-a07ca99cd0dc}",
    //         "sessionState": 0,
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967462,
    //             "originalUri": {
    //                 "uri": "moz-extension://8ad10c72-9b45-4e90-b620-f47528583032/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "8ad10c72-9b45-4e90-b620-f47528583032",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://8ad10c72-9b45-4e90-b620-f47528583032/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "8ad10c72-9b45-4e90-b620-f47528583032",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionType": 0
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{e270d717-c41b-4201-848c-458efaaf489f}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{e98e0c2d-ff38-4adc-b5c3-521c64bb2e5b}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{9243c56c-568d-43c6-913b-e89837dce3c4}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{77c6572a-7a1b-4ece-9d3b-7eccd3091c30}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{cd8d03cd-9807-4f38-9a75-10e62607729a}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{10ff97cc-00fc-490b-9cc0-4b415529519a}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{386877c6-ca04-44f6-9c0b-5292295878dd}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{f9434dd6-2f06-4887-a8b0-2fad6b8a8b43}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{39689e06-b7ee-4b75-90f8-b7437422e801}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{0a529905-a4b0-476a-8834-7893ed06a402}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{11ff73c4-5d0c-4855-a72c-dd8e39ba1ee1}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         }
    //     ],
    //     "id": "{d8fef61b-58f1-43b2-bbc1-a07ca99cd0dc}",
    //     "context": {
    //         "request": {
    //             "requestId": "{e270d717-c41b-4201-848c-458efaaf489f}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         "tabId": 19
    //     },
    //     "version": 11,
    //     "selectedAccountId": Accounts[0].id,
    //     "challenges": {
    //         "identification": {
    //             "id": "identification",
    //             "type": 0,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetIdentityFields": {
    //                     "name": "GetIdentityFields",
    //                     "payload": [
    //                         "accountId",
    //                         "username",
    //                         "email"
    //                     ]
    //                 }
    //             }
    //         },
    //         "password": {
    //             "id": "password",
    //             "type": 2,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetPasswordFields": {
    //                     "name": "GetPasswordFields",
    //                     "payload": [
    //                         "password"
    //                     ]
    //                 }
    //             }
    //         },
    //         "srp": {
    //             "id": "srp",
    //             "type": 3,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "SelectSecurePasswordField": {
    //                     "name": "SelectSecurePasswordField",
    //                     "payload": "securePassword"
    //                 },
    //                 "ExchangePublicKeys": {
    //                     "name": "ExchangePublicKeys",
    //                     "payload": "17d76f229f95afbdbabb3aca67e43d6950707abeb8e8b480357002a6221736f799d9ec943b59a48a52644019302fa17407ac5065388628f0b5096db96df53318ad2cd3d42e16cad38481fb8feebc57bb2a5820a530099520edbb241b5370a9ba8de1d41f0f0a390e3bf78bc570894d3988b8bfbade6be864e5dbaf0b152f012c5b1c022a75983d395697b80a605c0dd63eec15c8f7fd5f26dc7b77383bf6e3b6a59935b1c18b548aadd183f285eb43e13599d54b42733d0f773d87526afbc495bbf4cb8f291963a01c52d49f1e25382af9d9d1e7366e44d1724ed2d2a26b50ea3ac0d997bdc76cc23bc2e957b208cc0c1d4839d7eaa91d4ead45150bb9431fc92eb54b196c18be593e113125a2e172f93a8f39c556e5307d5881eb2ec19d9e61d4fe7e8109deb670e8e9c811b450120a97d47ffe321a823de6a23beeb9a6596be4ac426b95d5adce5f76e68ae25edb54909abbbc2b7e25eacfa8879a54acd0b13312e794219c767061f6b2776cdd6132fd15a1f11bfc299b81485e651244b82c7332e03ab79dfc3331a37afe9eb5317a156a4543c69645fbdecc9256f15a6f5ff0072e5c11c9626ee3f520014e590124068dcb1bf5c4cbc149d7127a03da8c0257c2206ec5acf0f8ae00db8e4c07409cd2e1177b915ee42b70851507e249dc15ab75d6bde7c8b63de8e46112b0c77c4596db4d2eb5d75eb3d52e3bd40366e1f2"
    //                 },
    //                 "ComputeClientProof": {
    //                     "name": "ComputeClientProof",
    //                     "payload": "0edb5309768825398e4aa197f9b554d1e37483c92acf560330e16b1f1ccbb675"
    //                 },
    //                 "VerifyServerProof": {
    //                     "name": "VerifyServerProof",
    //                     "payload": "29c79283ec666731fd01ef2df65a6f8f3e435a8f08fd2d3e44c4dae1e1c63f11"
    //                 }
    //             },
    //             "srpState": {
    //                 "fields": {
    //                     "username": "bob.bobby@example.org",
    //                     "password": "4e5k-6kc4-crmz-70kv"
    //                 },
    //                 "clientPrivateKeyHexa": "efd42c1c9f8511df093bb539bdf864c1acd9f2c444b25598facea59a87281d44",
    //                 "clientPublicKeyHexA": "5ee7c09e1f90070b81c8023bd795ba30fe1253382691caf6f416a3a3f1d56d82a63239d674bdd67b7b03731bc9d38b44afe1dd9109e8d9ddb8f146d6e41c13d606871c1c2edadc9ec55d785d8bdbd062b5c6827ef202dfbcab23df34aefa86e69bfe96d5d4765c6545ce379ee1ebea78fcef93d86b26dbd90aaf50efd5ed890ac3ae8e6d2ba2e96ae5d6667b44dd0a5528f28c5ea6a99bbd566635cce7c45cad2aba9e5414bae7b7b4871d4f107dbe0a525600c883aa709a283979fca50f4b111d702f0cf4c76c118fc7d55de0dd3322ee41492e90839ecf7524feb13a2b58c4b6e620ef59d13736fbc8954870509ea818433105ce4221c6352046000986214f95d9d3194fd7dd6b9389f0b39cc4651a643a1996e380476d58f583aa5d64cc9643dd028519d04245327787508872907124a685cd294899d3441316bad2c6399b4285e145eab9ebf16701e6ba275defb55ee19b187f6f0f09a3acd96ca90efc53bcc59cd3a385e4cace9d4e801fd1d2c4b0981a0bbda5336c8c961eec0c0a18f59305f0062a06db939e206408bd74d53d76e3f87e0327288c7262cc898203329d665ab908cb306ba13d3095aa9cf3a3295dbb59c9b26ef90b7d19f8acbc386d1c721c73deaf34590ef9d90e2f3c49bd0c6f4c9e87e37b369f0a75845eea0a5599e6ffa61bd037a4ff01d66d9c180d4adc2e84490ce5135d31e689ad055e29eb1b",
    //                 "serverPublicKeyHexB": "17d76f229f95afbdbabb3aca67e43d6950707abeb8e8b480357002a6221736f799d9ec943b59a48a52644019302fa17407ac5065388628f0b5096db96df53318ad2cd3d42e16cad38481fb8feebc57bb2a5820a530099520edbb241b5370a9ba8de1d41f0f0a390e3bf78bc570894d3988b8bfbade6be864e5dbaf0b152f012c5b1c022a75983d395697b80a605c0dd63eec15c8f7fd5f26dc7b77383bf6e3b6a59935b1c18b548aadd183f285eb43e13599d54b42733d0f773d87526afbc495bbf4cb8f291963a01c52d49f1e25382af9d9d1e7366e44d1724ed2d2a26b50ea3ac0d997bdc76cc23bc2e957b208cc0c1d4839d7eaa91d4ead45150bb9431fc92eb54b196c18be593e113125a2e172f93a8f39c556e5307d5881eb2ec19d9e61d4fe7e8109deb670e8e9c811b450120a97d47ffe321a823de6a23beeb9a6596be4ac426b95d5adce5f76e68ae25edb54909abbbc2b7e25eacfa8879a54acd0b13312e794219c767061f6b2776cdd6132fd15a1f11bfc299b81485e651244b82c7332e03ab79dfc3331a37afe9eb5317a156a4543c69645fbdecc9256f15a6f5ff0072e5c11c9626ee3f520014e590124068dcb1bf5c4cbc149d7127a03da8c0257c2206ec5acf0f8ae00db8e4c07409cd2e1177b915ee42b70851507e249dc15ab75d6bde7c8b63de8e46112b0c77c4596db4d2eb5d75eb3d52e3bd40366e1f2",
    //                 "salt": "0edb5309768825398e4aa197f9b554d1e37483c92acf560330e16b1f1ccbb675",
    //                 "clientProof": "700fea96add5cad5bf80a2a1131aa44012c46093c0a19803daab103eeb1eb570",
    //                 "serverProofValid": true
    //             }
    //         },
    //         "dsNonce": {
    //             "id": "dsNonce",
    //             "type": 1,
    //             "parameters": {},
    //             "state": 1,
    //             "messages": {}
    //         }
    //     }
    // },
    // "respondGetPublicKey": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{d8fef61b-58f1-43b2-bbc1-a07ca99cd0dc}",
    //         "channelId": "{d8fef61b-58f1-43b2-bbc1-a07ca99cd0dc}",
    //         "sessionState": 0,
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967462,
    //             "originalUri": {
    //                 "uri": "moz-extension://8ad10c72-9b45-4e90-b620-f47528583032/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "8ad10c72-9b45-4e90-b620-f47528583032",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://8ad10c72-9b45-4e90-b620-f47528583032/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "8ad10c72-9b45-4e90-b620-f47528583032",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionType": 0
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{e270d717-c41b-4201-848c-458efaaf489f}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{e98e0c2d-ff38-4adc-b5c3-521c64bb2e5b}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{9243c56c-568d-43c6-913b-e89837dce3c4}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{77c6572a-7a1b-4ece-9d3b-7eccd3091c30}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{cd8d03cd-9807-4f38-9a75-10e62607729a}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{10ff97cc-00fc-490b-9cc0-4b415529519a}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{386877c6-ca04-44f6-9c0b-5292295878dd}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{f9434dd6-2f06-4887-a8b0-2fad6b8a8b43}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{39689e06-b7ee-4b75-90f8-b7437422e801}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{0a529905-a4b0-476a-8834-7893ed06a402}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{11ff73c4-5d0c-4855-a72c-dd8e39ba1ee1}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{9340d3b1-b1f2-41c9-8fd9-2468d5796222}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         }
    //     ],
    //     "id": "{d8fef61b-58f1-43b2-bbc1-a07ca99cd0dc}",
    //     "context": {
    //         "request": {
    //             "requestId": "{e270d717-c41b-4201-848c-458efaaf489f}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         "tabId": 19
    //     },
    //     "version": 12,
    //     "selectedAccountId": Accounts[0].id,
    //     "challenges": {
    //         "identification": {
    //             "id": "identification",
    //             "type": 0,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetIdentityFields": {
    //                     "name": "GetIdentityFields",
    //                     "payload": [
    //                         "accountId",
    //                         "username",
    //                         "email"
    //                     ]
    //                 }
    //             }
    //         },
    //         "password": {
    //             "id": "password",
    //             "type": 2,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetPasswordFields": {
    //                     "name": "GetPasswordFields",
    //                     "payload": [
    //                         "password"
    //                     ]
    //                 }
    //             }
    //         },
    //         "srp": {
    //             "id": "srp",
    //             "type": 3,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "SelectSecurePasswordField": {
    //                     "name": "SelectSecurePasswordField",
    //                     "payload": "securePassword"
    //                 },
    //                 "ExchangePublicKeys": {
    //                     "name": "ExchangePublicKeys",
    //                     "payload": "17d76f229f95afbdbabb3aca67e43d6950707abeb8e8b480357002a6221736f799d9ec943b59a48a52644019302fa17407ac5065388628f0b5096db96df53318ad2cd3d42e16cad38481fb8feebc57bb2a5820a530099520edbb241b5370a9ba8de1d41f0f0a390e3bf78bc570894d3988b8bfbade6be864e5dbaf0b152f012c5b1c022a75983d395697b80a605c0dd63eec15c8f7fd5f26dc7b77383bf6e3b6a59935b1c18b548aadd183f285eb43e13599d54b42733d0f773d87526afbc495bbf4cb8f291963a01c52d49f1e25382af9d9d1e7366e44d1724ed2d2a26b50ea3ac0d997bdc76cc23bc2e957b208cc0c1d4839d7eaa91d4ead45150bb9431fc92eb54b196c18be593e113125a2e172f93a8f39c556e5307d5881eb2ec19d9e61d4fe7e8109deb670e8e9c811b450120a97d47ffe321a823de6a23beeb9a6596be4ac426b95d5adce5f76e68ae25edb54909abbbc2b7e25eacfa8879a54acd0b13312e794219c767061f6b2776cdd6132fd15a1f11bfc299b81485e651244b82c7332e03ab79dfc3331a37afe9eb5317a156a4543c69645fbdecc9256f15a6f5ff0072e5c11c9626ee3f520014e590124068dcb1bf5c4cbc149d7127a03da8c0257c2206ec5acf0f8ae00db8e4c07409cd2e1177b915ee42b70851507e249dc15ab75d6bde7c8b63de8e46112b0c77c4596db4d2eb5d75eb3d52e3bd40366e1f2"
    //                 },
    //                 "ComputeClientProof": {
    //                     "name": "ComputeClientProof",
    //                     "payload": "0edb5309768825398e4aa197f9b554d1e37483c92acf560330e16b1f1ccbb675"
    //                 },
    //                 "VerifyServerProof": {
    //                     "name": "VerifyServerProof",
    //                     "payload": "29c79283ec666731fd01ef2df65a6f8f3e435a8f08fd2d3e44c4dae1e1c63f11"
    //                 }
    //             },
    //             "srpState": {
    //                 "fields": {
    //                     "username": "bob.bobby@example.org",
    //                     "password": "4e5k-6kc4-crmz-70kv"
    //                 },
    //                 "clientPrivateKeyHexa": "efd42c1c9f8511df093bb539bdf864c1acd9f2c444b25598facea59a87281d44",
    //                 "clientPublicKeyHexA": "5ee7c09e1f90070b81c8023bd795ba30fe1253382691caf6f416a3a3f1d56d82a63239d674bdd67b7b03731bc9d38b44afe1dd9109e8d9ddb8f146d6e41c13d606871c1c2edadc9ec55d785d8bdbd062b5c6827ef202dfbcab23df34aefa86e69bfe96d5d4765c6545ce379ee1ebea78fcef93d86b26dbd90aaf50efd5ed890ac3ae8e6d2ba2e96ae5d6667b44dd0a5528f28c5ea6a99bbd566635cce7c45cad2aba9e5414bae7b7b4871d4f107dbe0a525600c883aa709a283979fca50f4b111d702f0cf4c76c118fc7d55de0dd3322ee41492e90839ecf7524feb13a2b58c4b6e620ef59d13736fbc8954870509ea818433105ce4221c6352046000986214f95d9d3194fd7dd6b9389f0b39cc4651a643a1996e380476d58f583aa5d64cc9643dd028519d04245327787508872907124a685cd294899d3441316bad2c6399b4285e145eab9ebf16701e6ba275defb55ee19b187f6f0f09a3acd96ca90efc53bcc59cd3a385e4cace9d4e801fd1d2c4b0981a0bbda5336c8c961eec0c0a18f59305f0062a06db939e206408bd74d53d76e3f87e0327288c7262cc898203329d665ab908cb306ba13d3095aa9cf3a3295dbb59c9b26ef90b7d19f8acbc386d1c721c73deaf34590ef9d90e2f3c49bd0c6f4c9e87e37b369f0a75845eea0a5599e6ffa61bd037a4ff01d66d9c180d4adc2e84490ce5135d31e689ad055e29eb1b",
    //                 "serverPublicKeyHexB": "17d76f229f95afbdbabb3aca67e43d6950707abeb8e8b480357002a6221736f799d9ec943b59a48a52644019302fa17407ac5065388628f0b5096db96df53318ad2cd3d42e16cad38481fb8feebc57bb2a5820a530099520edbb241b5370a9ba8de1d41f0f0a390e3bf78bc570894d3988b8bfbade6be864e5dbaf0b152f012c5b1c022a75983d395697b80a605c0dd63eec15c8f7fd5f26dc7b77383bf6e3b6a59935b1c18b548aadd183f285eb43e13599d54b42733d0f773d87526afbc495bbf4cb8f291963a01c52d49f1e25382af9d9d1e7366e44d1724ed2d2a26b50ea3ac0d997bdc76cc23bc2e957b208cc0c1d4839d7eaa91d4ead45150bb9431fc92eb54b196c18be593e113125a2e172f93a8f39c556e5307d5881eb2ec19d9e61d4fe7e8109deb670e8e9c811b450120a97d47ffe321a823de6a23beeb9a6596be4ac426b95d5adce5f76e68ae25edb54909abbbc2b7e25eacfa8879a54acd0b13312e794219c767061f6b2776cdd6132fd15a1f11bfc299b81485e651244b82c7332e03ab79dfc3331a37afe9eb5317a156a4543c69645fbdecc9256f15a6f5ff0072e5c11c9626ee3f520014e590124068dcb1bf5c4cbc149d7127a03da8c0257c2206ec5acf0f8ae00db8e4c07409cd2e1177b915ee42b70851507e249dc15ab75d6bde7c8b63de8e46112b0c77c4596db4d2eb5d75eb3d52e3bd40366e1f2",
    //                 "salt": "0edb5309768825398e4aa197f9b554d1e37483c92acf560330e16b1f1ccbb675",
    //                 "clientProof": "700fea96add5cad5bf80a2a1131aa44012c46093c0a19803daab103eeb1eb570",
    //                 "serverProofValid": true
    //             }
    //         },
    //         "dsNonce": {
    //             "id": "dsNonce",
    //             "type": 1,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetPublicKey": {
    //                     "name": "GetPublicKey",
    //                     "payload": "clientKey"
    //                 }
    //             }
    //         }
    //     }
    // },
    // "respondToSignNonce": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{749c0f97-cd7f-4d15-b046-b788afb382f3}",
    //         "channelId": "{749c0f97-cd7f-4d15-b046-b788afb382f3}",
    //         "sessionState": 0,
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967562,
    //             "originalUri": {
    //                 "uri": "moz-extension://a4c19875-b566-4922-a510-736de585591d/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "a4c19875-b566-4922-a510-736de585591d",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://a4c19875-b566-4922-a510-736de585591d/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "a4c19875-b566-4922-a510-736de585591d",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionType": 0
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{10c51a81-69fa-474c-87db-6a3d567851a5}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{20999e83-a3cf-4252-b43e-248114496e56}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{7b672a11-cdec-423f-905a-00de44153ab6}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{9dcd0580-c030-4e48-8a3c-7a7a194dcd72}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{ed6dcced-0c2e-4795-ac55-ffba21d59940}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{6c9b24bd-0baa-4656-b7d7-ff5cf39206df}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{b4933033-e311-4e42-9cdf-e1383ed02dd3}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{bf07b48c-6e6b-437f-a796-aa2de88441ac}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{2b6e1769-8795-405a-892c-bc5effd46bb6}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{39c2a3c8-9d8b-4079-99e1-ea6079a44565}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{42b1a0c1-7474-430d-95ab-fe476ea813d9}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{06ff6b73-f881-42ce-963a-ee892b8e9d62}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         }
    //     ],
    //     "id": "{749c0f97-cd7f-4d15-b046-b788afb382f3}" as ERequestType,
    //     "context": {
    //         "request": {
    //             "requestId": "{10c51a81-69fa-474c-87db-6a3d567851a5}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         "tabId": 34
    //     },
    //     "version": 12,
    //     "selectedAccountId": "id6f-5cf1-kjey-999q",
    //     "challenges": {
    //         "identification": {
    //             "id": "identification",
    //             "type": 0,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetIdentityFields": {
    //                     "name": "GetIdentityFields",
    //                     "payload": [
    //                         "accountId",
    //                         "username",
    //                         "email"
    //                     ]
    //                 }
    //             }
    //         },
    //         "password": {
    //             "id": "password",
    //             "type": 2,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetPasswordFields": {
    //                     "name": "GetPasswordFields",
    //                     "payload": [
    //                         "password"
    //                     ]
    //                 }
    //             }
    //         },
    //         "srp": {
    //             "id": "srp",
    //             "type": 3,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "SelectSecurePasswordField": {
    //                     "name": "SelectSecurePasswordField",
    //                     "payload": "securePassword"
    //                 },
    //                 "ExchangePublicKeys": {
    //                     "name": "ExchangePublicKeys",
    //                     "payload": "90dc495beb61e5c4788d0388fadab4e21c18134eb1152f4c1cd7e5ea855de07e197b9d602221203b5474819d7ff6961d5dd81fd8ae190a72ac428f86e9d625ff9da706045fdfad1f7ac70e54d66bc9375698dae796eb9e8a6e81204780b6eb2b156aabaeb0dae6a60fd101d7d6cae1edfb41480f03135fe804a4ace2c0d3fbeb409b1fb89a2dbe76b4ce69a8644774d62e646d5342bfead5595d39eb46d1e8c79806ee3a36ba06af59ea840992a9454dde0728884f68a37ca7e0f21ee82e27a81373e2e6f3bd63a9901fa0f1ae55235d577efc6de5ba69c765bdd01258f68c9f594627f5b85eaaae3b9f2545343bbb26ccbef34ee1459d306f9464f1ead07385cdcf1c64c9f6eb422e54c9f97ceebe8b0c8880371dfbc7f2dd96444b32c02967be7e996596812b175fc764498b3136a411423829eaaddb8124c633badc1cb6758a3e675a15079bb2574d93873305530999722b271cb2aa9410b7b186c740461c4a6aea1dce37795eda32097e667c2f5c9bdd009a8960aeebaaec3c8ed5905c493e264a97f00790b8818f78a443b7b15534d7ec1d9ba39edb96ff3e03aa79e06bd84e47a85d157db1602d9383e847ce87098f60523a6fb6e7ecb83d8086eebffda137190129b6159a8ea8c65b48bc19177060cda090d1a65622bb710a1404c7338bc19d847bd296e80441e4844ebd55e606a70856d496fc2195c22cae66fda623"
    //                 },
    //                 "ComputeClientProof": {
    //                     "name": "ComputeClientProof",
    //                     "payload": "ee869c6f5ec266476de9a9918ef349a152f1cc5f265a9ad391873e8f6a1f30a3"
    //                 }
    //             },
    //             "srpState": {
    //                 "fields": {
    //                     "username": "bob.bobby@example.org",
    //                     "password": "6yw3-bnf5-0ojp-shll"
    //                 },
    //                 "clientPrivateKeyHexa": "43ff59f82526ad1e54f80ad81ebdb08a74a74333a9cce1066c8ea004e486beec",
    //                 "clientPublicKeyHexA": "8afb5cd263c0956031d9fd651e3acd6554acc8f37579dc11ad2e2ad5823e60502515dc71e3e9edc2bdcdfb745ada8891c2e1e3d05a965433942159fad06836be603258adabfa62020380a8a975ee354e6a78254620fc5dd9dafe565e6837bf2b11243af040ad3e21a5ceb94fd9759d59a70c996ff84a16f2b8a7f88038e56bddc63af6d5b45807969407e4e04b02323a6edd0300f2b2c2f0d8bda2b4ce36461375d8bbe31ddb97094a5f5dc5a287b7b8b7d02a97f27aee0d48c2b8bc614d911289b1d646d19ccdf328869bdb1ebe86aad9654cde821dcbfb388eeea8ac1939fa7540f156c780f1375c8985d6d37ded8a760b249f4f665668428064e42fb9f1066b961313313346dd57c87edcebe3934bc8de797fd2d1d4aad5195ec487279d27a997391b593f7b8c166586d46aa55d23a5bf719cfa2a758af7097b2eaba3a804270ab7975fb86f47da9e7bb90e426d95ad71b3e183e59a63548e2dc99da60e48eac4ea93bf5681d6be6d0a2ebae31b6462bdbf9b982055968ccfff2fb752141ba2a087fc154265d615c13de6026eee1a7984373bdffa807a6cfaa27f340554c8d92b6b2f0c18a4da9f3d441974cedd5d94ea92ed8584488b1b78321f2a7440710a92860bea9df3efb1bf883b1f78d9fa199517fd9ff75948d3d3d44311b20b7c72d961440d0c07d8e2a6093d778f57d9dc438c415f9c6e0060f82f385618ca3f",
    //                 "serverPublicKeyHexB": "90dc495beb61e5c4788d0388fadab4e21c18134eb1152f4c1cd7e5ea855de07e197b9d602221203b5474819d7ff6961d5dd81fd8ae190a72ac428f86e9d625ff9da706045fdfad1f7ac70e54d66bc9375698dae796eb9e8a6e81204780b6eb2b156aabaeb0dae6a60fd101d7d6cae1edfb41480f03135fe804a4ace2c0d3fbeb409b1fb89a2dbe76b4ce69a8644774d62e646d5342bfead5595d39eb46d1e8c79806ee3a36ba06af59ea840992a9454dde0728884f68a37ca7e0f21ee82e27a81373e2e6f3bd63a9901fa0f1ae55235d577efc6de5ba69c765bdd01258f68c9f594627f5b85eaaae3b9f2545343bbb26ccbef34ee1459d306f9464f1ead07385cdcf1c64c9f6eb422e54c9f97ceebe8b0c8880371dfbc7f2dd96444b32c02967be7e996596812b175fc764498b3136a411423829eaaddb8124c633badc1cb6758a3e675a15079bb2574d93873305530999722b271cb2aa9410b7b186c740461c4a6aea1dce37795eda32097e667c2f5c9bdd009a8960aeebaaec3c8ed5905c493e264a97f00790b8818f78a443b7b15534d7ec1d9ba39edb96ff3e03aa79e06bd84e47a85d157db1602d9383e847ce87098f60523a6fb6e7ecb83d8086eebffda137190129b6159a8ea8c65b48bc19177060cda090d1a65622bb710a1404c7338bc19d847bd296e80441e4844ebd55e606a70856d496fc2195c22cae66fda623",
    //                 "salt": "ee869c6f5ec266476de9a9918ef349a152f1cc5f265a9ad391873e8f6a1f30a3",
    //                 "clientProof": "fdc8b8fdb603c6c462ecc84aa1e3432cb668187470e6215a67fc849242ef5762"
    //             },
    //             "abortionReasonCode": "SRP:BadClientProof"
    //         },
    //         "dsNonce": {
    //             "id": "dsNonce",
    //             "type": 1,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetPublicKey": {
    //                     "name": "GetPublicKey",
    //                     "payload": "clientKey"
    //                 },
    //                 "SignNonce": {
    //                     "name": "SignNonce",
    //                     "payload": "AAECAwQA"
    //                 }
    //             },
    //             "dsState": {
    //                 "privateKey": "-----BEGIN PRIVATE KEY-----\n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\nxqvH01R8VHeTvK6YU0ia7wQJsChLONAa\n-----END PRIVATE KEY-----"
    //             }
    //         }
    //     }
    // },
    // "approveChallengeOtp": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{60c7e208-b724-4cdb-a211-49eb3833dbd6}",
    //         "channelId": "{60c7e208-b724-4cdb-a211-49eb3833dbd6}",
    //         "sessionState": 0,
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967598,
    //             "currentUri": {
    //                 "uri": "moz-extension://a4c19875-b566-4922-a510-736de585591d/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "a4c19875-b566-4922-a510-736de585591d",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "originalUri": {
    //                 "uri": "moz-extension://a4c19875-b566-4922-a510-736de585591d/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "a4c19875-b566-4922-a510-736de585591d",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionType": 0
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{c23b5f4b-cb9e-4349-b09f-fed324fc4cc9}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{e5f36690-3503-4d7f-a885-07cc930a985a}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{87f78941-53cf-4183-be55-f49867927598}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{3670e2e3-8a22-4538-8cf6-16518c59cde9}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{f09b5e92-afba-4a5b-a8b1-c29574e5fd33}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{d7c604d0-456c-45e0-9225-9e9480eaaf31}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{4b41e5c4-a46d-4e46-8316-53fbe5b51171}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{63c15231-3a5a-47ff-aa8b-b0e4d4fe03e4}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{43a0525d-b5af-414c-9ad8-68194dc4804f}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{24b38b1a-d317-4a5f-a06e-d3fb9bc9f3fe}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{af3aa456-2f14-4e28-851d-1a410b45f15c}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{466eb843-21c6-4608-a9e1-48af2a64d06a}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{63fd1449-02a1-4421-a3b9-a734ec061b70}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         }
    //     ],
    //     "id": "{60c7e208-b724-4cdb-a211-49eb3833dbd6}",
    //     "context": {
    //         "request": {
    //             "requestId": "{c23b5f4b-cb9e-4349-b09f-fed324fc4cc9}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         "tabId": 41
    //     },
    //     "version": 13,
    //     "selectedAccountId": Accounts[0].id,
    //     "challenges": {
    //         "identification": {
    //             "id": "identification",
    //             "type": 0,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetIdentityFields": {
    //                     "name": "GetIdentityFields",
    //                     "payload": [
    //                         "accountId",
    //                         "username",
    //                         "email"
    //                     ]
    //                 }
    //             }
    //         },
    //         "password": {
    //             "id": "password",
    //             "type": 2,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetPasswordFields": {
    //                     "name": "GetPasswordFields",
    //                     "payload": [
    //                         "password"
    //                     ]
    //                 }
    //             }
    //         },
    //         "srp": {
    //             "id": "srp",
    //             "type": 3,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "SelectSecurePasswordField": {
    //                     "name": "SelectSecurePasswordField",
    //                     "payload": "securePassword"
    //                 },
    //                 "ExchangePublicKeys": {
    //                     "name": "ExchangePublicKeys",
    //                     "payload": "cf51e7c69c5be92c2670defc7eec5574f95adb92075fa19c311c86a4df5309ec7552b46b83ef916c1d474eb15f836fa1497a88d0a3d298780d2ad37038a4626d33f5847e079135c4bb6d77450b94ac3b04db8927859aa926543532ff9084370333e2590ff3714c5cd720a1f3799d34fb424970dd4aba8cd5ca3a26a010a6ef3726e9e12aeceb86ee195a964655e11226d2c2d6b9e0f92ac5f1fdb361c57be8d62045365b6154ff4a410e289747866287e5d7b93370655f1945c16161e47a0f6600d38f6edde77813da7e2b7479d067b56ba357c83c649c17fe09e87bf0886a1ae7128f037d0e790d35b3eec0aca5554e93e1609322d1c45539c1af8be0f46ed224e46bd31f973981e63a56fa76a6c443790e97b58a9c7ff57049863db3b9990bfc0108b47d07f074c51bb666271533a94802f29dd61db0625522000df7a9eeb009e76a4b5428310b0c27e4736b8fb6a3132bbccf66bb8b35bc1dcb85d9b73e220871bcc2e28bf2ce13ff6f764e9b58795044b81e778b2b11d3a75744f27ab037485c7c1173cf668234c4e88d60732d7b3c24769c7d811cf5b6ca36493eaa1d69e5505999ed5efa4fd7f5e6fda8ff87fec7a94ea593c21aeeb76592883e63e2ca3a7c4fb1fdf6ad9e60074eb67fbed049a92e654239797021ca7838382539676f51104c6cb9f023501275264a4e6adc39171be810266f23edd4d1a9b8cda58213"
    //                 },
    //                 "ComputeClientProof": {
    //                     "name": "ComputeClientProof",
    //                     "payload": "9c101194d7e1bde071e038ac77648962e19ae3f3e21d76ccc1db4de45fe7ead5"
    //                 }
    //             },
    //             "srpState": {
    //                 "fields": {
    //                     "username": "bob.bobby@example.org",
    //                     "password": "6yw3-bnf5-0ojp-shll"
    //                 },
    //                 "clientPrivateKeyHexa": "b823146a842976c91019393d4085ddd2a665e174e1ba51bac7d6f8b2d621d78d",
    //                 "clientPublicKeyHexA": "3bed5461bd9bfbe3f3342e8530b2bd605386b55626e493406bb051a1406b931357cd86150beb3e2c5d344894e48ca773675df63bc542f9afd2d7e59191b43f13d981cde9862723daab765298c0495d8a27716cf17c442ed0a53b1bf0f6a0f3af04716156ee7393cf1365998525269a95e528cee0ae8e3fab26c5aea9b867a03c91833bc50e512addc467657f18a1fdbcb8a098b62eb75bd91dc4d9e93c897b47296d7149b508018763d74149e02f28764a819002a4837d57a9b64b55545836b13f5c9f7881a5467d08988de0681951c76fbb0c5073416b7af5765bd935370313c0f706151c5de56e86d435d2969a0d69e01c2f399d84cb4c2b14ef221bb97b68885f821f52f5818c8e29a2380df829c76b0e93e61aace7a9be4802d9829af3a1b4d1e56e5c19909b4749b5a3406f2575ece7a5efe81a4e87c8e21ed033820a05c2c3e9951e077dd2f768e79ba200b9dae54ad7dd8f7c4aff7d6ed56f25d993c94b7a21e7bd6096c2c971d078a638f8a2e5263427b1d61a01824ba5fbfbb65cbd78bd8e8092f42c06e04fb97461f14739626810179920f883b508809da40405d330513becd68ce4dd0bb52f52386b436bd87bd131d9c1fad6dab5fd3cd64a1e06eaabe85992d9f088bc795bd9d081e10113233c33d3d6758480e740b0cc4bc89d4a692c82762f9587e222430165837313aa1030080d476f8338cb71883c9efc5b",
    //                 "serverPublicKeyHexB": "cf51e7c69c5be92c2670defc7eec5574f95adb92075fa19c311c86a4df5309ec7552b46b83ef916c1d474eb15f836fa1497a88d0a3d298780d2ad37038a4626d33f5847e079135c4bb6d77450b94ac3b04db8927859aa926543532ff9084370333e2590ff3714c5cd720a1f3799d34fb424970dd4aba8cd5ca3a26a010a6ef3726e9e12aeceb86ee195a964655e11226d2c2d6b9e0f92ac5f1fdb361c57be8d62045365b6154ff4a410e289747866287e5d7b93370655f1945c16161e47a0f6600d38f6edde77813da7e2b7479d067b56ba357c83c649c17fe09e87bf0886a1ae7128f037d0e790d35b3eec0aca5554e93e1609322d1c45539c1af8be0f46ed224e46bd31f973981e63a56fa76a6c443790e97b58a9c7ff57049863db3b9990bfc0108b47d07f074c51bb666271533a94802f29dd61db0625522000df7a9eeb009e76a4b5428310b0c27e4736b8fb6a3132bbccf66bb8b35bc1dcb85d9b73e220871bcc2e28bf2ce13ff6f764e9b58795044b81e778b2b11d3a75744f27ab037485c7c1173cf668234c4e88d60732d7b3c24769c7d811cf5b6ca36493eaa1d69e5505999ed5efa4fd7f5e6fda8ff87fec7a94ea593c21aeeb76592883e63e2ca3a7c4fb1fdf6ad9e60074eb67fbed049a92e654239797021ca7838382539676f51104c6cb9f023501275264a4e6adc39171be810266f23edd4d1a9b8cda58213",
    //                 "salt": "9c101194d7e1bde071e038ac77648962e19ae3f3e21d76ccc1db4de45fe7ead5",
    //                 "clientProof": "537ae003ad62ec632bf9647caba1eb17b29349c2a184e7ba046589367735d54a"
    //             },
    //             "abortionReasonCode": "SRP:BadClientProof"
    //         },
    //         "dsNonce": {
    //             "id": "dsNonce",
    //             "type": 1,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetPublicKey": {
    //                     "name": "GetPublicKey",
    //                     "payload": "clientKey"
    //                 },
    //                 "SignNonce": {
    //                     "name": "SignNonce",
    //                     "payload": "AAECAwQA"
    //                 }
    //             },
    //             "dsState": {
    //                 "privateKey": "-----BEGIN PRIVATE KEY-----\n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\nxqvH01R8VHeTvK6YU0ia7wQJsChLONAa\n-----END PRIVATE KEY-----",
    //                 "signedMessage": "-----BEGIN CUSTOM SIGNATURE FORMAT -----\nQ2mKOiIp34sYdD7obZrGLmU8BtOkOUBSR6mvkXUOURYnJVlFJKmu7HX08nj/KfAS\nyIBj3L6uQiVP6TS81epFes+vsCIJjsiY3tOHVXQokyzcIzK/qbVzfJCoqsL6Mraw\nDzZZ/rKU5UCDkEgQHRwioZOHJm1LP9VK4LNLDkBdy7cts3Lhujzw16GHDnTsa0Ot\n51XelV1nBeVd9qBbUAO1SLqQ7uJcVgEbOwvxPqvX1dCs1xxBGSA6TYlqgzd4Idpo\ne+V+hBGaFNX6A+QXSE9q5JL09XX6Fz9tDddKtEhMjQK0Un2gHJ5cPSiuZ50ti2Kx\nEcbRkhP9FBnvZHHyjJeTqe9iXF3bQ3lbMr3FiTOyVI7E7ZGeXtUWforX6Gs53r5Q\nEPdWtZQXar5QgrRq7dQuCJZZ0XeeVHYMP7qL2y7n3YT37D6ZQDLNeTpwTSyqPXgS\n53CPWPLYS1OPoGBSI8PJ6YJ1lq+SKM2ody+16YOc8WUfH34qE346WrLKMUAao1ux\nWKS68SpTDmOEdnmmjVeUI2Skak5d+O0XmQZCTqt8mx28XkpZa24TRw1XP+7PXR5T\nM7w31BG9ry1MfwdwxpG/2Zi/s6HsbMz3TG63dYef/ZqmEbPgeLJXBX8rItpM7wMB\nN+zz2F820PNpY6kIdCtYEUJbJIU4thCyCGjhYiRfTp0=\n----END CUSTOM SIGNATURE FORMAT -----",
    //                 "nonce": "AAECAwQA"
    //             }
    //         },
    //         "emailOtp": {
    //             "id": "emailOtp",
    //             "type": 4,
    //             "parameters": {},
    //             "state": 1,
    //             "messages": {}
    //         }
    //     }
    // },
    // "respondToGetOtp": {
    //     "metadata": {
    //         [EMetadataProperty.Category]: "",
    //         [EMetadataProperty.Version]: 0,
    //         [EMetadataProperty.Status]: 0,
    //         [EMetadataProperty.ChangePassUrl]: "",
    //     },
    //     "sessionInfo": {
    //         "sessionId": "{60c7e208-b724-4cdb-a211-49eb3833dbd6}",
    //         "channelId": "{60c7e208-b724-4cdb-a211-49eb3833dbd6}",
    //         "sessionState": 0,
    //         "extensionId": "secretstsar@bobby",
    //         "pageContext": {
    //             "innerWindowId": 4294967598,
    //             "originalUri": {
    //                 "uri": "moz-extension://a4c19875-b566-4922-a510-736de585591d/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "a4c19875-b566-4922-a510-736de585591d",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             },
    //             "currentUri": {
    //                 "uri": "moz-extension://a4c19875-b566-4922-a510-736de585591d/build/index.html#/window/sample",
    //                 "scheme": "moz-extension",
    //                 "hostname": "a4c19875-b566-4922-a510-736de585591d",
    //                 "port": -1,
    //                 "path": "/build/index.html"
    //             }
    //         },
    //         "appSessionOptions": {
    //             "appCheckpointCapability": 2,
    //             "channelConstraints": undefined,
    //             "webAppActor": undefined
    //         },
    //         "sessionType": 0
    //     },
    //     "requests": [
    //         {
    //             "requestId": "{c23b5f4b-cb9e-4349-b09f-fed324fc4cc9}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{e5f36690-3503-4d7f-a885-07cc930a985a}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{87f78941-53cf-4183-be55-f49867927598}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{3670e2e3-8a22-4538-8cf6-16518c59cde9}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{f09b5e92-afba-4a5b-a8b1-c29574e5fd33}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{d7c604d0-456c-45e0-9225-9e9480eaaf31}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{4b41e5c4-a46d-4e46-8316-53fbe5b51171}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{63c15231-3a5a-47ff-aa8b-b0e4d4fe03e4}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{43a0525d-b5af-414c-9ad8-68194dc4804f}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{24b38b1a-d317-4a5f-a06e-d3fb9bc9f3fe}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{af3aa456-2f14-4e28-851d-1a410b45f15c}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{466eb843-21c6-4608-a9e1-48af2a64d06a}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         },
    //         {
    //             "requestId": "{63fd1449-02a1-4421-a3b9-a734ec061b70}",
    //             "requestType": "ApproveChallengeRequest" as ERequestType
    //         },
    //         {
    //             "requestId": "{6a50d88d-38ea-4341-b941-6179e57df1b4}",
    //             "requestType": "RespondToChallengeMessage" as ERequestType
    //         }
    //     ],
    //     "id": "{60c7e208-b724-4cdb-a211-49eb3833dbd6}",
    //     "context": {
    //         "request": {
    //             "requestId": "{c23b5f4b-cb9e-4349-b09f-fed324fc4cc9}",
    //             "requestType": "ApproveOnboardingRequest" as ERequestType
    //         },
    //         "tabId": 41
    //     },
    //     "version": 14,
    //     "selectedAccountId": Accounts[0].id,
    //     "challenges": {
    //         "identification": {
    //             "id": "identification",
    //             "type": 0,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetIdentityFields": {
    //                     "name": "GetIdentityFields",
    //                     "payload": [
    //                         "accountId",
    //                         "username",
    //                         "email"
    //                     ]
    //                 }
    //             }
    //         },
    //         "password": {
    //             "id": "password",
    //             "type": 2,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetPasswordFields": {
    //                     "name": "GetPasswordFields",
    //                     "payload": [
    //                         "password"
    //                     ]
    //                 }
    //             }
    //         },
    //         "srp": {
    //             "id": "srp",
    //             "type": 3,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "SelectSecurePasswordField": {
    //                     "name": "SelectSecurePasswordField",
    //                     "payload": "securePassword"
    //                 },
    //                 "ExchangePublicKeys": {
    //                     "name": "ExchangePublicKeys",
    //                     "payload": "cf51e7c69c5be92c2670defc7eec5574f95adb92075fa19c311c86a4df5309ec7552b46b83ef916c1d474eb15f836fa1497a88d0a3d298780d2ad37038a4626d33f5847e079135c4bb6d77450b94ac3b04db8927859aa926543532ff9084370333e2590ff3714c5cd720a1f3799d34fb424970dd4aba8cd5ca3a26a010a6ef3726e9e12aeceb86ee195a964655e11226d2c2d6b9e0f92ac5f1fdb361c57be8d62045365b6154ff4a410e289747866287e5d7b93370655f1945c16161e47a0f6600d38f6edde77813da7e2b7479d067b56ba357c83c649c17fe09e87bf0886a1ae7128f037d0e790d35b3eec0aca5554e93e1609322d1c45539c1af8be0f46ed224e46bd31f973981e63a56fa76a6c443790e97b58a9c7ff57049863db3b9990bfc0108b47d07f074c51bb666271533a94802f29dd61db0625522000df7a9eeb009e76a4b5428310b0c27e4736b8fb6a3132bbccf66bb8b35bc1dcb85d9b73e220871bcc2e28bf2ce13ff6f764e9b58795044b81e778b2b11d3a75744f27ab037485c7c1173cf668234c4e88d60732d7b3c24769c7d811cf5b6ca36493eaa1d69e5505999ed5efa4fd7f5e6fda8ff87fec7a94ea593c21aeeb76592883e63e2ca3a7c4fb1fdf6ad9e60074eb67fbed049a92e654239797021ca7838382539676f51104c6cb9f023501275264a4e6adc39171be810266f23edd4d1a9b8cda58213"
    //                 },
    //                 "ComputeClientProof": {
    //                     "name": "ComputeClientProof",
    //                     "payload": "9c101194d7e1bde071e038ac77648962e19ae3f3e21d76ccc1db4de45fe7ead5"
    //                 }
    //             },
    //             "srpState": {
    //                 "fields": {
    //                     "username": "bob.bobby@example.org",
    //                     "password": "6yw3-bnf5-0ojp-shll"
    //                 },
    //                 "clientPrivateKeyHexa": "b823146a842976c91019393d4085ddd2a665e174e1ba51bac7d6f8b2d621d78d",
    //                 "clientPublicKeyHexA": "3bed5461bd9bfbe3f3342e8530b2bd605386b55626e493406bb051a1406b931357cd86150beb3e2c5d344894e48ca773675df63bc542f9afd2d7e59191b43f13d981cde9862723daab765298c0495d8a27716cf17c442ed0a53b1bf0f6a0f3af04716156ee7393cf1365998525269a95e528cee0ae8e3fab26c5aea9b867a03c91833bc50e512addc467657f18a1fdbcb8a098b62eb75bd91dc4d9e93c897b47296d7149b508018763d74149e02f28764a819002a4837d57a9b64b55545836b13f5c9f7881a5467d08988de0681951c76fbb0c5073416b7af5765bd935370313c0f706151c5de56e86d435d2969a0d69e01c2f399d84cb4c2b14ef221bb97b68885f821f52f5818c8e29a2380df829c76b0e93e61aace7a9be4802d9829af3a1b4d1e56e5c19909b4749b5a3406f2575ece7a5efe81a4e87c8e21ed033820a05c2c3e9951e077dd2f768e79ba200b9dae54ad7dd8f7c4aff7d6ed56f25d993c94b7a21e7bd6096c2c971d078a638f8a2e5263427b1d61a01824ba5fbfbb65cbd78bd8e8092f42c06e04fb97461f14739626810179920f883b508809da40405d330513becd68ce4dd0bb52f52386b436bd87bd131d9c1fad6dab5fd3cd64a1e06eaabe85992d9f088bc795bd9d081e10113233c33d3d6758480e740b0cc4bc89d4a692c82762f9587e222430165837313aa1030080d476f8338cb71883c9efc5b",
    //                 "serverPublicKeyHexB": "cf51e7c69c5be92c2670defc7eec5574f95adb92075fa19c311c86a4df5309ec7552b46b83ef916c1d474eb15f836fa1497a88d0a3d298780d2ad37038a4626d33f5847e079135c4bb6d77450b94ac3b04db8927859aa926543532ff9084370333e2590ff3714c5cd720a1f3799d34fb424970dd4aba8cd5ca3a26a010a6ef3726e9e12aeceb86ee195a964655e11226d2c2d6b9e0f92ac5f1fdb361c57be8d62045365b6154ff4a410e289747866287e5d7b93370655f1945c16161e47a0f6600d38f6edde77813da7e2b7479d067b56ba357c83c649c17fe09e87bf0886a1ae7128f037d0e790d35b3eec0aca5554e93e1609322d1c45539c1af8be0f46ed224e46bd31f973981e63a56fa76a6c443790e97b58a9c7ff57049863db3b9990bfc0108b47d07f074c51bb666271533a94802f29dd61db0625522000df7a9eeb009e76a4b5428310b0c27e4736b8fb6a3132bbccf66bb8b35bc1dcb85d9b73e220871bcc2e28bf2ce13ff6f764e9b58795044b81e778b2b11d3a75744f27ab037485c7c1173cf668234c4e88d60732d7b3c24769c7d811cf5b6ca36493eaa1d69e5505999ed5efa4fd7f5e6fda8ff87fec7a94ea593c21aeeb76592883e63e2ca3a7c4fb1fdf6ad9e60074eb67fbed049a92e654239797021ca7838382539676f51104c6cb9f023501275264a4e6adc39171be810266f23edd4d1a9b8cda58213",
    //                 "salt": "9c101194d7e1bde071e038ac77648962e19ae3f3e21d76ccc1db4de45fe7ead5",
    //                 "clientProof": "537ae003ad62ec632bf9647caba1eb17b29349c2a184e7ba046589367735d54a"
    //             },
    //             "abortionReasonCode": "SRP:BadClientProof"
    //         },
    //         "dsNonce": {
    //             "id": "dsNonce",
    //             "type": 1,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetPublicKey": {
    //                     "name": "GetPublicKey",
    //                     "payload": "clientKey"
    //                 },
    //                 "SignNonce": {
    //                     "name": "SignNonce",
    //                     "payload": "AAECAwQA"
    //                 }
    //             },
    //             "dsState": {
    //                 "privateKey": "-----BEGIN PRIVATE KEY-----\n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\nxqvH01R8VHeTvK6YU0ia7wQJsChLONAa\n-----END PRIVATE KEY-----",
    //                 "signedMessage": "-----BEGIN CUSTOM SIGNATURE FORMAT -----\nQ2mKOiIp34sYdD7obZrGLmU8BtOkOUBSR6mvkXUOURYnJVlFJKmu7HX08nj/KfAS\nyIBj3L6uQiVP6TS81epFes+vsCIJjsiY3tOHVXQokyzcIzK/qbVzfJCoqsL6Mraw\nDzZZ/rKU5UCDkEgQHRwioZOHJm1LP9VK4LNLDkBdy7cts3Lhujzw16GHDnTsa0Ot\n51XelV1nBeVd9qBbUAO1SLqQ7uJcVgEbOwvxPqvX1dCs1xxBGSA6TYlqgzd4Idpo\ne+V+hBGaFNX6A+QXSE9q5JL09XX6Fz9tDddKtEhMjQK0Un2gHJ5cPSiuZ50ti2Kx\nEcbRkhP9FBnvZHHyjJeTqe9iXF3bQ3lbMr3FiTOyVI7E7ZGeXtUWforX6Gs53r5Q\nEPdWtZQXar5QgrRq7dQuCJZZ0XeeVHYMP7qL2y7n3YT37D6ZQDLNeTpwTSyqPXgS\n53CPWPLYS1OPoGBSI8PJ6YJ1lq+SKM2ody+16YOc8WUfH34qE346WrLKMUAao1ux\nWKS68SpTDmOEdnmmjVeUI2Skak5d+O0XmQZCTqt8mx28XkpZa24TRw1XP+7PXR5T\nM7w31BG9ry1MfwdwxpG/2Zi/s6HsbMz3TG63dYef/ZqmEbPgeLJXBX8rItpM7wMB\nN+zz2F820PNpY6kIdCtYEUJbJIU4thCyCGjhYiRfTp0=\n----END CUSTOM SIGNATURE FORMAT -----",
    //                 "nonce": "AAECAwQA"
    //             }
    //         },
    //         "emailOtp": {
    //             "id": "emailOtp",
    //             "type": 4,
    //             "parameters": {},
    //             "state": 2,
    //             "messages": {
    //                 "GetOtp": {
    //                     "name": "GetOtp",
    //                     "payload": "email"
    //                 }
    //             }
    //         }
    //     }
    // }
}
_sessions.save = _sessions.putPrivateKey;

export const Sessions: Record<string, Session> = {};

Object.keys(_sessions).forEach((key) => {
    const sess = _sessions[key];
    const newId = '[' + randomPassword() + ']';
    const clone: Session = {
        ...sess,
        id: newId,
        operation: {
            ...sess.operation,
            id: newId
        }
    };
    Sessions[key] = clone;
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

export const SimulationUrls = Object.keys(Sessions).map((phase, i) => {
    const sess = Sessions[phase];
    let url: string;
    if (phase.endsWith('_ApproveOnboardingRequest')) {
        url = `/intent/${sess.id}/0`;
    } else if (phase.startsWith('create')) {
        const { id } = sess.createFieldOptions![sess.createFieldOptions!.length - 1]
        url = `/create-field/${sess.id}/${id}`;
    } else if (phase.startsWith('put')) {
        const { id } = sess.putFields![sess.putFields!.length - 1]
        url = `/put-field/${sess.id}/${id}`;
    } else if (phase.startsWith('getUserAttributes')) {
        url = `/get-user-attributes/${sess.id}/1`;
    } else if (phase.startsWith('save')) {
        url = `/save/:${sess.id}/${Accounts[0].id}` // put a random account id as the newly saved account
    } else if (phase.startsWith('reject')) {
        // use the last one for now. This is not correct
        // since the rejectedFieldId might not be the last one
        // in the array. But we let it pass for the sake of
        // simulation.
        const { fieldId } = sess.rejectedFieldValues![sess.rejectedFieldValues!.length - 1]
        url = `/create-field/${sess.id}/${fieldId}/rejected`;
    } else if (phase.startsWith('approveChallenge')) {
        const challengeIds = Object.keys(sess.challenges!);
        const challengeId = challengeIds[challengeIds.length - 1];

        url = `auth/${sess.id}/challenge/${challengeId}/approve`;
    } else if (phase.startsWith('respond')) {
        const challengeIds = Object.keys(sess.challenges!);
        const challengeId = challengeIds[challengeIds.length - 1];

        const messages = sess.challenges![challengeId].messages;
        const messageIds = Object.keys(messages);
        const messageId = messageIds[messageIds.length - 1];
        url = `auth/${sess.id}/challenge/${challengeId}/message/${messageId}`;
    } else if (phase.startsWith('approveTransitionToAuthOp')) {
        url = `transition-to-auth-op/${sess.transitionedFromSessionId}/${sess.id}`;
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
    db.sessions.bulkAdd(Object.values(Sessions));
}
