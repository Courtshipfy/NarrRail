<svelte:options immutable />

<script>import { createEventDispatcher, setContext } from 'svelte';
import cc from 'classcat';
import { getMarkerId } from '@xyflow/system';
import { useStore } from '../../store';
import { BezierEdgeInternal } from '../edges';
import { useHandleEdgeSelect } from '../../hooks/useHandleEdgeSelect';
export let id;
export let type = 'default';
export let source = '';
export let target = '';
export let data = {};
export let style = undefined;
export let zIndex = undefined;
export let animated = false;
export let selected = false;
export let selectable = undefined;
export let deletable = undefined;
export let hidden = false;
export let label = undefined;
export let labelStyle = undefined;
export let markerStart = undefined;
export let markerEnd = undefined;
export let sourceHandle = undefined;
export let targetHandle = undefined;
export let sourceX;
export let sourceY;
export let targetX;
export let targetY;
export let sourcePosition;
export let targetPosition;
export let ariaLabel = undefined;
export let interactionWidth = undefined;
// @ todo: support edge updates
let className = '';
export { className as class };
setContext('svelteflow__edge_id', id);
const { edgeLookup, edgeTypes, flowId, elementsSelectable } = useStore();
const dispatch = createEventDispatcher();
$: edgeType = type || 'default';
$: edgeComponent = $edgeTypes[edgeType] || BezierEdgeInternal;
$: markerStartUrl = markerStart ? `url('#${getMarkerId(markerStart, $flowId)}')` : undefined;
$: markerEndUrl = markerEnd ? `url('#${getMarkerId(markerEnd, $flowId)}')` : undefined;
$: isSelectable = selectable ?? $elementsSelectable;
const handleEdgeSelect = useHandleEdgeSelect();
function onClick(event) {
    const edge = $edgeLookup.get(id);
    if (edge) {
        handleEdgeSelect(id);
        dispatch('edgeclick', { event, edge });
    }
}
function onMouseEvent(event, type) {
    const edge = $edgeLookup.get(id);
    if (edge) {
        dispatch(type, { event, edge });
    }
}
</script>

<!-- svelte-ignore a11y-click-events-have-key-events -->
<!-- svelte-ignore a11y-no-noninteractive-element-interactions -->
{#if !hidden}
  <svg style:z-index={zIndex}>
    <g
      class={cc(['svelte-flow__edge', className])}
      class:animated
      class:selected
      class:selectable={isSelectable}
      data-id={id}
      on:click={onClick}
      on:contextmenu={(e) => {
        onMouseEvent(e, 'edgecontextmenu');
      }}
      on:mouseenter={(e) => {
        onMouseEvent(e, 'edgemouseenter');
      }}
      on:mouseleave={(e) => {
        onMouseEvent(e, 'edgemouseleave');
      }}
      aria-label={ariaLabel === null
        ? undefined
        : ariaLabel
          ? ariaLabel
          : `Edge from ${source} to ${target}`}
      role="img"
    >
      <svelte:component
        this={edgeComponent}
        {id}
        {source}
        {target}
        {sourceX}
        {sourceY}
        {targetX}
        {targetY}
        {sourcePosition}
        {targetPosition}
        {animated}
        {selected}
        {label}
        {labelStyle}
        {data}
        {style}
        {interactionWidth}
        selectable={isSelectable}
        deletable={deletable ?? true}
        type={edgeType}
        sourceHandleId={sourceHandle}
        targetHandleId={targetHandle}
        markerStart={markerStartUrl}
        markerEnd={markerEndUrl}
      />
    </g>
  </svg>
{/if}
