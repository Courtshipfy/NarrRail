/// <reference types="svelte" />
import { type HandleConnection, type HandleType } from '@xyflow/system';
export type useHandleConnectionsParams = {
    type: HandleType;
    nodeId?: string;
    id?: string | null;
};
/**
 *  Hook to check if a <Handle /> is connected to another <Handle /> and get the connections.
 *
 * @public
 * @deprecated Use `useNodeConnections` instead.
 * @param param.nodeId
 * @param param.type - handle type 'source' or 'target'
 * @param param.id - the handle id (this is only needed if the node has multiple handles of the same type)
 * @returns an array with connections
 */
export declare function useHandleConnections({ type, nodeId, id }: useHandleConnectionsParams): import("svelte/store").Readable<HandleConnection[]>;
