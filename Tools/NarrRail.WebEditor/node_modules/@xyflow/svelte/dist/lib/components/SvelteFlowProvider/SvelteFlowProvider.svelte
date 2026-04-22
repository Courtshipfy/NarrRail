<script>import { onDestroy, setContext } from 'svelte';
import { createStore, key } from '../../store';
export let initialNodes = undefined;
export let initialEdges = undefined;
export let initialWidth = undefined;
export let initialHeight = undefined;
export let fitView = undefined;
export let nodeOrigin = undefined;
const store = createStore({
    nodes: initialNodes,
    edges: initialEdges,
    width: initialWidth,
    height: initialHeight,
    nodeOrigin,
    fitView
});
setContext(key, {
    getStore: () => store
});
onDestroy(() => {
    store.reset();
});
</script>

<slot />
