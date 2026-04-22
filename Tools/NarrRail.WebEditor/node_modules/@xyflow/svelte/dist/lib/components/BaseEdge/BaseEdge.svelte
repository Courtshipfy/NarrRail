<script>import cc from 'classcat';
import EdgeLabel from '../EdgeLabel/EdgeLabel.svelte';
export let id = undefined;
export let path;
export let label = undefined;
export let labelX = undefined;
export let labelY = undefined;
export let labelStyle = undefined;
export let markerStart = undefined;
export let markerEnd = undefined;
export let style = undefined;
export let interactionWidth = 20;
let className = undefined;
export { className as class };
let interactionWidthValue = interactionWidth === undefined ? 20 : interactionWidth;
</script>

<path
  {id}
  d={path}
  class={cc(['svelte-flow__edge-path', className])}
  marker-start={markerStart}
  marker-end={markerEnd}
  fill="none"
  {style}
/>

{#if interactionWidthValue}
  <path
    d={path}
    stroke-opacity={0}
    stroke-width={interactionWidthValue}
    fill="none"
    class="svelte-flow__edge-interaction"
  />
{/if}

{#if label}
  <EdgeLabel x={labelX} y={labelY} style={labelStyle}>
    {label}
  </EdgeLabel>
{/if}
