// @ts-ignore
import Cite from "citation-js";
// @ts-ignore
//import fs from 'fs';
import refs from './refs.bib';

export class Library {
    protected references: Record<string, typeof Cite>;
    protected cited: Record<string, number>;
    protected nextIndex = 1;

    constructor(cslEntries: Array<any>) {
        this.references = {};
        this.cited = {};
        for (const entry of cslEntries) {
            this.references[entry.label] = new Cite(entry);
        }
    }

    cite(...ids: Array<string>): string {
        const indecies = this.#addCitations(ids);
        return `[${indecies.join(', ')}]`;
    }

    #addCitations(ids: Array<string>): Array<number> {
        const res: Array<number> = [];
        for (const id of ids) {
            if (! this.references[id]) {
                throw new Error('Invalid reference "' + id + '"');
            }
            if (! this.cited[id]) {
                this.cited[id] = this.nextIndex++;
            }
            res.push(this.cited[id]);
        }
        return res;
    }

    getCitedReferences(): Array<{ index: number, citation: typeof Cite }> {
        const res: Array<{ index: number, citation: typeof Cite }> = [];
        const keys = Object.keys(this.cited) as Array<keyof typeof this.cited>;
        keys.forEach(key => {
            const num: number = this.cited[key];
            res.push({ index: num, citation: this.references[key]});
        })
        return res;
    }
}

// const data = fs.readFileSync(
//     new URL('./', import.meta.url).pathname + 'refs.bib',
//     { encoding: "utf8" }
// );
// const entries = Cite.parse.bibtex.text(data);
const entries = Cite.parse.bibtex.text(refs);

export default new Library(entries);
