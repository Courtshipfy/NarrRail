<script>import { getContext } from 'svelte';
import { EdgeLabelRenderer } from '../EdgeLabelRenderer';
import { useHandleEdgeSelect } from '../../hooks/useHandleEdgeSelect';
export let style = undefined;
export let x = undefined;
export let y = undefined;
const handleEdgeSelect = useHandleEdgeSelect();
const id = getContext('svelteflow__edge_id');
</script>

<EdgeLabelRenderer>
  <div
    class="svelte-flow__edge-label"
    style:transform="translate(-50%, -50%) translate({x}px,{y}px)"
    style={'pointer-events: all;' + style}
    role="button"
    tabindex="-1"
    on:keyup={() => {}}
    on:click={() => {
      if (id) handleEdgeSelect(id);
    }}
  >
    <slot />
  </div>
</EdgeLabelRenderer>
