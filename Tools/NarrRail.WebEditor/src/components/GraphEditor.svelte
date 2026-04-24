<script>
  import { writable } from 'svelte/store';
  import { onMount } from 'svelte';
  import { SvelteFlow, Controls, Background, MiniMap } from '@xyflow/svelte';
  import '@xyflow/svelte/dist/style.css';

  import DialogueNode from '../nodes/DialogueNode.svelte';
  import ChoiceNode from '../nodes/ChoiceNode.svelte';
  import EndNode from '../nodes/EndNode.svelte';
  import JumpNode from '../nodes/JumpNode.svelte';
  import SetVariableNode from '../nodes/SetVariableNode.svelte';
  import EmitEventNode from '../nodes/EmitEventNode.svelte';

  export let nodes = [];
  export let edges = [];

  const nodesStore = writable([]);
  const edgesStore = writable([]);
  let contextMenu = { show: false, x: 0, y: 0, position: { x: 0, y: 0 } };
  let selectedNodes = [];
  let selectedEdges = [];

  // 初始化时设置数据
  onMount(() => {
    console.log('GraphEditor mounted with nodes:', nodes);
    console.log('GraphEditor mounted with edges:', edges);
    nodesStore.set(nodes);
    edgesStore.set(edges);

    // 添加键盘删除功能
    const handleKeyDown = (event) => {
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

        console.log('删除键被按下，选中的节点:', selectedNodes, '选中的边:', selectedEdges);

        // 删除选中的节点
        if (selectedNodes.length > 0) {
          const nodeIdsToRemove = selectedNodes.map(n => n.id);
          console.log('准备删除节点:', nodeIdsToRemove);

          nodesStore.update(nodes => {
            const updated = nodes.filter(n => !nodeIdsToRemove.includes(n.id));
            console.log('删除后的节点列表:', updated);

            // 触发节点变化事件
            const event = new CustomEvent('nodes-change', { detail: updated });
            window.dispatchEvent(event);

            return updated;
          });

          // 级联删除相关的边
          edgesStore.update(edges => {
            const updated = edges.filter(e =>
              !nodeIdsToRemove.includes(e.source) &&
              !nodeIdsToRemove.includes(e.target)
            );
            console.log('级联删除边后的边列表:', updated);

            // 触发边变化事件
            const event = new CustomEvent('edges-change', { detail: updated });
            window.dispatchEvent(event);

            return updated;
          });

          selectedNodes = [];
        }

        // 删除选中的边
        if (selectedEdges.length > 0) {
          const edgeIdsToRemove = selectedEdges.map(e => e.id);
          console.log('准备删除边:', edgeIdsToRemove);

          edgesStore.update(edges => {
            const updated = edges.filter(e => !edgeIdsToRemove.includes(e.id));
            console.log('删除后的边列表:', updated);

            // 触发边变化事件
            const event = new CustomEvent('edges-change', { detail: updated });
            window.dispatchEvent(event);

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
      console.log('外部 nodes 变化:', nodes);
      nodesStore.set(nodes);
    }
  }

  $: {
    if (edges && Array.isArray(edges)) {
      console.log('外部 edges 变化:', edges);
      edgesStore.set(edges);
    }
  }

  // 监听 store 变化
  $: {
    console.log('edgesStore 当前值:', $edgesStore);
    // 每次 edgesStore 变化时，主动触发事件
    if ($edgesStore) {
      const event = new CustomEvent('edges-change', { detail: $edgesStore });
      window.dispatchEvent(event);
    }
  }

  $: {
    console.log('nodesStore 当前值:', $nodesStore);
    // 每次 nodesStore 变化时，主动触发事件
    if ($nodesStore) {
      const event = new CustomEvent('nodes-change', { detail: $nodesStore });
      window.dispatchEvent(event);
    }
  }

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
    console.log('handleNodesChange 被调用:', changes);
    nodesStore.update(nodes => {
      let updated = [...nodes];
      const removedNodeIds = [];

      changes.forEach(change => {
        console.log('节点变化类型:', change.type, change);
        if (change.type === 'position' && change.dragging === false) {
          const node = updated.find(n => n.id === change.id);
          if (node && change.position) {
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

          console.log('级联删除边，剩余边:', filteredEdges);
          const edgeEvent = new CustomEvent('edges-change', { detail: filteredEdges });
          window.dispatchEvent(edgeEvent);

          return filteredEdges;
        });
      }

      // 立即触发节点变化事件
      console.log('触发 nodes-change 事件:', updated);
      const nodeEvent = new CustomEvent('nodes-change', { detail: updated });
      window.dispatchEvent(nodeEvent);

      return updated;
    });
  }

  // 处理边变化
  function handleEdgesChange(changes) {
    console.log('handleEdgesChange 被调用:', changes);
    edgesStore.update(edges => {
      let updated = [...edges];
      changes.forEach(change => {
        console.log('边变化类型:', change.type, change);
        if (change.type === 'remove') {
          updated = updated.filter(e => e.id !== change.id);
        } else if (change.type === 'add') {
          // 新增边 - 确保样式正确
          const newEdge = {
            ...change.item,
            type: 'default',
            animated: false,
            style: 'stroke: rgba(168, 85, 247, 0.6); stroke-width: 2px;'
          };
          updated.push(newEdge);
          console.log('添加新边:', newEdge);
        }
      });

      // 立即触发事件
      console.log('触发 edges-change 事件 (from handleEdgesChange):', updated);
      const event = new CustomEvent('edges-change', { detail: updated });
      window.dispatchEvent(event);

      return updated;
    });
  }

  // 处理连接
  function handleConnect(connection) {
    console.log('handleConnect 被调用:', connection);
    const newEdge = {
      id: `e${Date.now()}`,
      source: connection.source,
      target: connection.target,
      type: 'default',
      animated: false,
      style: 'stroke: rgba(168, 85, 247, 0.6); stroke-width: 2px;',
      data: {
        priority: 0,
        condition: { logic: 'All', terms: [] }
      }
    };

    console.log('创建新边:', newEdge);

    edgesStore.update(edges => {
      const updated = [...edges, newEdge];
      console.log('更新后的边列表:', updated);

      // 立即触发事件
      console.log('触发 edges-change 事件:', updated);
      const event = new CustomEvent('edges-change', { detail: updated });
      window.dispatchEvent(event);

      return updated;
    });
  }

  // 处理节点拖拽结束
  function handleNodeDragStop(event) {
    console.log('节点拖拽结束:', event);
  }

  // 处理边更新
  function handleEdgeUpdate(event) {
    console.log('边更新:', event);
  }

  function handleEdgeUpdateStart(event) {
    console.log('边更新开始:', event);
  }

  function handleEdgeUpdateEnd(event) {
    console.log('边更新结束:', event);
  }

  // 处理节点点击
  function handleNodeClick(event) {
    const node = event.detail.node;
    console.log('节点被点击:', node);
    selectedNodes = [node];
    selectedEdges = [];
    const customEvent = new CustomEvent('node-click', { detail: node });
    window.dispatchEvent(customEvent);
  }

  // 处理边点击
  function handleEdgeClick(event) {
    const edge = event.detail.edge;
    console.log('边被点击:', edge);
    selectedEdges = [edge];
    selectedNodes = [];
  }

  // 处理画布点击（空白处）
  function handlePaneClick(event) {
    console.log('画布被点击');
    selectedNodes = [];
    selectedEdges = [];
    // 点击空白处，取消选中
    const customEvent = new CustomEvent('node-click', { detail: null });
    window.dispatchEvent(customEvent);
  }

  // 处理画布右键
  function handlePaneContextMenu(event) {
    event.preventDefault();
    const bounds = event.target.getBoundingClientRect();
    contextMenu = {
      show: true,
      x: event.clientX,
      y: event.clientY,
      position: {
        x: event.clientX - bounds.left,
        y: event.clientY - bounds.top
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

    nodesStore.update(nodes => [...nodes, newNode]);
    const event = new CustomEvent('nodes-change', { detail: $nodesStore });
    window.dispatchEvent(event);

    contextMenu.show = false;
  }

  // 获取默认节点数据
  function getDefaultNodeData(type) {
    switch (type) {
      case 'dialogue':
        return { speakerId: '', textKey: '' };
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

<div class="graph-editor" on:contextmenu={handlePaneContextMenu} on:click={closeContextMenu}>
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
    on:nodedragstop={handleNodeDragStop}
    on:edgeupdate={handleEdgeUpdate}
    on:edgeupdatestart={handleEdgeUpdateStart}
    on:edgeupdateend={handleEdgeUpdateEnd}
    defaultEdgeOptions={{ type: 'default', animated: false }}
    nodesDraggable={true}
    nodesConnectable={true}
    elementsSelectable={true}
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
    <MiniMap
      nodeColor="#a855f7"
      maskColor="rgba(255, 255, 255, 0.6)"
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
