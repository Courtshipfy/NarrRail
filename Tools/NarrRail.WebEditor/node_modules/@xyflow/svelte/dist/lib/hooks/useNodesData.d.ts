/// <reference types="svelte" />
import { type Readable } from 'svelte/store';
import type { Node } from '../types';
/**
 * Hook for receiving data of one or multiple nodes
 *
 * @public
 * @param nodeId - The id (or ids) of the node to get the data from
 * @returns A readable store with an array of data objects
 */
export declare function useNodesData<NodeType extends Node = Node>(nodeId: string): Readable<Pick<NodeType, 'id' | 'data' | 'type'> | null>;
export declare function useNodesData<NodeType extends Node = Node>(nodeIds: string[]): Readable<Pick<NodeType, 'id' | 'data' | 'type'>[]>;
