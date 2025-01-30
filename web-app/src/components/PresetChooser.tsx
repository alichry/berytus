import { useState } from 'preact/hooks';
import { h, Fragment } from 'preact';
import { Signal } from '@preact/signals';

const presets = [
    { id: 'password', label: 'Password-based authentication'},
    { id: 'srp', label: 'SRP authentication' },
    { id: 'phoneOtp', label: 'Phone TOTP authentication' },
    { id: 'emailOtp', label: 'Email TOTP authentication' },
    { id: 'key', label: 'Key-based authentication' },
]

interface Props {
    selectedPresets: Signal<string>;
    onNext(): void;
    // addPreset(): void;
    // rmPreset(i: number): void;
}

export default function PresetChooser({ selectedPresets } : Props) {
    //const [selectedPresets, setSelectedPresets] = useState<Array<string>>(["password"])
    // quick hack to have Signal work with lsits. One day before submission.
    const addPreset = () => {
        console.log("Next:", remaining[0].id);
        selectedPresets.value = selectedPresets.value + "," + remaining[0].id;

    }
    const setPreset = (i: number, preset: string) => {
        const newPresets = [...list];
        console.log("set", i, "curent", list[i], "new", preset);
        newPresets[i] = preset;
        console.log("newstr:", newPresets.join(','));
        selectedPresets.value = newPresets.join(',');
    }
    const rmPreset = (i: number) => selectedPresets.value = selectedPresets.value.split(",").splice(i, 1).join(',');
    const list = selectedPresets.value.split(',');
    const remaining = presets.filter(f => list.indexOf(f.id) === -1);
    return (
        <>
            <p className="text-sm text-slate-700 mb-4">You can now configure the authenntication flow for your account. Feel free to define several authentication challenges.</p>
            <div className="flex flex-col gap-4 mb-4">
                { list.map((preset, i) => (
                    <>
                        {i !== 0 ? <hr /> : null}
                        {/* <p className="text-sm text-slate-700">
                            Select the authentication scheme for the
                            { " " + (i + 1) }{ i === 0 ? 'st' : i === 1 ? 'nd' : i === 2 ? 'rd' : 'th' }
                            { " " }
                            challenge (factor)
                        </p> */}
                        <div className="flex flex-row flex-wrap gap-4">
                            <select
                                className="min-w-[4rem] flex-1 p-2 text-black"
                                onChange={(e) => setPreset(i, e.target.value)}
                                value={preset}
                            >
                                {[...remaining, presets.find(p => p.id === preset)!].map((p => (
                                    <option value={p.id}>
                                        #{i + 1} - {p.label}
                                    </option>
                                )))}
                            </select>
                            { i !== 0 ? (
                                <button
                                    className="flex-1 border rounded px-5 py-1.5 border-black bg-[#281401] max-w-[4rem]"
                                    onClick={() => {
                                        rmPreset(i);
                                        //setSelectedPresets([...selectedPresets.slice(0, i), ...selectedPresets.slice(i + 1)])
                                    }}
                                >
                                        🗑️
                                </button>
                            ) : null}
                            { i === list.length - 1 && remaining.length > 0 ? (
                                <button
                                    className="flex-1 border rounded px-5 py-1.5 border-black bg-[#281401] max-w-[4rem]"
                                    onClick={() => {
                                        addPreset();
                                        // setSelectedPresets([...selectedPresets, "password"])
                                    }}
                                >
                                    +
                                </button>
                            ) : null}
                        </div>

                    </>
                ))}
            </div>

        </>
    );
}