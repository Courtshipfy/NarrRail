import { useStore } from '../store';
/**
 * Hook for seeing if nodes are initialized
 * @returns - nodesInitialized Writable
 */
export function useNodesInitialized() {
    const { nodesInitialized } = useStore();
    return {
        subscribe: nodesInitialized.subscribe
    };
}
/**
 * Hook for seeing if the flow is initialized
 * @returns - initialized Writable
 */
export function useInitialized() {
    const { initialized } = useStore();
    return {
        subscribe: initialized.subscribe
    };
}
