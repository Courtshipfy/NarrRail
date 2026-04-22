<script>import { getStraightPath } from '@xyflow/system';
import { BaseEdge } from '../BaseEdge';
export let label = undefined;
export let labelStyle = undefined;
export let style = undefined;
export let markerStart = undefined;
export let markerEnd = undefined;
export let interactionWidth = undefined;
export let sourceX;
export let sourceY;
export let targetX;
export let targetY;
$: [path, labelX, labelY] = getStraightPath({
    sourceX,
    sourceY,
    targetX,
    targetY
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
