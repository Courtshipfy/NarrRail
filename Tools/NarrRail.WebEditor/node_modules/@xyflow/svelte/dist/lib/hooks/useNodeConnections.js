import { getContext } from 'svelte';
import { derived } from 'svelte/store';
import { areConnectionMapsEqual, errorMessages } from '@xyflow/system';
import { useStore } from '../store';
const error014 = errorMessages['error014']();
const initialConnections = [];
/**
 * Hook to retrieve all edges connected to a node. Can be filtered by handle type and id.
 *
 * @public
 * @param param.id - node id - optional if called inside a custom node
 * @param param.handleType - filter by handle type 'source' or 'target'
 * @param param.handleId - filter by handle id (this is only needed if the node has multiple handles of the same type)
 * @todo @param param.onConnect - gets called when a connection is established
 * @todo @param param.onDisconnect - gets called when a connection is removed
 * @returns an array with connections
 */
export function useNodeConnections({ id, handleType, handleId } = {}) {
    const { edges, connectionLookup } = useStore();
    const nodeId = getContext('svelteflow__node_id');
    const currentNodeId = id ?? nodeId;
    if (!currentNodeId) {
        throw new Error(error014);
    }
    let prevConnections = undefined;
    return derived([edges, connectionLookup], ([, connectionLookup], set) => {
        const nextConnections = connectionLookup.get(`${currentNodeId}${handleType ? (handleId ? `-${handleType}-${handleId}` : `-${handleType}`) : ''}`);
        if (!areConnectionMapsEqual(nextConnections, prevConnections)) {
            prevConnections = nextConnections;
            set(Array.from(prevConnections?.values() || []));
        }
    }, initialConnections);
}
