<script>import { onMount, hasContext } from 'svelte';
import { get } from 'svelte/store';
import cc from 'classcat';
import { ConnectionMode, PanOnScrollMode } from '@xyflow/system';
import { Zoom } from '../Zoom';
import { Pane } from '../Pane';
import { Viewport as ViewportComponent } from '../Viewport';
import { NodeRenderer } from '../NodeRenderer';
import { EdgeRenderer } from '../EdgeRenderer';
import { UserSelection } from '../../components/UserSelection';
import { NodeSelection } from '../../components/NodeSelection';
import { KeyHandler } from '../../components/KeyHandler';
import { ConnectionLine } from '../../components/ConnectionLine';
import { Attribution } from '../../components/Attribution';
import { key, useStore, createStoreContext } from '../../store';
import { updateStore, updateStoreByKeys } from './utils';
import { useColorModeClass } from '../../hooks/useColorModeClass';
export let id = '1';
export let nodes;
export let edges;
export let fitView = undefined;
export let fitViewOptions = undefined;
export let minZoom = undefined;
export let maxZoom = undefined;
export let initialViewport = undefined;
export let viewport = undefined;
export let nodeTypes = undefined;
export let edgeTypes = undefined;
export let selectionKey = undefined;
export let selectionMode = undefined;
export let panActivationKey = undefined;
export let multiSelectionKey = undefined;
export let zoomActivationKey = undefined;
export let nodesDraggable = undefined;
export let nodesConnectable = undefined;
export let nodeDragThreshold = undefined;
export let elementsSelectable = undefined;
export let snapGrid = undefined;
export let deleteKey = undefined;
export let connectionRadius = undefined;
export let connectionLineType = undefined;
export let connectionMode = ConnectionMode.Strict;
export let connectionLineStyle = '';
export let connectionLineContainerStyle = '';
export let onMoveStart = undefined;
export let onMove = undefined;
export let onMoveEnd = undefined;
export let isValidConnection = undefined;
export let translateExtent = undefined;
export let nodeExtent = undefined;
export let onlyRenderVisibleElements = undefined;
export let panOnScrollMode = PanOnScrollMode.Free;
export let preventScrolling = true;
export let zoomOnScroll = true;
export let zoomOnDoubleClick = true;
export let zoomOnPinch = true;
export let panOnScroll = false;
export let panOnDrag = true;
export let selectionOnDrag = undefined;
export let autoPanOnConnect = true;
export let autoPanOnNodeDrag = true;
export let onerror = undefined;
export let ondelete = undefined;
export let onedgecreate = undefined;
export let attributionPosition = undefined;
export let proOptions = undefined;
export let defaultEdgeOptions = undefined;
export let width = undefined;
export let height = undefined;
export let colorMode = 'light';
export let onconnect = undefined;
export let onconnectstart = undefined;
export let onconnectend = undefined;
export let onbeforedelete = undefined;
export let oninit = undefined;
export let nodeOrigin = undefined;
export let paneClickDistance = 0;
export let nodeClickDistance = 0;
export let defaultMarkerColor = '#b1b1b7';
export let style = undefined;
let className = undefined;
export { className as class };
let domNode;
let clientWidth;
let clientHeight;
const initViewport = $viewport || initialViewport;
const store = hasContext(key)
    ? useStore()
    : createStoreContext({
        nodes: get(nodes),
        edges: get(edges),
        width,
        height,
        fitView,
        nodeOrigin,
        nodeExtent
    });
onMount(() => {
    store.width.set(clientWidth);
    store.height.set(clientHeight);
    store.domNode.set(domNode);
    store.syncNodeStores(nodes);
    store.syncEdgeStores(edges);
    store.syncViewport(viewport);
    if (fitView !== undefined) {
        store.fitViewQueued.set(fitView);
    }
    if (fitViewOptions) {
        store.fitViewOptions.set(fitViewOptions);
    }
    updateStore(store, {
        nodeTypes,
        edgeTypes,
        minZoom,
        maxZoom,
        translateExtent,
        paneClickDistance
    });
    return () => {
        store.reset();
    };
});
// Update width & height on resize
$: {
    if (clientWidth !== undefined && clientHeight !== undefined) {
        store.width.set(clientWidth);
        store.height.set(clientHeight);
    }
}
// Call oninit once when flow is intialized
const { initialized } = store;
let onInitCalled = false;
$: {
    if (!onInitCalled && $initialized) {
        oninit?.();
        onInitCalled = true;
    }
}
// this updates the store for simple changes
// where the prop names equals the store name
$: {
    const updatableProps = {
        flowId: id,
        connectionLineType,
        connectionRadius,
        selectionMode,
        snapGrid,
        defaultMarkerColor,
        nodesDraggable,
        nodesConnectable,
        elementsSelectable,
        onlyRenderVisibleElements,
        isValidConnection,
        autoPanOnConnect,
        autoPanOnNodeDrag,
        onerror,
        ondelete,
        onedgecreate,
        connectionMode,
        nodeDragThreshold,
        onconnect,
        onconnectstart,
        onconnectend,
        onbeforedelete,
        nodeOrigin
    };
    updateStoreByKeys(store, updatableProps);
}
$: updateStore(store, {
    nodeTypes,
    edgeTypes,
    minZoom,
    maxZoom,
    translateExtent,
    paneClickDistance
});
$: colorModeClass = useColorModeClass(colorMode);
</script>

<div
  bind:this={domNode}
  bind:clientWidth
  bind:clientHeight
  {style}
  class={cc(['svelte-flow', className, $colorModeClass])}
  data-testid="svelte-flow__wrapper"
  on:dragover
  on:drop
  {...$$restProps}
  role="application"
>
  <KeyHandler
    {selectionKey}
    {deleteKey}
    {panActivationKey}
    {multiSelectionKey}
    {zoomActivationKey}
  />
  <Zoom
    initialViewport={initViewport}
    {onMoveStart}
    {onMove}
    {onMoveEnd}
    panOnScrollMode={panOnScrollMode === undefined ? PanOnScrollMode.Free : panOnScrollMode}
    preventScrolling={preventScrolling === undefined ? true : preventScrolling}
    zoomOnScroll={zoomOnScroll === undefined ? true : zoomOnScroll}
    zoomOnDoubleClick={zoomOnDoubleClick === undefined ? true : zoomOnDoubleClick}
    zoomOnPinch={zoomOnPinch === undefined ? true : zoomOnPinch}
    panOnScroll={panOnScroll === undefined ? false : panOnScroll}
    panOnDrag={panOnDrag === undefined ? true : panOnDrag}
    paneClickDistance={paneClickDistance === undefined ? 0 : paneClickDistance}
  >
    <Pane
      on:paneclick
      on:panecontextmenu
      panOnDrag={panOnDrag === undefined ? true : panOnDrag}
      {selectionOnDrag}
    >
      <ViewportComponent>
        <EdgeRenderer
          on:edgeclick
          on:edgecontextmenu
          on:edgemouseenter
          on:edgemouseleave
          {defaultEdgeOptions}
        />
        <ConnectionLine
          containerStyle={connectionLineContainerStyle}
          style={connectionLineStyle}
          isCustomComponent={$$slots.connectionLine}
        >
          <slot name="connectionLine" slot="connectionLine" />
        </ConnectionLine>
        <div class="svelte-flow__edgelabel-renderer" />
        <div class="svelte-flow__viewport-portal" />
        <NodeRenderer
          {nodeClickDistance}
          on:nodeclick
          on:nodemouseenter
          on:nodemousemove
          on:nodemouseleave
          on:nodedragstart
          on:nodedrag
          on:nodedragstop
          on:nodecontextmenu
        />
        <NodeSelection
          on:selectionclick
          on:selectioncontextmenu
          on:nodedragstart
          on:nodedrag
          on:nodedragstop
        />
      </ViewportComponent>
      <UserSelection />
    </Pane>
  </Zoom>
  <Attribution {proOptions} position={attributionPosition} />
  <slot />
</div>

<style>
  .svelte-flow {
    width: 100%;
    height: 100%;
    overflow: hidden;
    position: relative;
    z-index: 0;

    background-color: var(--background-color, var(--background-color-default));
  }

  :root {
    --background-color-default: #fff;
    --background-pattern-color-default: #ddd;

    --minimap-mask-color-default: rgba(240, 240, 240, 0.6);
    --minimap-mask-stroke-color-default: none;
    --minimap-mask-stroke-width-default: 1;

    --controls-button-background-color-default: #fefefe;
    --controls-button-background-color-hover-default: #f4f4f4;
    --controls-button-color-default: inherit;
    --controls-button-color-hover-default: inherit;
    --controls-button-border-color-default: #eee;
  }
</style>
