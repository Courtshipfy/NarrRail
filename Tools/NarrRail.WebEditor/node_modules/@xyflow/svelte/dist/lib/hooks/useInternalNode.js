import { derived } from 'svelte/store';
import { useStore } from '../store';
/**
 * Hook to get an internal node by id.
 *
 * @public
 * @param id - the node id
 * @returns a readable with an internal node or undefined
 */
export function useInternalNode(id) {
    const { nodeLookup, nodes } = useStore();
    return derived([nodeLookup, nodes], ([nodeLookup]) => nodeLookup.get(id));
}
