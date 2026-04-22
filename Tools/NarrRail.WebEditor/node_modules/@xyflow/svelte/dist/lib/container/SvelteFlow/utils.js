// this is helper function for updating the store
// for props where we need to call a specific store action
export function updateStore(store, { nodeTypes, edgeTypes, minZoom, maxZoom, translateExtent, paneClickDistance }) {
    if (nodeTypes !== undefined) {
        store.setNodeTypes(nodeTypes);
    }
    if (edgeTypes !== undefined) {
        store.setEdgeTypes(edgeTypes);
    }
    if (minZoom !== undefined) {
        store.setMinZoom(minZoom);
    }
    if (maxZoom !== undefined) {
        store.setMaxZoom(maxZoom);
    }
    if (translateExtent !== undefined) {
        store.setTranslateExtent(translateExtent);
    }
    if (paneClickDistance !== undefined) {
        store.setPaneClickDistance(paneClickDistance);
    }
}
const getKeys = (obj) => Object.keys(obj);
export function updateStoreByKeys(store, keys) {
    getKeys(keys).forEach((prop) => {
        const update = keys[prop];
        if (update !== undefined) {
            // @ts-expect-error @todo: how to fix this TS error?
            store[prop].set(update);
        }
    });
}
