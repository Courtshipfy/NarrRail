<script>import { getBezierPath } from '@xyflow/system';
import { BaseEdge } from '../BaseEdge';
export let label = undefined;
export let labelStyle = undefined;
export let style = undefined;
export let markerStart = undefined;
export let markerEnd = undefined;
export let interactionWidth = undefined;
export let sourceX;
export let sourceY;
export let sourcePosition;
export let targetX;
export let targetY;
export let targetPosition;
$: [path, labelX, labelY] = getBezierPath({
    sourceX,
    sourceY,
    targetX,
    targetY,
    sourcePosition,
    targetPosition
});
// this is a workaround for suppressing the warning about unused props
$$restProps;
</script>

<BaseEdge
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
