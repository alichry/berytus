/// <reference types="vite-plugin-svgr/client" />
import { useCallback, useEffect, useState, type PropsWithChildren } from "react";
import { cn } from "../utils/joinClassName";
import Play from '../assets/play-button-svgrepo-com.svg?react';
import Pause from '../assets/pause-svgrepo-com.svg?react';
import Spinner from '../assets/spinner.svg?react';
import bulletStyles from '../styles/BulletStatus.module.scss';
import {
    fetchEntries,
    type FetchEntry
} from '../signals/fetch-entries.js';
import {
    FetchMonitor,
    FetchMonitorViaServiceWorker,
    FetchMonitorViaWebExtensions
} from '../scripts/setup-fetch-monitor.js';

interface BulletStatusProps {
    className?: string;
    state: "Pending" | "Active" | "Error";
}

function BulletStatus(props: PropsWithChildren<BulletStatusProps>) {
    return (
        <div className={
            cn("py-2",
                props.className,
                bulletStyles.bullet,
                bulletStyles[props.state]
            )}>
            {props.children}
        </div>
    );
}

interface FetchMonitorManagerProps {
    source: FetchMonitorSource;
    className?: string;
}

export enum FetchMonitorSource {
    WebExtensions = "WebExtensions",
    ServiceWorker = "ServiceWorker"
}

enum FetchMonitorState {
    Pending = 'Pending',
    Loading = 'Loading',
    Active = 'Active',
    Error = 'Error'
}

export default function FetchMonitorManager(props: FetchMonitorManagerProps) {
    const [monState, setMonState] = useState<FetchMonitorState>(FetchMonitorState.Pending);
    const cb = useCallback((entry: FetchEntry) => {
            fetchEntries.set([...fetchEntries.get(), entry]);
    }, [fetchEntries]);
    const [worker, setWorker] = useState<FetchMonitor | null>(null);
    useEffect(() => {
        const run = async () => {
            if (worker) {
                await worker.stop();
            }
            setWorker(props.source === FetchMonitorSource.ServiceWorker
                ? new FetchMonitorViaServiceWorker(cb)
                : new FetchMonitorViaWebExtensions(cb));
        }
        run();
        return () => {
            if (! worker) {
                return;
            }
            worker.stop();
        }
    }, [cb]);
    if (! worker) {
        return null;
    }
    return (
        <div className={cn(props.className, "flex flex-row items-center gap-2")}>
            { monState === FetchMonitorState.Pending ? (
                <Play
                    className="cursor-pointer w-3 h-3 fill-green-400"
                    onClick={() => {
                        setMonState(FetchMonitorState.Loading);
                        worker.start()
                            .then(() => setMonState(FetchMonitorState.Active))
                            .catch(() => setMonState(FetchMonitorState.Error))
                    }}
                />
            ) : null}
            { monState === FetchMonitorState.Loading ? (
                <Spinner className="animate-spin w-3 h-3 fill-white" />
            ) : null}
            { monState === FetchMonitorState.Active ? (
                <Pause
                    className="cursor-pointer w-3 h-3 [&_path]:fill-amber-200"
                    onClick={() => {
                        setMonState(FetchMonitorState.Loading);
                        Promise.resolve(worker.stop())
                            .then(() => setMonState(FetchMonitorState.Pending))
                            .catch(() => setMonState(FetchMonitorState.Error))
                    }}
                />
            ) : null}
            <BulletStatus state={monState === FetchMonitorState.Loading ? "Pending" : monState}>
                {props.source}
            </BulletStatus>
        </div>
    )
}