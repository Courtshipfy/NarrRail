<script>import { getSmoothStepPath } from '@xyflow/system';
import { BaseEdge } from '../BaseEdge';
export let id = undefined;
export let label = undefined;
export let labelStyle = undefined;
export let style = undefined;
export let markerStart = undefined;
export let markerEnd = undefined;
export let pathOptions = undefined;
export let interactionWidth = undefined;
export let sourceX;
export let sourceY;
export let sourcePosition;
export let targetX;
export let targetY;
export let targetPosition;
$: [path, labelX, labelY] = getSmoothStepPath({
    sourceX,
    sourceY,
    targetX,
    targetY,
    sourcePosition,
    targetPosition,
    borderRadius: 0,
    offset: pathOptions?.offset
});
</script>

<BaseEdge
  {id}
  {path}
  {labelX}
  {labelY}
  {label}
  {labelStyle}
  {markerStart}
  {markerEnd}
  {interactionWidth}
  {style}
/>
