export function cn(...classNames: Array<string | undefined | false>) {
    return classNames.filter(n => typeof n === "string").join(' ');
}