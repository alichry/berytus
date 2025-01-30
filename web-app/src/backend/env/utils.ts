export const env = (key: string, def?: string): string => {
    const value = key in process.env ? process.env[key] : def;
    if (value === undefined) {
        console.error('Error: missing env', key);
        process.exit(1);
    }
    if (value.length === 0) {
        console.error('Error: empty env', key);
        process.exit(1);
    }
    return value;
}

export const int = (value: string, nonZero = true): number => {
    const num = Number(value);
    if (Number.isNaN(num)) {
        console.error(
            'Error: one of the env values is an invalid number, got NaN for str:',
            value
        );
        process.exit(2);
    }
    if (! Number.isInteger(num)) {
        console.error(
            'Error: one of the env values is an invalid integer, str:',
            value
        );
        process.exit(2);
    }
    if (num < 0) {
        console.error(
            'Error: one of the env values is a valid integer but negative, str:',
            value
        );
        process.exit(2);
    }
    if (nonZero && num === 0) {
        console.error(
            'Error: one of the env values is a valid integer but happened to be 0, str:',
            value
        );
        process.exit(2);
    }
    return num;
}
