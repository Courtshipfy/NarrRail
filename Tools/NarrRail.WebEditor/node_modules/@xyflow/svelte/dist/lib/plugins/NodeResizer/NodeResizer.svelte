<script>import ResizeControl from './ResizeControl.svelte';
import { ResizeControlVariant, XY_RESIZER_HANDLE_POSITIONS, XY_RESIZER_LINE_POSITIONS } from '@xyflow/system';
export let nodeId = undefined;
export let isVisible = true;
export let handleClass = undefined;
export let handleStyle = undefined;
export let lineClass = undefined;
export let lineStyle = undefined;
export let color = undefined;
export let minWidth = 10;
export let minHeight = 10;
export let maxWidth = Number.MAX_VALUE;
export let maxHeight = Number.MAX_VALUE;
export let keepAspectRatio = false;
export let shouldResize = undefined;
export let onResizeStart = undefined;
export let onResize = undefined;
export let onResizeEnd = undefined;
let _minWidth = minWidth || 10;
let _minHeight = minHeight || 10;
let _maxWidth = maxWidth || Number.MAX_VALUE;
let _maxHeight = maxHeight || Number.MAX_VALUE;
</script>

{#if isVisible}
  {#each XY_RESIZER_LINE_POSITIONS as position (position)}
    <ResizeControl
      class={lineClass}
      style={lineStyle}
      {nodeId}
      {position}
      variant={ResizeControlVariant.Line}
      {color}
      minWidth={_minWidth}
      minHeight={_minHeight}
      maxWidth={_maxWidth}
      maxHeight={_maxHeight}
      {onResizeStart}
      {keepAspectRatio}
      {shouldResize}
      {onResize}
      {onResizeEnd}
    />
  {/each}
  {#each XY_RESIZER_HANDLE_POSITIONS as position (position)}
    <ResizeControl
      class={handleClass}
      style={handleStyle}
      {nodeId}
      {position}
      {color}
      minWidth={_minWidth}
      minHeight={_minHeight}
      maxWidth={_maxWidth}
      maxHeight={_maxHeight}
      {onResizeStart}
      {keepAspectRatio}
      {shouldResize}
      {onResize}
      {onResizeEnd}
    />
  {/each}
{/if}
