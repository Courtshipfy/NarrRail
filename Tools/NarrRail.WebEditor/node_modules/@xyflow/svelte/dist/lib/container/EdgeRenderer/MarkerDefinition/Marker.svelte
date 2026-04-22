<script>import { MarkerType } from '@xyflow/system';
export let id;
export let type;
export let width = 12.5;
export let height = 12.5;
export let markerUnits = 'strokeWidth';
export let orient = 'auto-start-reverse';
export let color = undefined;
export let strokeWidth = undefined;
</script>

<marker
  class="svelte-flow__arrowhead"
  {id}
  markerWidth={`${width}`}
  markerHeight={`${height}`}
  viewBox="-10 -10 20 20"
  {markerUnits}
  {orient}
  refX="0"
  refY="0"
>
  {#if type === MarkerType.Arrow}
    <polyline
      stroke={color}
      stroke-linecap="round"
      stroke-linejoin="round"
      stroke-width={strokeWidth}
      fill="none"
      points="-5,-4 0,0 -5,4"
    />
  {:else if type === MarkerType.ArrowClosed}
    <polyline
      stroke={color}
      stroke-linecap="round"
      stroke-linejoin="round"
      stroke-width={strokeWidth}
      fill={color}
      points="-5,-4 0,0 -5,4 -5,-4"
    />
  {/if}
</marker>
