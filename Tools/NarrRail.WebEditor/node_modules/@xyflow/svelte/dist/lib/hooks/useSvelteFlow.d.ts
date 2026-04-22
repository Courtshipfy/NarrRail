/// <reference types="svelte" />
import { type Writable } from 'svelte/store';
import { type FitBoundsOptions, type SetCenterOptions, type Viewport, type ViewportHelperFunctionOptions, type XYPosition, type ZoomInOut, type Rect, type HandleType, type HandleConnection } from '@xyflow/system';
import type { Edge, FitViewOptions, InternalNode, Node } from '../types';
/**
 * Hook for accessing the ReactFlow instance.
 *
 * @public
 *
 * @returns helper functions
 */
export declare function useSvelteFlow(): {
    /**
     * Zooms viewport in by 1.2.
     *
     * @param options.duration - optional duration. If set, a transition will be applied
     */
    zoomIn: ZoomInOut;
    /**
     * Zooms viewport out by 1 / 1.2.
     *
     * @param options.duration - optional duration. If set, a transition will be applied
     */
    zoomOut: ZoomInOut;
    /**
     * Returns an internal node by id.
     *
     * @param id - the node id
     * @returns the node or undefined if no node was found
     */
    getInternalNode: (id: string) => InternalNode | undefined;
    /**
     * Returns a node by id.
     *
     * @param id - the node id
     * @returns the node or undefined if no node was found
     */
    getNode: (id: string) => Node | undefined;
    /**
     * Returns nodes.
     *
     * @returns nodes array
     */
    getNodes: (ids?: string[]) => Node[];
    /**
     * Returns an edge by id.
     *
     * @param id - the edge id
     * @returns the edge or undefined if no edge was found
     */
    getEdge: (id: string) => Edge | undefined;
    /**
     * Returns edges.
     *
     * @returns edges array
     */
    getEdges: (ids?: string[]) => Edge[];
    /**
     * Sets the current zoom level.
     *
     * @param zoomLevel - the zoom level to set
     * @param options.duration - optional duration. If set, a transition will be applied
     */
    setZoom: (zoomLevel: number, options?: ViewportHelperFunctionOptions) => Promise<boolean>;
    /**
     * Returns the current zoom level.
     *
     * @returns current zoom as a number
     */
    getZoom: () => number;
    /**
     * Sets the center of the view to the given position.
     *
     * @param x - x position
     * @param y - y position
     * @param options.zoom - optional zoom
     */
    setCenter: (x: number, y: number, options?: SetCenterOptions) => Promise<boolean>;
    /**
     * Sets the current viewport.
     *
     * @param viewport - the viewport to set
     * @param options.duration - optional duration. If set, a transition will be applied
     */
    setViewport: (viewport: Viewport, options?: ViewportHelperFunctionOptions) => Promise<boolean>;
    /**
     * Returns the current viewport.
     *
     * @returns Viewport
     */
    getViewport: () => Viewport;
    /**
     * Fits the view.
     *
     * @param options.padding - optional padding
     * @param options.includeHiddenNodes - optional includeHiddenNodes
     * @param options.minZoom - optional minZoom
     * @param options.maxZoom - optional maxZoom
     * @param options.duration - optional duration. If set, a transition will be applied
     * @param options.nodes - optional nodes to fit the view to
     */
    fitView: (options?: FitViewOptions) => Promise<boolean>;
    /**
     * Returns all nodes that intersect with the given node or rect.
     *
     * @param node - the node or rect to check for intersections
     * @param partially - if true, the node is considered to be intersecting if it partially overlaps with the passed node or rect
     * @param nodes - optional nodes array to check for intersections
     *
     * @returns an array of intersecting nodes
     */
    getIntersectingNodes: (nodeOrRect: Node | {
        id: Node['id'];
    } | Rect, partially?: boolean, nodesToIntersect?: Node[]) => Node[];
    /**
     * Checks if the given node or rect intersects with the passed rect.
     *
     * @param node - the node or rect to check for intersections
     * @param area - the rect to check for intersections
     * @param partially - if true, the node is considered to be intersecting if it partially overlaps with the passed react
     *
     * @returns true if the node or rect intersects with the given area
     */
    isNodeIntersecting: (nodeOrRect: Node | {
        id: Node['id'];
    } | Rect, area: Rect, partially?: boolean) => boolean;
    /**
     * Fits the view to the given bounds .
     *
     * @param bounds - the bounds ({ x: number, y: number, width: number, height: number }) to fit the view to
     * @param options.padding - optional padding
     */
    fitBounds: (bounds: Rect, options?: FitBoundsOptions) => Promise<boolean>;
    /**
     * Deletes nodes and edges.
     *
     * @param params.nodes - optional nodes array to delete
     * @param params.edges - optional edges array to delete
     *
     * @returns a promise that resolves with the deleted nodes and edges
     */
    deleteElements: ({ nodes, edges }: {
        nodes?: (Node | {
            id: Node['id'];
        })[];
        edges?: (Edge | {
            id: Edge['id'];
        })[];
    }) => Promise<{
        deletedNodes: Node[];
        deletedEdges: Edge[];
    }>;
    /**
     * Converts a screen / client position to a flow position.
     *
     * @param clientPosition - the screen / client position. When you are working with events you can use event.clientX and event.clientY
     * @param options.snapToGrid - if true, the converted position will be snapped to the grid
     * @returns position as { x: number, y: number }
     *
     * @example
     * const flowPosition = screenToFlowPosition({ x: event.clientX, y: event.clientY })
     */
    screenToFlowPosition: (clientPosition: XYPosition, options?: {
        snapToGrid: boolean;
    }) => XYPosition;
    /**
     * Converts a flow position to a screen / client position.
     *
     * @param flowPosition - the screen / client position. When you are working with events you can use event.clientX and event.clientY
     * @returns position as { x: number, y: number }
     *
     * @example
     * const clientPosition = flowToScreenPosition({ x: node.position.x, y: node.position.y })
     */
    flowToScreenPosition: (flowPosition: XYPosition) => XYPosition;
    viewport: Writable<Viewport>;
    /**
     * Updates a node.
     *
     * @param id - id of the node to update
     * @param nodeUpdate - the node update as an object or a function that receives the current node and returns the node update
     * @param options.replace - if true, the node is replaced with the node update, otherwise the changes get merged
     *
     * @example
     * updateNode('node-1', (node) => ({ position: { x: node.position.x + 10, y: node.position.y } }));
     */
    updateNode: (id: string, nodeUpdate: Partial<Node> | ((node: Node) => Partial<Node>), options?: {
        replace: boolean;
    }) => void;
    /**
     * Updates the data attribute of a node.
     *
     * @param id - id of the node to update
     * @param dataUpdate - the data update as an object or a function that receives the current data and returns the data update
     * @param options.replace - if true, the data is replaced with the data update, otherwise the changes get merged
     *
     * @example
     * updateNodeData('node-1', { label: 'A new label' });
     */
    updateNodeData: (id: string, dataUpdate: object | ((node: Node) => object), options?: {
        replace: boolean;
    }) => void;
    /**
     * Returns the nodes, edges and the viewport as a JSON object.
     *
     * @returns the nodes, edges and the viewport as a JSON object
     */
    toObject: () => {
        nodes: Node[];
        edges: Edge[];
        viewport: Viewport;
    };
    /**
     * Returns the bounds of the given nodes or node ids.
     *
     * @param nodes - the nodes or node ids to calculate the bounds for
     *
     * @returns the bounds of the given nodes
     */
    getNodesBounds: (nodes: (Node | InternalNode | string)[]) => Rect;
    /** Gets all connections for a given handle belonging to a specific node.
     *
     * @param type - handle type 'source' or 'target'
     * @param id - the handle id (this is only needed if you have multiple handles of the same type, meaning you have to provide a unique id for each handle)
     * @param nodeId - the node id the handle belongs to
     * @returns an array with handle connections
     */
    getHandleConnections: ({ type, id, nodeId }: {
        type: HandleType;
        nodeId: string;
        id?: string | null;
    }) => HandleConnection[];
};
