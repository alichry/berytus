import CommonBodyContainer from "./CommonBodyContainer";
import { Container } from "./Container";
import TopBar from "../controlled/TopBar";
import { url } from "@root/workers/paths";
import { faAddressCard, faUpRightFromSquare, faUser, faFlask } from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { useIdentity, useNavigateWithPageContextRoute } from "@root/hooks";
import { PAGECONTEXT_WINDOW } from "@root/pagecontext";

const isDev = process.env.NODE_ENV === "development";

/**
 * BRTTODO: Show past sessions
 */
export default function Home() {
    const navigate = useNavigateWithPageContextRoute();
    const identity = useIdentity();

    return <Container>
        <TopBar />
        <CommonBodyContainer className="text-neutral-50 py-8 flex flex-row flex-wrap items-center justify-center text-6xl gap-x-24 gap-y-16">
            <FontAwesomeIcon
                icon={faAddressCard}
                className="cursor-pointer"
                bounce={!identity?.userAttributes} // keep bouncing till there is an identity yo!
                onClick={() => { navigate('/me') }}
            />
            <FontAwesomeIcon
                icon={faUpRightFromSquare}
                className="cursor-pointer"
                onClick={() => { window.open("https://poc.berytus.io") }}
            />
            <FontAwesomeIcon
                icon={faUser}
                className="cursor-pointer"
                onClick={() => { navigate('/account') }}
                //text="Go to accounts"
            />
            <FontAwesomeIcon
                icon={faFlask}
                className={isDev ? "cursor-pointer" : "cursor-not-allowed opacity-25"}
                onClick={() => {
                    isDev && navigate('/simulate');
                }}
                //text="Go to simulator"
            />
        </CommonBodyContainer>
    </Container>
}