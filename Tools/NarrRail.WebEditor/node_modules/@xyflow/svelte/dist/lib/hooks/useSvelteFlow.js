import { get } from 'svelte/store';
import { getOverlappingArea, isRectObject, nodeToRect, pointToRendererPoint, getViewportForBounds, getElementsToRemove, rendererPointToPoint, evaluateAbsolutePosition, getNodesBounds } from '@xyflow/system';
import { useStore } from '../store';
import { isNode } from '../utils';
/**
 * Hook for accessing the ReactFlow instance.
 *
 * @public
 *
 * @returns helper functions
 */
export function useSvelteFlow() {
    const { zoomIn, zoomOut, fitView, onbeforedelete, snapGrid, viewport, width, height, minZoom, maxZoom, panZoom, nodes, edges, domNode, nodeLookup, nodeOrigin, edgeLookup, connectionLookup } = useStore();
    const getNodeRect = (node) => {
        const $nodeLookup = get(nodeLookup);
        const nodeToUse = isNode(node) ? node : $nodeLookup.get(node.id);
        const position = nodeToUse.parentId
            ? evaluateAbsolutePosition(nodeToUse.position, nodeToUse.measured, nodeToUse.parentId, $nodeLookup, get(nodeOrigin))
            : nodeToUse.position;
        const nodeWithPosition = {
            ...nodeToUse,
            position,
            width: nodeToUse.measured?.width ?? nodeToUse.width,
            height: nodeToUse.measured?.height ?? nodeToUse.height
        };
        return nodeToRect(nodeWithPosition);
    };
    const updateNode = (id, nodeUpdate, options = { replace: false }) => {
        const node = get(nodeLookup).get(id)?.internals.userNode;
        if (!node) {
            return;
        }
        const nextNode = typeof nodeUpdate === 'function' ? nodeUpdate(node) : nodeUpdate;
        if (options.replace) {
            nodes.update((nds) => nds.map((node) => {
                if (node.id === id) {
                    return isNode(nextNode) ? nextNode : { ...node, ...nextNode };
                }
                return node;
            }));
        }
        else {
            Object.assign(node, nextNode);
            nodes.update((nds) => nds);
        }
    };
    const getInternalNode = (id) => get(nodeLookup).get(id);
    return {
        zoomIn,
        zoomOut,
        getInternalNode,
        getNode: (id) => getInternalNode(id)?.internals.userNode,
        getNodes: (ids) => (ids === undefined ? get(nodes) : getElements(get(nodeLookup), ids)),
        getEdge: (id) => get(edgeLookup).get(id),
        getEdges: (ids) => (ids === undefined ? get(edges) : getElements(get(edgeLookup), ids)),
        setZoom: (zoomLevel, options) => {
            const currentPanZoom = get(panZoom);
            return currentPanZoom
                ? currentPanZoom.scaleTo(zoomLevel, { duration: options?.duration })
                : Promise.resolve(false);
        },
        getZoom: () => get(viewport).zoom,
        setViewport: async (nextViewport, options) => {
            const currentViewport = get(viewport);
            const currentPanZoom = get(panZoom);
            if (!currentPanZoom) {
                return Promise.resolve(false);
            }
            await currentPanZoom.setViewport({
                x: nextViewport.x ?? currentViewport.x,
                y: nextViewport.y ?? currentViewport.y,
                zoom: nextViewport.zoom ?? currentViewport.zoom
            }, { duration: options?.duration });
            return Promise.resolve(true);
        },
        getViewport: () => get(viewport),
        setCenter: async (x, y, options) => {
            const nextZoom = typeof options?.zoom !== 'undefined' ? options.zoom : get(maxZoom);
            const currentPanZoom = get(panZoom);
            if (!currentPanZoom) {
                return Promise.resolve(false);
            }
            await currentPanZoom.setViewport({
                x: get(width) / 2 - x * nextZoom,
                y: get(height) / 2 - y * nextZoom,
                zoom: nextZoom
            }, { duration: options?.duration });
            return Promise.resolve(true);
        },
        fitView,
        fitBounds: async (bounds, options) => {
            const currentPanZoom = get(panZoom);
            if (!currentPanZoom) {
                return Promise.resolve(false);
            }
            const viewport = getViewportForBounds(bounds, get(width), get(height), get(minZoom), get(maxZoom), options?.padding ?? 0.1);
            await currentPanZoom.setViewport(viewport, { duration: options?.duration });
            return Promise.resolve(true);
        },
        getIntersectingNodes: (nodeOrRect, partially = true, nodesToIntersect) => {
            const isRect = isRectObject(nodeOrRect);
            const nodeRect = isRect ? nodeOrRect : getNodeRect(nodeOrRect);
            if (!nodeRect) {
                return [];
            }
            return (nodesToIntersect || get(nodes)).filter((n) => {
                const internalNode = get(nodeLookup).get(n.id);
                if (!internalNode || (!isRect && n.id === nodeOrRect.id)) {
                    return false;
                }
                const currNodeRect = nodeToRect(internalNode);
                const overlappingArea = getOverlappingArea(currNodeRect, nodeRect);
                const partiallyVisible = partially && overlappingArea > 0;
                return partiallyVisible || overlappingArea >= nodeRect.width * nodeRect.height;
            });
        },
        isNodeIntersecting: (nodeOrRect, area, partially = true) => {
            const isRect = isRectObject(nodeOrRect);
            const nodeRect = isRect ? nodeOrRect : getNodeRect(nodeOrRect);
            if (!nodeRect) {
                return false;
            }
            const overlappingArea = getOverlappingArea(nodeRect, area);
            const partiallyVisible = partially && overlappingArea > 0;
            return partiallyVisible || overlappingArea >= nodeRect.width * nodeRect.height;
        },
        deleteElements: async ({ nodes: nodesToRemove = [], edges: edgesToRemove = [] }) => {
            const { nodes: matchingNodes, edges: matchingEdges } = await getElementsToRemove({
                nodesToRemove,
                edgesToRemove,
                nodes: get(nodes),
                edges: get(edges),
                onBeforeDelete: get(onbeforedelete)
            });
            if (matchingNodes) {
                nodes.update((nds) => nds.filter((node) => !matchingNodes.some(({ id }) => id === node.id)));
            }
            if (matchingEdges) {
                edges.update((eds) => eds.filter((edge) => !matchingEdges.some(({ id }) => id === edge.id)));
            }
            return {
                deletedNodes: matchingNodes,
                deletedEdges: matchingEdges
            };
        },
        screenToFlowPosition: (position, options = { snapToGrid: true }) => {
            const _domNode = get(domNode);
            if (!_domNode) {
                return position;
            }
            const _snapGrid = options.snapToGrid ? get(snapGrid) : false;
            const { x, y, zoom } = get(viewport);
            const { x: domX, y: domY } = _domNode.getBoundingClientRect();
            const correctedPosition = {
                x: position.x - domX,
                y: position.y - domY
            };
            return pointToRendererPoint(correctedPosition, [x, y, zoom], _snapGrid !== null, _snapGrid || [1, 1]);
        },
        /**
         *
         * @param position
         * @returns
         */
        flowToScreenPosition: (position) => {
            const _domNode = get(domNode);
            if (!_domNode) {
                return position;
            }
            const { x, y, zoom } = get(viewport);
            const { x: domX, y: domY } = _domNode.getBoundingClientRect();
            const rendererPosition = rendererPointToPoint(position, [x, y, zoom]);
            return {
                x: rendererPosition.x + domX,
                y: rendererPosition.y + domY
            };
        },
        toObject: () => {
            return {
                nodes: get(nodes).map((node) => ({
                    ...node,
                    // we want to make sure that changes to the nodes object that gets returned by toObject
                    // do not affect the nodes object
                    position: { ...node.position },
                    data: { ...node.data }
                })),
                edges: get(edges).map((edge) => ({ ...edge })),
                viewport: { ...get(viewport) }
            };
        },
        updateNode,
        updateNodeData: (id, dataUpdate, options) => {
            const node = get(nodeLookup).get(id)?.internals.userNode;
            if (!node) {
                return;
            }
            const nextData = typeof dataUpdate === 'function' ? dataUpdate(node) : dataUpdate;
            node.data = options?.replace ? nextData : { ...node.data, ...nextData };
            nodes.update((nds) => nds);
        },
        getNodesBounds: (nodes) => {
            const _nodeLookup = get(nodeLookup);
            const _nodeOrigin = get(nodeOrigin);
            return getNodesBounds(nodes, { nodeLookup: _nodeLookup, nodeOrigin: _nodeOrigin });
        },
        getHandleConnections: ({ type, id, nodeId }) => Array.from(get(connectionLookup)
            .get(`${nodeId}-${type}-${id ?? null}`)
            ?.values() ?? []),
        viewport
    };
}
// eslint-disable-next-line @typescript-eslint/no-explicit-any
function getElements(lookup, ids) {
    const result = [];
    for (const id of ids) {
        const item = lookup.get(id);
        if (item) {
            const element = 'internals' in item ? item.internals?.userNode : item;
            result.push(element);
        }
    }
    return result;
}
