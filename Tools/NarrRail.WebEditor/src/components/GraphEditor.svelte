<script>
  import { writable } from 'svelte/store';
  import { onMount } from 'svelte';
  import { SvelteFlow, Controls, Background } from '@xyflow/svelte';
  import '@xyflow/svelte/dist/style.css';

  import DialogueNode from '../nodes/DialogueNode.svelte';
  import ChoiceNode from '../nodes/ChoiceNode.svelte';
  import EndNode from '../nodes/EndNode.svelte';
  import JumpNode from '../nodes/JumpNode.svelte';
  import SetVariableNode from '../nodes/SetVariableNode.svelte';
  import EmitEventNode from '../nodes/EmitEventNode.svelte';

  export let nodes = [];
  export let edges = [];
  export let edgeRenderMode = 'straight';
  export let presetSpeakers = [];

  const nodesStore = writable([]);
  const edgesStore = writable([]);
  let contextMenu = { show: false, x: 0, y: 0, position: { x: 0, y: 0 } };
  let selectedNodes = [];
  let selectedEdges = [];
  let currentEdgeType = 'straight';
  let isNodeDragging = false;
  let suppressNodeClickUntil = 0;
  let pendingNodesDispatch = null;
  let pendingEdgesDispatch = null;
  let dispatchTimer = null;

  function scheduleDispatch(eventName, detail) {
    if (eventName === 'nodes-change') {
      pendingNodesDispatch = detail;
    } else if (eventName === 'edges-change') {
      pendingEdgesDispatch = detail;
    }

    if (dispatchTimer) return;

    dispatchTimer = setTimeout(() => {
      if (pendingNodesDispatch) {
        const nodeEvent = new CustomEvent('nodes-change', { detail: pendingNodesDispatch });
        window.dispatchEvent(nodeEvent);
      }

      if (pendingEdgesDispatch) {
        const edgeEvent = new CustomEvent('edges-change', { detail: pendingEdgesDispatch });
        window.dispatchEvent(edgeEvent);
      }

      pendingNodesDispatch = null;
      pendingEdgesDispatch = null;
      dispatchTimer = null;
    }, 0);
  }

  // 初始化时设置数据
  onMount(() => {
    nodesStore.set(nodes);
    edgesStore.set(edges);

    // 添加键盘删除功能
    const handleKeyDown = (event) => {
      if (event.key === 'Escape' && contextMenu.show) {
        closeContextMenu();
        event.preventDefault();
        return;
      }

      if (event.key === 'Delete' || event.key === 'Backspace') {
        // 检查焦点是否在输入框、文本域或可编辑元素上
        const activeElement = document.activeElement;
        const isInputFocused =
          activeElement.tagName === 'INPUT' ||
          activeElement.tagName === 'TEXTAREA' ||
          activeElement.isContentEditable;

        // 如果焦点在输入框上，不执行删除操作
        if (isInputFocused) {
          return;
        }

        // 删除选中的节点
        if (selectedNodes.length > 0) {
          const nodeIdsToRemove = selectedNodes.map(n => n.id);

          nodesStore.update(nodes => {
            const updated = nodes.filter(n => !nodeIdsToRemove.includes(n.id));
            scheduleDispatch('nodes-change', updated);
            return updated;
          });

          // 级联删除相关的边
          edgesStore.update(edges => {
            const updated = edges.filter(e =>
              !nodeIdsToRemove.includes(e.source) &&
              !nodeIdsToRemove.includes(e.target)
            );
            scheduleDispatch('edges-change', updated);
            return updated;
          });

          selectedNodes = [];
        }

        // 删除选中的边
        if (selectedEdges.length > 0) {
          const edgeIdsToRemove = selectedEdges.map(e => e.id);

          edgesStore.update(edges => {
            const updated = edges.filter(e => !edgeIdsToRemove.includes(e.id));
            scheduleDispatch('edges-change', updated);
            return updated;
          });

          selectedEdges = [];
        }

        event.preventDefault();
      }
    };

    window.addEventListener('keydown', handleKeyDown);

    return () => {
      window.removeEventListener('keydown', handleKeyDown);
    };
  });

  // 监听外部 props 变化
  $: {
    if (nodes && Array.isArray(nodes)) {
      nodesStore.set(nodes);
    }
  }

  $: {
    if (edges && Array.isArray(edges)) {
      edgesStore.set(edges);
    }
  }

  $: {
    // 由外部显式渲染模式驱动连线类型
    // bezier(default) / straight
    currentEdgeType = edgeRenderMode === 'bezier' ? 'default' : 'straight';
  }

  // 不再在 store 响应式块里广播事件，避免与外部同步形成回流/重复触发。
  // 事件只在用户交互处理函数中发出（如 handleNodesChange / handleEdgesChange / handleConnect）。

  // 定义节点类型
  const nodeTypes = {
    dialogue: DialogueNode,
    choice: ChoiceNode,
    end: EndNode,
    jump: JumpNode,
    setvariable: SetVariableNode,
    emitevent: EmitEventNode
  };

  // 处理节点变化
  function handleNodesChange(changes) {
    const hasDraggingPositionChange = changes.some(
      (change) => change.type === 'position' && change.dragging === true
    );
    if (hasDraggingPositionChange && !isNodeDragging) {
      isNodeDragging = true;
      const dragStartEvent = new CustomEvent('node-drag-start');
      window.dispatchEvent(dragStartEvent);
    }

    nodesStore.update(nodes => {
      let updated = [...nodes];
      const removedNodeIds = [];

      changes.forEach(change => {
        if (change.type === 'position') {
          const node = updated.find(n => n.id === change.id);
          if (node && change.position) {
            // 拖拽过程中持续同步位置，保证撤销栈能捕获移动轨迹
            node.position = change.position;
          }
        } else if (change.type === 'remove') {
          removedNodeIds.push(change.id);
          updated = updated.filter(n => n.id !== change.id);
        } else if (change.type === 'add') {
          updated.push(change.item);
        }
      });

      // 如果有节点被删除，级联删除相关的边
      if (removedNodeIds.length > 0) {
        edgesStore.update(edges => {
          const filteredEdges = edges.filter(edge =>
            !removedNodeIds.includes(edge.source) &&
            !removedNodeIds.includes(edge.target)
          );
          scheduleDispatch('edges-change', filteredEdges);
          return filteredEdges;
        });
      }

      scheduleDispatch('nodes-change', updated);
      return updated;
    });
  }

  // 处理边变化
  function handleEdgesChange(changes) {
    edgesStore.update(edges => {
      let updated = [...edges];
      changes.forEach(change => {
        if (change.type === 'remove') {
          updated = updated.filter(e => e.id !== change.id);
        } else if (change.type === 'add') {
          // 新增边 - 确保样式正确
          const newEdge = {
            ...change.item,
            type: currentEdgeType,
            animated: false,
            style: 'stroke: rgba(168, 85, 247, 0.6); stroke-width: 2px;'
          };
          updated.push(newEdge);
        }
      });

      scheduleDispatch('edges-change', updated);
      return updated;
    });
  }

  // 处理连接
  function handleConnect(connection) {
    const newEdge = {
      id: `e${Date.now()}`,
      source: connection.source,
      target: connection.target,
      type: currentEdgeType,
      animated: false,
      style: 'stroke: rgba(168, 85, 247, 0.6); stroke-width: 2px;',
      data: {
        priority: 0,
        condition: { logic: 'All', terms: [] }
      }
    };

    edgesStore.update(edges => {
      const updated = [...edges, newEdge];
      scheduleDispatch('edges-change', updated);
      return updated;
    });
  }

  // 处理节点拖拽结束
  function handleNodeDragStop(event) {
    if (isNodeDragging) {
      isNodeDragging = false;
      suppressNodeClickUntil = Date.now() + 180;
      const dragStopEvent = new CustomEvent('node-drag-stop');
      window.dispatchEvent(dragStopEvent);
    }

    // 拖拽结束后再同步一次最新节点数据
    scheduleDispatch('nodes-change', $nodesStore);
  }

  // 处理边更新
  function handleEdgeUpdate(event) {}

  function handleEdgeUpdateStart(event) {}

  function handleEdgeUpdateEnd(event) {}

  // 处理节点点击
  function handleNodeClick(event) {
    if (Date.now() < suppressNodeClickUntil) {
      return;
    }

    const node = event.detail.node;
    selectedNodes = [node];
    selectedEdges = [];
    const customEvent = new CustomEvent('node-click', { detail: node });
    window.dispatchEvent(customEvent);
  }

  // 处理边点击
  function handleEdgeClick(event) {
    const edge = event.detail.edge;
    selectedEdges = [edge];
    selectedNodes = [];
    const customEvent = new CustomEvent('edge-click', { detail: edge });
    window.dispatchEvent(customEvent);
  }

  // 处理画布点击（空白处）
  function handlePaneClick(event) {
    closeContextMenu();
    selectedNodes = [];
    selectedEdges = [];
    // 点击空白处，取消选中
    const nodeEvent = new CustomEvent('node-click', { detail: null });
    window.dispatchEvent(nodeEvent);
    const edgeEvent = new CustomEvent('edge-click', { detail: null });
    window.dispatchEvent(edgeEvent);
  }

  // 处理画布右键
  function handlePaneContextMenu(event) {
    event.preventDefault();
    const pane = document.querySelector('.svelte-flow__pane');
    const bounds = pane?.getBoundingClientRect();

    const x = event.clientX;
    const y = event.clientY;

    contextMenu = {
      show: true,
      x,
      y,
      position: {
        x: bounds ? x - bounds.left : x,
        y: bounds ? y - bounds.top : y
      }
    };
  }

  // 创建节点
  function createNode(type) {
    const newNode = {
      id: `node-${Date.now()}`,
      type: type,
      position: contextMenu.position,
      data: getDefaultNodeData(type)
    };

    nodesStore.update(nodes => {
      const updated = [...nodes, newNode];
      scheduleDispatch('nodes-change', updated);
      return updated;
    });

    contextMenu.show = false;
  }

  // 获取默认节点数据
  function getDefaultNodeData(type) {
    switch (type) {
      case 'dialogue': {
        const firstSpeaker = Array.isArray(presetSpeakers) ? presetSpeakers[0] : null;
        const defaultSpeakerId = typeof firstSpeaker === 'string'
          ? firstSpeaker
          : (firstSpeaker?.id || '');
        return { speakerId: defaultSpeakerId, textKey: '' };
      }
      case 'choice':
        return { choices: [] };
      case 'jump':
        return { targetNodeId: '' };
      case 'setvariable':
        return { variableName: '', operation: 'Set', value: '' };
      case 'emitevent':
        return { eventId: '', parameters: {} };
      case 'end':
        return {};
      default:
        return {};
    }
  }

  // 关闭右键菜单
  function closeContextMenu() {
    contextMenu.show = false;
  }

</script>

<div
  class="graph-editor"
  role="region"
  aria-label="NarrRail Graph Editor Canvas"
  on:contextmenu={handlePaneContextMenu}
>
  <SvelteFlow
    nodes={nodesStore}
    edges={edgesStore}
    {nodeTypes}
    on:nodeschange={handleNodesChange}
    on:edgeschange={handleEdgesChange}
    on:connect={handleConnect}
    on:nodeclick={handleNodeClick}
    on:edgeclick={handleEdgeClick}
    on:paneclick={handlePaneClick}
    on:panecontextmenu={handlePaneContextMenu}
    on:nodedragstop={handleNodeDragStop}
    on:edgeupdate={handleEdgeUpdate}
    on:edgeupdatestart={handleEdgeUpdateStart}
    on:edgeupdateend={handleEdgeUpdateEnd}
    defaultEdgeOptions={{ type: currentEdgeType, animated: false }}
    nodesDraggable={true}
    nodesConnectable={true}
    elementsSelectable={true}
    selectionOnDrag={true}
    panOnDrag={[1]}
    fitView
    minZoom={0.2}
    maxZoom={2}
  >
    <Background variant="dots" gap={20} size={1} />
    <Controls
      showZoom={true}
      showFitView={true}
      style="background: rgba(255, 255, 255, 0.8); backdrop-filter: blur(20px); border-radius: 12px; border: 0.5px solid rgba(0, 0, 0, 0.04);"
    />

  </SvelteFlow>

  {#if contextMenu.show}
    <div class="context-menu" style="left: {contextMenu.x}px; top: {contextMenu.y}px;">
      <div class="context-menu-header">添加节点</div>
      <button class="context-menu-item" on:click={() => createNode('dialogue')}>
        <span class="material-symbols-outlined">chat</span>
        <span>对话节点</span>
      </button>
      <button class="context-menu-item" on:click={() => createNode('choice')}>
        <span class="material-symbols-outlined">fork_right</span>
        <span>选择节点</span>
      </button>
      <button class="context-menu-item" on:click={() => createNode('jump')}>
        <span class="material-symbols-outlined">arrow_forward</span>
        <span>跳转节点</span>
      </button>
      <button class="context-menu-item" on:click={() => createNode('setvariable')}>
        <span class="material-symbols-outlined">edit_square</span>
        <span>设置变量</span>
      </button>
      <button class="context-menu-item" on:click={() => createNode('emitevent')}>
        <span class="material-symbols-outlined">notifications_active</span>
        <span>触发事件</span>
      </button>
      <button class="context-menu-item" on:click={() => createNode('end')}>
        <span class="material-symbols-outlined">stop_circle</span>
        <span>结束节点</span>
      </button>
    </div>
  {/if}
</div>

<style>
  .graph-editor {
    width: 100%;
    height: 100%;
    position: relative;
  }

  /* 连线在节点下方 */
  :global(.svelte-flow__edges) {
    z-index: 0 !important;
  }

  :global(.svelte-flow__nodes) {
    z-index: 1 !important;
  }

  :global(.svelte-flow__node) {
    cursor: grab;
  }

  :global(.svelte-flow__node:active) {
    cursor: grabbing;
  }

  :global(.svelte-flow__edge-path) {
    stroke: rgba(168, 85, 247, 0.6);
    stroke-width: 3px;
  }

  :global(.svelte-flow__edge.animated .svelte-flow__edge-path) {
    stroke-dasharray: 5;
    animation: dashdraw 0.5s linear infinite;
  }

  :global(.svelte-flow__edge.selected .svelte-flow__edge-path) {
    stroke: rgba(168, 85, 247, 1);
    stroke-width: 4px;
  }

  @keyframes dashdraw {
    to {
      stroke-dashoffset: -10;
    }
  }

  :global(.svelte-flow__handle) {
    width: 12px;
    height: 12px;
    background: rgba(168, 85, 247, 0.8);
    border: 2px solid white;
    box-shadow: 0 2px 8px rgba(168, 85, 247, 0.3);
  }

  :global(.svelte-flow__handle:hover) {
    background: rgba(168, 85, 247, 1);
    transform: scale(1.2);
  }

  /* 多选框（marquee）：内部全透明，仅显示一个外扩包裹边框 */
  :global(.svelte-flow__selection) {
    background: transparent !important;
    border: none !important;
    box-shadow: none !important;
    backdrop-filter: none !important;
    pointer-events: none !important;
    overflow: visible !important;
  }

  :global(.svelte-flow__selection:before) {
    content: "";
    position: absolute;
    inset: -10px; /* 外扩，和节点保持“包住但留缝”的距离 */
    background: transparent !important;
    border: 2px solid rgba(99, 102, 241, 0.78) !important;
    border-radius: 14px !important;
    box-shadow: 0 0 0 1px rgba(255, 255, 255, 0.92);
    pointer-events: none !important;
  }

  /* 调整控制面板位置 */
  :global(.svelte-flow__controls) {
    bottom: 76px !important;
    left: 16px !important;
  }

  /* 调整小地图位置 */
  :global(.svelte-flow__minimap) {
    bottom: 76px !important;
    right: 16px !important;
  }

  /* 右键菜单样式 */
  .context-menu {
    position: fixed;
    background: rgba(255, 255, 255, 0.95);
    backdrop-filter: blur(20px);
    border-radius: 12px;
    border: 0.5px solid rgba(0, 0, 0, 0.08);
    box-shadow: 0 8px 32px rgba(0, 0, 0, 0.12);
    padding: 8px;
    min-width: 200px;
    z-index: 1000;
    animation: menuFadeIn 0.15s ease-out;
  }

  @keyframes menuFadeIn {
    from {
      opacity: 0;
      transform: scale(0.95);
    }
    to {
      opacity: 1;
      transform: scale(1);
    }
  }

  .context-menu-header {
    font-family: 'Plus Jakarta Sans', sans-serif;
    font-size: 11px;
    font-weight: 700;
    color: #86868b;
    text-transform: uppercase;
    letter-spacing: 0.05em;
    padding: 8px 12px 4px;
  }

  .context-menu-item {
    display: flex;
    align-items: center;
    gap: 10px;
    width: 100%;
    padding: 10px 12px;
    border: none;
    background: transparent;
    border-radius: 8px;
    cursor: pointer;
    font-family: 'Plus Jakarta Sans', sans-serif;
    font-size: 13px;
    font-weight: 600;
    color: #1d1d1f;
    transition: all 0.15s ease;
    text-align: left;
  }

  .context-menu-item:hover {
    background: rgba(168, 85, 247, 0.08);
    color: #a855f7;
  }

  .context-menu-item .material-symbols-outlined {
    font-size: 18px;
    color: #86868b;
  }

  .context-menu-item:hover .material-symbols-outlined {
    color: #a855f7;
  }
</style>
