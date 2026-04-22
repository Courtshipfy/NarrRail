import { type CoordinateExtent, type NodeOrigin } from '@xyflow/system';
import type { Node, Edge } from '../types';
import type { SvelteFlowStore } from './types';
export declare const key: unique symbol;
export declare function createStore({ nodes, edges, width, height, fitView: fitViewOnCreate, nodeOrigin, nodeExtent }: {
    nodes?: Node[];
    edges?: Edge[];
    width?: number;
    height?: number;
    fitView?: boolean;
    nodeOrigin?: NodeOrigin;
    nodeExtent?: CoordinateExtent;
}): SvelteFlowStore;
export declare function useStore(): SvelteFlowStore;
export declare function createStoreContext({ nodes, edges, width, height, fitView, nodeOrigin, nodeExtent }: {
    nodes?: Node[];
    edges?: Edge[];
    width?: number;
    height?: number;
    fitView?: boolean;
    nodeOrigin?: NodeOrigin;
    nodeExtent?: CoordinateExtent;
}): SvelteFlowStore;
