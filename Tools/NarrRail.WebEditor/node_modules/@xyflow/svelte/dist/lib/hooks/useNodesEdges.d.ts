/// <reference types="svelte" />
/**
 * Hook for getting the current nodes from the store.
 *
 * @public
 * @returns store with an array of nodes
 */
export declare function useNodes(): {
    subscribe: (this: void, run: import("svelte/store").Subscriber<import("..").Node[]>) => import("svelte/store").Unsubscriber;
    update: (this: void, updater: import("svelte/store").Updater<import("..").Node[]>) => void;
    set: (this: void, value: import("..").Node[]) => import("..").Node[];
    setDefaultOptions: (opts: Partial<Omit<import("..").Node<Record<string, unknown>, string>, "id">>) => void;
    setOptions: (opts: import("../store/utils").NodeStoreOptions) => void;
};
/**
 * Hook for getting the current edges from the store.
 *
 * @public
 * @returns store with an array of edges
 */
export declare function useEdges(): import("svelte/store").Writable<import("..").Edge[]> & {
    setDefaultOptions: (opts: import("..").DefaultEdgeOptions) => void;
};
