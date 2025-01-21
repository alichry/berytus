import { Container } from "./Container"
import TopBar from "../controlled/TopBar"

export default function SaveView() {
    return <Container>
        <TopBar />
        <div className="absolute top-1/3 w-full">
            <p className="text-lg text-center">Registration complete!</p>
        </div>
    </Container>
}