<script>import { getContext, onMount } from 'svelte';
import cc from 'classcat';
import { useStore } from '../../store';
import { XYResizer, ResizeControlVariant } from '@xyflow/system';
export let nodeId = undefined;
export let position = undefined;
export let variant = ResizeControlVariant.Handle;
export let color = undefined;
export let minWidth = 10;
$: _minWidth = minWidth ?? 10;
export let minHeight = 10;
$: _minHeight = minHeight ?? 10;
export let maxWidth = Number.MAX_VALUE;
$: _maxWidth = maxWidth ?? Number.MAX_VALUE;
export let maxHeight = Number.MAX_VALUE;
$: _maxHeight = maxHeight ?? Number.MAX_VALUE;
export let keepAspectRatio = false;
export let shouldResize = undefined;
export let onResizeStart = undefined;
export let onResize = undefined;
export let onResizeEnd = undefined;
export let style = '';
let className = '';
export { className as class };
const { nodeLookup, snapGrid, viewport, nodes, nodeOrigin, domNode } = useStore();
const contextNodeId = getContext('svelteflow__node_id');
$: id = typeof nodeId === 'string' ? nodeId : contextNodeId;
let resizeControlRef;
let resizer = null;
$: defaultPosition = (variant === ResizeControlVariant.Line ? 'right' : 'bottom-right');
$: controlPosition = position ?? defaultPosition;
$: positionClassNames = controlPosition.split('-');
$: colorStyleProp = variant === ResizeControlVariant.Line ? 'border-color' : 'background-color';
$: _style = style ?? '';
$: controlStyle = color ? `${_style} ${colorStyleProp}: ${color};` : _style;
onMount(() => {
    if (resizeControlRef) {
        resizer = XYResizer({
            domNode: resizeControlRef,
            nodeId: id,
            getStoreItems: () => {
                return {
                    nodeLookup: $nodeLookup,
                    transform: [$viewport.x, $viewport.y, $viewport.zoom],
                    snapGrid: $snapGrid ?? undefined,
                    snapToGrid: !!$snapGrid,
                    nodeOrigin: $nodeOrigin,
                    paneDomNode: $domNode
                };
            },
            onChange: (change, childChanges) => {
                const node = $nodeLookup.get(id)?.internals.userNode;
                if (!node) {
                    return;
                }
                if (change.x !== undefined && change.y !== undefined) {
                    node.position = { x: change.x, y: change.y };
                }
                if (change.width !== undefined && change.height !== undefined) {
                    node.width = change.width;
                    node.height = change.height;
                }
                for (const childChange of childChanges) {
                    const childNode = $nodeLookup.get(childChange.id)?.internals.userNode;
                    if (childNode) {
                        childNode.position = childChange.position;
                    }
                }
                $nodes = $nodes;
            }
        });
    }
    return () => {
        resizer?.destroy();
    };
});
$: {
    resizer?.update({
        controlPosition,
        boundaries: {
            minWidth: _minWidth,
            minHeight: _minHeight,
            maxWidth: _maxWidth,
            maxHeight: _maxHeight
        },
        keepAspectRatio: !!keepAspectRatio,
        onResizeStart,
        onResize,
        onResizeEnd,
        shouldResize
    });
}
</script>

<div
  class={cc(['svelte-flow__resize-control', 'nodrag', ...positionClassNames, variant, className])}
  bind:this={resizeControlRef}
  style={controlStyle}
>
  <slot />
</div>
