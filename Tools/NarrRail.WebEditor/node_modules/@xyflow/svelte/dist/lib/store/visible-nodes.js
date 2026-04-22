import { derived } from 'svelte/store';
import { getNodesInside } from '@xyflow/system';
export function getVisibleNodes(store) {
    return derived([
        store.nodeLookup,
        store.onlyRenderVisibleElements,
        store.width,
        store.height,
        store.viewport,
        store.nodes
    ], ([nodeLookup, onlyRenderVisibleElements, width, height, viewport]) => {
        const transform = [viewport.x, viewport.y, viewport.zoom];
        return onlyRenderVisibleElements
            ? getNodesInside(nodeLookup, { x: 0, y: 0, width, height }, transform, true)
            : Array.from(nodeLookup.values());
    });
}
