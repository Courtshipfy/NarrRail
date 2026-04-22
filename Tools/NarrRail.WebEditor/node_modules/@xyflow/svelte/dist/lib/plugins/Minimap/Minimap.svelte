<script context="module">"use strict";
// eslint-disable-next-line @typescript-eslint/no-explicit-any
const getAttrFunction = (func) => func instanceof Function ? func : () => func;
</script>

<script>import cc from 'classcat';
import { getBoundsOfRects, getInternalNodesBounds, getNodeDimensions, nodeHasDimensions } from '@xyflow/system';
import { useStore } from '../../store';
import { Panel } from '../../container/Panel';
import MinimapNode from './MinimapNode.svelte';
import interactive from './interactive';
export let position = 'bottom-right';
export let ariaLabel = 'Mini map';
export let nodeStrokeColor = 'transparent';
export let nodeColor = undefined;
export let nodeClass = '';
export let nodeBorderRadius = 5;
export let nodeStrokeWidth = 2;
export let bgColor = undefined;
export let maskColor = undefined;
export let maskStrokeColor = undefined;
export let maskStrokeWidth = undefined;
export let width = undefined;
export let height = undefined;
export let pannable = true;
export let zoomable = true;
export let inversePan = undefined;
export let zoomStep = undefined;
export let style = '';
let className = '';
export { className as class };
const defaultWidth = 200;
const defaultHeight = 150;
const { nodes, nodeLookup, viewport, width: containerWidth, height: containerHeight, flowId, panZoom, translateExtent } = useStore();
const nodeColorFunc = nodeColor === undefined ? undefined : getAttrFunction(nodeColor);
const nodeStrokeColorFunc = getAttrFunction(nodeStrokeColor);
const nodeClassFunc = getAttrFunction(nodeClass);
const shapeRendering = 
// @ts-expect-error - TS doesn't know about chrome
typeof window === 'undefined' || !!window.chrome ? 'crispEdges' : 'geometricPrecision';
const labelledBy = `svelte-flow__minimap-desc-${$flowId}`;
$: viewBB = {
    x: -$viewport.x / $viewport.zoom,
    y: -$viewport.y / $viewport.zoom,
    width: $containerWidth / $viewport.zoom,
    height: $containerHeight / $viewport.zoom
};
let boundingRect = viewBB;
$: {
    boundingRect =
        $nodeLookup.size > 0
            ? getBoundsOfRects(getInternalNodesBounds($nodeLookup, { filter: (n) => !n.hidden }), viewBB)
            : viewBB;
    $nodes;
}
$: elementWidth = width ?? defaultWidth;
$: elementHeight = height ?? defaultHeight;
$: scaledWidth = boundingRect.width / elementWidth;
$: scaledHeight = boundingRect.height / elementHeight;
$: viewScale = Math.max(scaledWidth, scaledHeight);
$: viewWidth = viewScale * elementWidth;
$: viewHeight = viewScale * elementHeight;
$: offset = 5 * viewScale;
$: x = boundingRect.x - (viewWidth - boundingRect.width) / 2 - offset;
$: y = boundingRect.y - (viewHeight - boundingRect.height) / 2 - offset;
$: viewboxWidth = viewWidth + offset * 2;
$: viewboxHeight = viewHeight + offset * 2;
const getViewScale = () => viewScale;
</script>

<Panel
  {position}
  style={style + (bgColor ? `;--xy-minimap-background-color-props:${bgColor}` : '')}
  class={cc(['svelte-flow__minimap', className])}
  data-testid="svelte-flow__minimap"
>
  {#if $panZoom}
    <svg
      width={elementWidth}
      height={elementHeight}
      viewBox="{x} {y} {viewboxWidth} {viewboxHeight}"
      class="svelte-flow__minimap-svg"
      role="img"
      aria-labelledby={labelledBy}
      style:--xy-minimap-mask-background-color-props={maskColor}
      style:--xy-minimap-mask-stroke-color-props={maskStrokeColor}
      style:--xy-minimap-mask-stroke-width-props={maskStrokeWidth
        ? maskStrokeWidth * viewScale
        : undefined}
      use:interactive={{
        panZoom: $panZoom,
        viewport,
        getViewScale,
        translateExtent: $translateExtent,
        width: $containerWidth,
        height: $containerHeight,
        inversePan,
        zoomStep,
        pannable,
        zoomable
      }}
    >
      {#if ariaLabel}<title id={labelledBy}>{ariaLabel}</title>{/if}

      {#each $nodes as userNode (userNode.id)}
        {@const node = $nodeLookup.get(userNode.id)}
        {#if node && nodeHasDimensions(node)}
          {@const nodeDimesions = getNodeDimensions(node)}
          <MinimapNode
            x={node.internals.positionAbsolute.x}
            y={node.internals.positionAbsolute.y}
            {...nodeDimesions}
            selected={node.selected}
            color={nodeColorFunc?.(node)}
            borderRadius={nodeBorderRadius}
            strokeColor={nodeStrokeColorFunc(node)}
            strokeWidth={nodeStrokeWidth}
            {shapeRendering}
            class={nodeClassFunc(node)}
          />
        {/if}
      {/each}
      <path
        class="svelte-flow__minimap-mask"
        d="M{x - offset},{y - offset}h{viewboxWidth + offset * 2}v{viewboxHeight +
          offset * 2}h{-viewboxWidth - offset * 2}z
      M{viewBB.x},{viewBB.y}h{viewBB.width}v{viewBB.height}h{-viewBB.width}z"
        fill-rule="evenodd"
        pointer-events="none"
      />
    </svg>
  {/if}
</Panel>
