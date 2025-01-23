export interface ISelector<I extends object> {
    items: Array<I>;
    selected: I;

    selectedIndex: number;
    canSelectPrevious: boolean;
    canSelectNext: boolean;

    selectNext(): void;
    selectPrevious(): void;
    select(index: number): void;
    /**
     * Update the items list and select the
     * first item in the list.
     */
    setItems(items: Array<I>): void;
    /**
     * Remove all items except for the selected one.
     */
    pruneItems(): void;
}

type OnSelect<I extends object> = (selected: I, index: number) => void;

export class Selector<I extends object> implements ISelector<I> {
    items!: I[];
    selected!: I;
    selectedIndex!: number;
    canSelectPrevious!: boolean;
    canSelectNext!: boolean;

    protected constructor(items?: I[]) {
        if (items) {
            this.setItems(items);
        }
    }

    setItems(items: I[]) {
        if (items.length === 0) {
            throw new Error('Cannot construct Selector with an empty item array');
        }
        this.items = items;
        this.selected = items[0];
        this.selectedIndex = 0;
        this.canSelectPrevious = false;
        this.canSelectNext = items.length > 1;
    }

    pruneItems() {
        if (this.items.length === 1) {
            return;
        }
        this.setItems(
            this.items.filter((_, ind) => ind === this.selectedIndex)
        );
    }

    selectNext(): void {
        this.select(this.selectedIndex + 1);
    }

    selectPrevious(): void {
        this.select(this.selectedIndex - 1);
    }

    select(index: number): void {
        if (index >= this.items.length || index < 0) {
            throw new Error('Selector.select() error. Passed index out of bounds');
        }
        this.selectedIndex = index;
        this.selected = this.items[index];
        this.canSelectNext = this.selectedIndex < this.items.length - 1;
        this.canSelectPrevious = this.selectedIndex > 0;
    }

}
