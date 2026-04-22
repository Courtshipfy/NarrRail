/// <reference types="svelte" />
import { type Readable } from 'svelte/store';
import type { InternalNode } from '../types';
/**
 * Hook to get an internal node by id.
 *
 * @public
 * @param id - the node id
 * @returns a readable with an internal node or undefined
 */
export declare function useInternalNode(id: string): Readable<InternalNode | undefined>;
