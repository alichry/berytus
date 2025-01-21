import { SimulationUrls } from "@root/db/dummy-data";
import { Container } from "../components/Container";
import TopBar from "./TopBar";
import Button from "../components/Button";
import { useNavigateWithPageContextRoute } from "@root/hooks";

export default function SimulationPortal() {
    const navigate = useNavigateWithPageContextRoute();
    return (
        <Container>
            <TopBar />
            <div className="flex flex-col gap-4">
                {SimulationUrls.map((su, i) => (
                    <Button
                        key={i}
                        text={su.phase}
                        onClick={() => navigate(su.url)}
                    />
                ))}
                <Button
                    text="Sample WebApp"
                    onClick={() => navigate("/sample")}
                />
            </div>

        </Container>
    )
}