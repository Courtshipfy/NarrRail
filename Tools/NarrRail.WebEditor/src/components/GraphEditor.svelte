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

  // 初始化时设置数据
  onMount(() => {
    console.log('GraphEditor mounted with nodes:', nodes);
    console.log('GraphEditor mounted with edges:', edges);
    nodesStore.set(nodes);
    edgesStore.set(edges);
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
    nodesStore.update(nodes => {
      changes.forEach(change => {
        if (change.type === 'position' && change.dragging === false) {
          const node = nodes.find(n => n.id === change.id);
          if (node && change.position) {
            node.position = change.position;
          }
        }
      });
      return nodes;
    });

    // 触发父组件更新
    const event = new CustomEvent('nodes-change', { detail: $nodesStore });
    window.dispatchEvent(event);
  }

  // 处理边变化
  function handleEdgesChange(changes) {
    edgesStore.update(edges => {
      changes.forEach(change => {
        if (change.type === 'remove') {
          edges = edges.filter(e => e.id !== change.id);
        }
      });
      return edges;
    });

    const event = new CustomEvent('edges-change', { detail: $edgesStore });
    window.dispatchEvent(event);
  }

  // 处理连接
  function handleConnect(connection) {
    const newEdge = {
      id: `e${Date.now()}`,
      source: connection.source,
      target: connection.target,
      type: 'smoothstep',
      animated: true,
      style: 'stroke: rgba(168, 85, 247, 0.6); stroke-width: 2px;',
      data: {
        priority: 0,
        condition: { logic: 'All', terms: [] }
      }
    };

    edgesStore.update(edges => [...edges, newEdge]);

    const event = new CustomEvent('edges-change', { detail: $edgesStore });
    window.dispatchEvent(event);
  }

  // 处理节点点击
  function handleNodeClick(event) {
    const node = event.detail.node;
    const customEvent = new CustomEvent('node-click', { detail: node });
    window.dispatchEvent(customEvent);
  }

  // 处理画布点击（空白处）
  function handlePaneClick(event) {
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
    on:paneclick={handlePaneClick}
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
