/* -*- Mode: indent-tabs-mode: nil; js-indent-level: 2 -*- */
/* vim: set sts=2 sw=2 et tw=80: */
"use strict";

/**
 * @type {import('../../berytus/src/Liaison.sys.mjs')}
 */
const { liaison } = ChromeUtils.importESModule(
  "resource://gre/modules/BerytusLiaison.sys.mjs"
);

function backgroundFunction() {
  return function () {
    class ManagerRequestHandler {
      getSigningKey = (context, args) => {
        throw new Error('Method not implemented.');
      }
      getCredentialsMetadata = (context, args) => {
        throw new Error('Method not implemented.');
      }
    }
    class LoginRequestHandler {
      approveOperation = (context, args) => {
        throw new Error('Method not implemented.');
      }
      closeOpeation = (context) => {
        throw new Error('Method not implemented.');
      }
      getRecordMetadata = (context) => {
        throw new Error('Method not implemented.');
      }
      updateMetadata = (context, args) => {
        throw new Error('Method not implemented.');
      }
    }
    class ChannelRequestHandler {
      generateKeyExchangeParameters = (context, args) => {
        throw new Error('Method not implemented.');
      }
      enableEndToEndEncryption = (context, args) => {
        throw new Error('Method not implemented.');
      }
      closeChannel = (context) => {
        throw new Error('Method not implemented.');
      }
    }
    class AccountCreationRequestHandler {
      approveTransitionToAuthOp = (context, args) => {
        throw new Error('Method not implemented.');
      }
      getUserAttributes = (context) => {
        throw new Error('Method not implemented.');
      }
      addField = (context, args) => {
        throw new Error('Method not implemented.');
      }
      rejectFieldValue = (context, args) => {
        throw new Error('Method not implemented.');
      }
    }
    class AccountAuthenticationRequestHandler {
      approveChallengeRequest = (context, args) => {
        throw new Error('Method not implemented.');
      }
      abortChallenge = (context, args) => {
        throw new Error('Method not implemented.');
      }
      closeChallenge = (context, args) => {
        throw new Error('Method not implemented.');
      }
      respondToChallengeMessage = (context, args) => {
        throw new Error('Method not implemented.');
      }
    }
    class Handler {
      manager;
      login;
      channel;
      accountCreation;
      accountAuthentication = {
        abortChallenge(context, args) { }
      }

      constructor({
        manager = new ManagerRequestHandler(),
        login = new LoginRequestHandler(),
        channel = new ChannelRequestHandler(),
        accountCreation = new AccountCreationRequestHandler(),
        accountAuthentication = new AccountAuthenticationRequestHandler()
      } = {}) {
        this.manager = manager;
        this.login = login;
        this.channel = channel;
        this.accountCreation = accountCreation;
        this.accountAuthentication = accountAuthentication;
      }
    }
    // PLACEHOLDER
  }
}

function background(cb) {
  const fn = backgroundFunction();
  fn._toString = fn.toString;
  fn.toString = () => {
    const cbCode = cb.toString();
    const executeCbCode = `(${cbCode})();`
    return fn._toString().replace("// PLACEHOLDER", executeCbCode);
  }
  return fn;
}

add_task(async function test_berytus_invalid_permissions() {
  const extension = ExtensionTestUtils.loadExtension({
    background: () => {
      browser.test.assertEq(undefined, browser.berytus, "API should be undefined when permissions are not set");
      browser.test.sendMessage("done");
    },
    manifest: {
      manifest_version: 2,
    },
  });

  await extension.startup();
  await extension.awaitMessage("done");
  await extension.unload();
});

add_task(async function test_berytus_registration() {

  const extension = ExtensionTestUtils.loadExtension({
    background: background(() => {
      const handler = new Handler();
      browser.berytus.registerRequestHandler(handler)
        .finally(() => {
          browser.test.sendMessage("done");
        })
    }),
    manifest: {
      manifest_version: 2,
      permissions: ["berytus"]
    },
  });

  await extension.startup();
  await extension.awaitMessage("done");

  Assert.equal(liaison.isManagerRegistered(extension.id), true);

  await extension.unload();
});

add_task(async function test_berytus_handler_getSigningKey() {

  const extension = ExtensionTestUtils.loadExtension({
    background: background(() => {
      const handler = new Handler({
        manager: new (class extends ManagerRequestHandler {
          getSigningKey = (ctx) => {
            // priv: MC4CAQAwBQYDK2VwBCIEINceTfVAd0DzkZKmfmGurcoljjOPm6Ix9CTNBXLcWt3b
            browser.berytus.resolveRequest(ctx.request.id, "MCowBQYDK2VwAyEAJevlUdx72BF8mxdwurBJI9WNgRDMaoYfb0VqywaLOJE=");
          }
        })()
      });
      browser.berytus.registerRequestHandler(handler)
        .finally(() => {
          browser.test.sendMessage("done");
        })
    }),
    manifest: {
      manifest_version: 2,
      permissions: ["berytus"]
    },
  });

  await extension.startup();
  await extension.awaitMessage("done");

  Assert.equal(liaison.isManagerRegistered(extension.id), true);
  Assert.equal(
    await liaison.getRequestHandler(extension.id).manager.getSigningKey(
      { document: { id: 4 } },
      {
        webAppActor: {
          originalUri: {
            hostname: "example.tld",
            path: "/",
            port: 443,
            scheme: 'https:',
            uri: 'https://example.tld/'
          },
          currentUri: {
            hostname: "example.tld",
            path: "/login",
            port: 443,
            scheme: 'https:',
            uri: 'https://example.tld/login'
          },
        }
      }
    ),
    "MCowBQYDK2VwAyEAJevlUdx72BF8mxdwurBJI9WNgRDMaoYfb0VqywaLOJE="
  );

  await extension.unload();
});