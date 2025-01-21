import GetUserAttributes from "@controlled/GetUserAttributes";
import { HashRouter, Route, Routes, useLocation } from "react-router-dom";
import UserIntent from "./ui/controlled/UserIntent";
import Loading from "./ui/components/Loading";
import Home from "./ui/components/Home";
import CreateField from "./ui/controlled/CreateField";
import PutField from "./ui/controlled/PutField";
import Save from "./ui/controlled/Save";
import Accounts from "./ui/controlled/Accounts";
import Account from "./ui/controlled/Account";
import SimulationPortal from "./ui/controlled/SimulationPortal";
import Me from "./ui/controlled/Me";
import { PropsWithChildren, useEffect } from "react";
import { useSettings } from "./hooks";
import ApproveChallenge from "./ui/controlled/challenge/ApproveChallenge";
import RespondToMessage from "./ui/controlled/challenge/RespondToMessage";
import AbortedChallenge from "./ui/controlled/challenge/AbortedChallenge";
import SealedChallenge from "./ui/controlled/challenge/SealedChallenge";
import AuthFinished from "./ui/controlled/AuthFinished";
import TransitionToAuth from "./ui/controlled/TransitionToAuth";
import EnableE2E from "./ui/controlled/EnableE2E";

function ResizeRootForSampleWebApp({ children }: PropsWithChildren) {
    const location = useLocation();
    useEffect(() => {
        if (! location.pathname.endsWith("/sample")) {
            document.getElementById('root')?.removeAttribute('style');
            return;
        }
        document.getElementById('root')?.setAttribute('style', 'max-width: initial;');
    }, [location]);
    return <>{children}</>;
}


export default function App() {
    return (
        <HashRouter>
            <ResizeRootForSampleWebApp>
                <Routes>
                    {/* pageContext = window|popup */}
                    <Route path="/:pageContext?">
                        <Route path="" element={<Home />} />
                        <Route path="me" element={<Me />} />
                        <Route path="loading" element={<Loading />} />
                        <Route path="intent/:sessionId/:afterVersion?" element={<UserIntent />} />
                        <Route path="enable-e2e/:sessionId" element={<EnableE2E />} />
                        {/* BRTTODO: 23/1/2024 Remove afterVersion -- this seems to be problematic. We should refactor how we process requests to use request id instead */}
                        <Route path="get-user-attributes/:sessionId/:afterVersion?" element={<GetUserAttributes />} />
                        <Route path="create-field/:sessionId/:fieldId/rejected" element={<CreateField rejected />} />
                        <Route path="create-field/:sessionId/:fieldId" element={<CreateField />} />
                        <Route path="put-field/:sessionId/:fieldId" element={<PutField />} />
                        <Route path="save/:sessionId/:accountId" element={<Save />} />
                        <Route path="transition-to-auth-op/:currentSessionId/:nextSessionId" element={<TransitionToAuth />} />
                        <Route path="account" element={<Accounts />} />
                        <Route path="account/:accountId" element={<Account />} />
                        <Route path="auth/:sessionId/challenge/:challengeId/approve"
                            element={<ApproveChallenge />}
                        />
                        <Route path="auth/:sessionId/challenge/:challengeId/aborted"
                            element={<AbortedChallenge />}
                        />
                        <Route path="auth/:sessionId/challenge/:challengeId/sealed"
                            element={<SealedChallenge />}
                        />
                        <Route path="auth/:sessionId/challenge/:challengeId/message/:messageId"
                            element={<RespondToMessage />}
                        />
                        <Route path="auth/:sessionId/finished" element={<AuthFinished />} />
                        <Route path="simulate" element={<SimulationPortal />} />
                    </Route>
                </Routes>
            </ResizeRootForSampleWebApp>
        </HashRouter>
    );
}