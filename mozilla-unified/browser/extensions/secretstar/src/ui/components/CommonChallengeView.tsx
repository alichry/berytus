import { faKey } from "@fortawesome/free-solid-svg-icons";
import TopBar from "../controlled/TopBar";
import Breadcrumbs from "./Breadcrumbs";
import { Container } from "./Container";
import type { BerytusChallengeInfoUnion } from "@berytus/types";
import { EBerytusChallengeType } from "@berytus/enums";

export interface CommonChallengeViewProps {
    challenge: BerytusChallengeInfoUnion;
}

export default function CommonChallengeView({ challenge }: CommonChallengeViewProps) {
    return (
        <Container>
            <TopBar />
            <div className="mx-8">
                <Breadcrumbs
                    headerIcon={faKey}
                    locations={[
                        "Authentication",
                        challenge.type
                    ]}
                />
            </div>
            <div className="ml-6 mr-8">

            </div>
        </Container>
    );
}