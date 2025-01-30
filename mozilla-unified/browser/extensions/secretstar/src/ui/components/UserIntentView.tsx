import styles from '../styles/UserIntentView.module.scss';
import { Container } from './Container';
import IntentContextView from './IntentContextView';
import '@styles/text.scss';
//import Button from './Button';
import IntentAction from './IntentAction';
//import IntentActionSplit from './IntentActionSplit';
import TopBar from '../controlled/TopBar';
import { Account } from '@root/db/db';
import CommonBodyContainer from './CommonBodyContainer';
import { ELoginUserIntent } from '@berytus/enums';
import type { UriParams } from '@berytus/types';

export interface UserIntentProps {
    uri: UriParams;
    checkpointCapability: (typeof ELoginUserIntent)[keyof typeof ELoginUserIntent];
    onIntent(intent: typeof ELoginUserIntent['Authenticate'], selectedAccount: Account): void;
    onIntent(intent: typeof ELoginUserIntent['Register'], selectedAccount?: undefined): void;
    onCancel(): void;
    accounts?: Array<Account>;
}

export default function UserIntent({ accounts, uri, checkpointCapability, onIntent, onCancel }: UserIntentProps) {
    //browser.authRealm.resolveRequest(requestId, sessionInfo.EUserIntentAuthenticate);
    // this is executed after AuthRealm
    return (
        <Container>
            <TopBar onCancel={() => onCancel()} />
            <CommonBodyContainer>
                <IntentContextView
                    uri={uri}
                    className="pb-3 mb-3 border-b-[0.5px] border-[#dfdfdf]"
                />
                {/* <IntentActionSplit
                    className={styles.bottomContainer}
                    onSubmit={onIntent}
                /> */}
                <IntentAction
                    className={styles.bottomContainer}
                    onLoginIntent={(account) => onIntent(ELoginUserIntent.Authenticate, account)}
                    onRegisterIntent={() => onIntent(ELoginUserIntent.Register)}
                    accounts={accounts}
                    checkpointCapability={checkpointCapability}
                />
            </CommonBodyContainer>
        </Container>
    );
    // return (
    //     <>
    //         <pre>{JSON.stringify(payload, null, 4)}</pre>
    //         <div className={styles.buttonContainer}>
    //             <button className={styles.button} onClick={() => onIntent(EUserIntent.Register)}>
    //                 Register
    //             </button>
    //             <button className={styles.button} onClick={() => onIntent(EUserIntent.Authenticate)}>
    //                 Authenticate
    //             </button>
    //         </div>
    //     </>
    // );
}