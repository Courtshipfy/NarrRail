/// <reference types="svelte" />
import { type NodeConnection, type HandleType } from '@xyflow/system';
type UseNodeConnectionsParams = {
    id?: string;
    handleType?: HandleType;
    handleId?: string;
};
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
export declare function useNodeConnections({ id, handleType, handleId }?: UseNodeConnectionsParams): import("svelte/store").Readable<NodeConnection[]>;
export {};
