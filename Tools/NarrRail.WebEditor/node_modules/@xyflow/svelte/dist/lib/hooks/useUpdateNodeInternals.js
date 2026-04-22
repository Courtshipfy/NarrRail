import { get } from 'svelte/store';
import { useStore } from '../store';
/**
 * Hook for updating node internals.
 *
 * @public
 * @returns function for updating node internals
 */
export function useUpdateNodeInternals() {
    const { domNode, updateNodeInternals } = useStore();
    // @todo: do we want to add this to system?
    const updateInternals = (id) => {
        const updateIds = Array.isArray(id) ? id : [id];
        const updates = new Map();
        updateIds.forEach((updateId) => {
            const nodeElement = get(domNode)?.querySelector(`.svelte-flow__node[data-id="${updateId}"]`);
            if (nodeElement) {
                updates.set(updateId, { id: updateId, nodeElement, force: true });
            }
        });
        requestAnimationFrame(() => updateNodeInternals(updates));
    };
    return updateInternals;
}
