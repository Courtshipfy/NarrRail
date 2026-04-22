<script>import { onMount } from 'svelte';
import { PanOnScrollMode } from '@xyflow/system';
import { useStore } from '../../store';
import zoom from '../../actions/zoom';
export let initialViewport = undefined;
export let onMoveStart = undefined;
export let onMove = undefined;
export let onMoveEnd = undefined;
export let panOnScrollMode;
export let preventScrolling;
export let zoomOnScroll;
export let zoomOnDoubleClick;
export let zoomOnPinch;
export let panOnDrag;
export let panOnScroll;
export let paneClickDistance;
const { viewport, panZoom, selectionRect, minZoom, maxZoom, dragging, translateExtent, lib, panActivationKeyPressed, zoomActivationKeyPressed, viewportInitialized } = useStore();
$: viewPort = initialViewport || { x: 0, y: 0, zoom: 1 };
$: _panOnDrag = $panActivationKeyPressed || panOnDrag;
$: _panOnScroll = $panActivationKeyPressed || panOnScroll;
const onTransformChange = (transform) => viewport.set({ x: transform[0], y: transform[1], zoom: transform[2] });
onMount(() => {
    $viewportInitialized = true;
});
</script>

<div
  class="svelte-flow__zoom"
  use:zoom={{
    viewport,
    minZoom: $minZoom,
    maxZoom: $maxZoom,
    initialViewport: viewPort,
    dragging,
    panZoom,
    onPanZoomStart: onMoveStart,
    onPanZoom: onMove,
    onPanZoomEnd: onMoveEnd,
    zoomOnScroll,
    zoomOnDoubleClick,
    zoomOnPinch,
    panOnScroll: _panOnScroll,
    panOnDrag: _panOnDrag,
    panOnScrollSpeed: 0.5,
    panOnScrollMode: panOnScrollMode || PanOnScrollMode.Free,
    zoomActivationKeyPressed: $zoomActivationKeyPressed,
    preventScrolling: typeof preventScrolling === 'boolean' ? preventScrolling : true,
    noPanClassName: 'nopan',
    noWheelClassName: 'nowheel',
    userSelectionActive: !!$selectionRect,
    translateExtent: $translateExtent,
    lib: $lib,
    paneClickDistance,
    onTransformChange
  }}
>
  <slot />
</div>

<style>
  .svelte-flow__zoom {
    width: 100%;
    height: 100%;
    position: absolute;
    top: 0;
    left: 0;
    z-index: 4;
  }
</style>
