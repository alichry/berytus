/**
 * The login operation encapsulates either the authentication
 * or the registration process.
 */
[SecureContext, Exposed=(Window)]
interface BerytusLoginOperation {
    readonly attribute BerytusOnboardingIntent intent;
};