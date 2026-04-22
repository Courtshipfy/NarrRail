<script>import { getContext } from 'svelte';
import cc from 'classcat';
import { Position, XYHandle, isMouseEvent, areConnectionMapsEqual, handleConnectionChange, ConnectionMode } from '@xyflow/system';
import { useStore } from '../../store';
export let id = undefined;
export let type = 'source';
export let position = Position.Top;
export let style = undefined;
export let isValidConnection = undefined;
export let onconnect = undefined;
export let ondisconnect = undefined;
// @todo implement connectablestart, connectableend
// export let isConnectableStart: $$Props['isConnectableStart'] = undefined;
// export let isConnectableEnd: $$Props['isConnectableEnd'] = undefined;
let isConnectableProp = undefined;
export { isConnectableProp as isConnectable };
let className = undefined;
export { className as class };
$: isTarget = type === 'target';
const nodeId = getContext('svelteflow__node_id');
const connectable = getContext('svelteflow__node_connectable');
$: isConnectable = isConnectableProp !== undefined ? isConnectableProp : $connectable;
$: handleId = id || null;
const store = useStore();
const { connectionMode, domNode, nodeLookup, connectionRadius, viewport, isValidConnection: isValidConnectionStore, lib, addEdge, onedgecreate, panBy, cancelConnection, updateConnection, autoPanOnConnect, edges, connectionLookup, onconnect: onConnectAction, onconnectstart: onConnectStartAction, onconnectend: onConnectEndAction, flowId, connection } = store;
function onPointerDown(event) {
    const isMouseTriggered = isMouseEvent(event);
    if ((isMouseTriggered && event.button === 0) || !isMouseTriggered) {
        XYHandle.onPointerDown(event, {
            handleId,
            nodeId,
            isTarget,
            connectionRadius: $connectionRadius,
            domNode: $domNode,
            nodeLookup: $nodeLookup,
            connectionMode: $connectionMode,
            lib: $lib,
            autoPanOnConnect: $autoPanOnConnect,
            flowId: $flowId,
            isValidConnection: isValidConnection ?? $isValidConnectionStore,
            updateConnection,
            cancelConnection,
            panBy,
            onConnect: (connection) => {
                const edge = $onedgecreate ? $onedgecreate(connection) : connection;
                if (!edge) {
                    return;
                }
                addEdge(edge);
                $onConnectAction?.(connection);
            },
            onConnectStart: (event, startParams) => {
                $onConnectStartAction?.(event, {
                    nodeId: startParams.nodeId,
                    handleId: startParams.handleId,
                    handleType: startParams.handleType
                });
            },
            onConnectEnd: (event, connectionState) => {
                $onConnectEndAction?.(event, connectionState);
            },
            getTransform: () => [$viewport.x, $viewport.y, $viewport.zoom],
            getFromHandle: () => $connection.fromHandle
        });
    }
}
let prevConnections = null;
let connections;
$: if (onconnect || ondisconnect) {
    // connectionLookup is not reactive, so we use edges to get notified about updates
    $edges;
    connections = $connectionLookup.get(`${nodeId}-${type}${id ? `-${id}` : ''}`);
}
$: {
    if (prevConnections && !areConnectionMapsEqual(connections, prevConnections)) {
        const _connections = connections ?? new Map();
        handleConnectionChange(prevConnections, _connections, ondisconnect);
        handleConnectionChange(_connections, prevConnections, onconnect);
    }
    prevConnections = connections ?? new Map();
}
$: connectionInProcess = !!$connection.fromHandle;
$: connectingFrom =
    $connection.fromHandle?.nodeId === nodeId &&
        $connection.fromHandle?.type === type &&
        $connection.fromHandle?.id === handleId;
$: connectingTo =
    $connection.toHandle?.nodeId === nodeId &&
        $connection.toHandle?.type === type &&
        $connection.toHandle?.id === handleId;
$: isPossibleEndHandle =
    $connectionMode === ConnectionMode.Strict
        ? $connection.fromHandle?.type !== type
        : nodeId !== $connection.fromHandle?.nodeId || handleId !== $connection.fromHandle?.id;
$: valid = connectingTo && $connection.isValid;
</script>

<!--
@component
The Handle component is the part of a node that can be used to connect nodes.
-->
<div
  data-handleid={handleId}
  data-nodeid={nodeId}
  data-handlepos={position}
  data-id="{$flowId}-{nodeId}-{id || null}-{type}"
  class={cc([
    'svelte-flow__handle',
    `svelte-flow__handle-${position}`,
    'nodrag',
    'nopan',
    position,
    className
  ])}
  class:valid
  class:connectingto={connectingTo}
  class:connectingfrom={connectingFrom}
  class:source={!isTarget}
  class:target={isTarget}
  class:connectablestart={isConnectable}
  class:connectableend={isConnectable}
  class:connectable={isConnectable}
  class:connectionindicator={isConnectable && (!connectionInProcess || isPossibleEndHandle)}
  on:mousedown={onPointerDown}
  on:touchstart={onPointerDown}
  {style}
  role="button"
  tabindex="-1"
>
  <slot />
</div>
