<svelte:options immutable />

<script>import { setContext, onDestroy, createEventDispatcher } from 'svelte';
import { get, writable } from 'svelte/store';
import cc from 'classcat';
import { errorMessages, Position } from '@xyflow/system';
import drag from '../../actions/drag';
import { useStore } from '../../store';
import DefaultNode from '../nodes/DefaultNode.svelte';
import { getNodeInlineStyleDimensions } from './utils';
export let node;
export let id;
export let data = {};
export let selected = false;
export let draggable = undefined;
export let selectable = undefined;
export let connectable = true;
export let deletable = true;
export let hidden = false;
export let dragging = false;
export let resizeObserver = null;
export let style = undefined;
export let type = 'default';
export let isParent = false;
export let positionX;
export let positionY;
export let sourcePosition = undefined;
export let targetPosition = undefined;
export let zIndex;
export let measuredWidth = undefined;
export let measuredHeight = undefined;
export let initialWidth = undefined;
export let initialHeight = undefined;
export let width = undefined;
export let height = undefined;
export let dragHandle = undefined;
export let initialized = false;
export let parentId = undefined;
export let nodeClickDistance = undefined;
let className = '';
export { className as class };
const store = useStore();
const { nodeTypes, nodeDragThreshold, selectNodesOnDrag, handleNodeSelection, updateNodeInternals, elementsSelectable, nodesDraggable } = store;
let nodeRef;
let prevNodeRef = null;
const dispatchNodeEvent = createEventDispatcher();
const connectableStore = writable(connectable);
let prevType = undefined;
let prevSourcePosition = undefined;
let prevTargetPosition = undefined;
$: nodeType = type || 'default';
$: nodeTypeValid = !!$nodeTypes[nodeType];
$: nodeComponent = $nodeTypes[nodeType] || DefaultNode;
$: {
    if (!nodeTypeValid) {
        console.warn('003', errorMessages['error003'](type));
    }
}
$: inlineStyleDimensions = getNodeInlineStyleDimensions({
    width,
    height,
    initialWidth,
    initialHeight,
    measuredWidth,
    measuredHeight
});
$: {
    connectableStore.set(!!connectable);
}
$: {
    // if type, sourcePosition or targetPosition changes,
    // we need to re-calculate the handle positions
    const doUpdate = (prevType && nodeType !== prevType) ||
        (prevSourcePosition && sourcePosition !== prevSourcePosition) ||
        (prevTargetPosition && targetPosition !== prevTargetPosition);
    if (doUpdate) {
        requestAnimationFrame(() => updateNodeInternals(new Map([
            [
                id,
                {
                    id,
                    nodeElement: nodeRef,
                    force: true
                }
            ]
        ])));
    }
    prevType = nodeType;
    prevSourcePosition = sourcePosition;
    prevTargetPosition = targetPosition;
}
setContext('svelteflow__node_id', id);
setContext('svelteflow__node_connectable', connectableStore);
$: {
    if (resizeObserver && (nodeRef !== prevNodeRef || !initialized)) {
        prevNodeRef && resizeObserver.unobserve(prevNodeRef);
        nodeRef && resizeObserver.observe(nodeRef);
        prevNodeRef = nodeRef;
    }
}
onDestroy(() => {
    if (prevNodeRef) {
        resizeObserver?.unobserve(prevNodeRef);
    }
});
function onSelectNodeHandler(event) {
    if (selectable && (!get(selectNodesOnDrag) || !draggable || get(nodeDragThreshold) > 0)) {
        // this handler gets called by XYDrag on drag start when selectNodesOnDrag=true
        // here we only need to call it when selectNodesOnDrag=false
        handleNodeSelection(id);
    }
    dispatchNodeEvent('nodeclick', { node: node.internals.userNode, event });
}
</script>

<!-- svelte-ignore a11y-click-events-have-key-events -->
<!-- svelte-ignore a11y-no-static-element-interactions -->
{#if !hidden}
  <div
    use:drag={{
      nodeId: id,
      isSelectable: selectable,
      disabled: false,
      handleSelector: dragHandle,
      noDragClass: 'nodrag',
      nodeClickDistance,
      onNodeMouseDown: handleNodeSelection,
      onDrag: (event, _, targetNode, nodes) => {
        dispatchNodeEvent('nodedrag', { event, targetNode, nodes });
      },
      onDragStart: (event, _, targetNode, nodes) => {
        dispatchNodeEvent('nodedragstart', { event, targetNode, nodes });
      },
      onDragStop: (event, _, targetNode, nodes) => {
        dispatchNodeEvent('nodedragstop', { event, targetNode, nodes });
      },
      store
    }}
    bind:this={nodeRef}
    data-id={id}
    class={cc(['svelte-flow__node', `svelte-flow__node-${nodeType}`, className])}
    class:dragging
    class:selected
    class:draggable
    class:connectable
    class:selectable
    class:nopan={draggable}
    class:parent={isParent}
    style:z-index={zIndex}
    style:transform="translate({positionX}px, {positionY}px)"
    style:visibility={initialized ? 'visible' : 'hidden'}
    style="{style ?? ''};{inlineStyleDimensions.width}{inlineStyleDimensions.height}"
    on:click={onSelectNodeHandler}
    on:mouseenter={(event) => dispatchNodeEvent('nodemouseenter', { node, event })}
    on:mouseleave={(event) => dispatchNodeEvent('nodemouseleave', { node, event })}
    on:mousemove={(event) => dispatchNodeEvent('nodemousemove', { node, event })}
    on:contextmenu={(event) => dispatchNodeEvent('nodecontextmenu', { node, event })}
  >
    <svelte:component
      this={nodeComponent}
      {data}
      {id}
      selected={selected ?? false}
      selectable={selectable ?? $elementsSelectable ?? true}
      deletable={deletable ?? true}
      {sourcePosition}
      {targetPosition}
      {zIndex}
      {dragging}
      draggable={draggable ?? $nodesDraggable ?? true}
      {dragHandle}
      {parentId}
      type={nodeType}
      isConnectable={$connectableStore}
      positionAbsoluteX={positionX}
      positionAbsoluteY={positionY}
      {width}
      {height}
    />
  </div>
{/if}
