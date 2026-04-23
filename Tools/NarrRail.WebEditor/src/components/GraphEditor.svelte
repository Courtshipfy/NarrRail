<script>
  import { writable } from 'svelte/store';
  import { onMount } from 'svelte';
  import { SvelteFlow, Controls, Background, MiniMap } from '@xyflow/svelte';
  import '@xyflow/svelte/dist/style.css';

  import DialogueNode from '../nodes/DialogueNode.svelte';
  import ChoiceNode from '../nodes/ChoiceNode.svelte';
  import EndNode from '../nodes/EndNode.svelte';

  export let nodes = [];
  export let edges = [];

  const nodesStore = writable([]);
  const edgesStore = writable([]);

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
    end: EndNode
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
</script>

<div class="graph-editor">
  <SvelteFlow
    nodes={nodesStore}
    edges={edgesStore}
    {nodeTypes}
    on:nodeschange={handleNodesChange}
    on:edgeschange={handleEdgesChange}
    on:connect={handleConnect}
    on:nodeclick={handleNodeClick}
    fitView
    minZoom={0.2}
    maxZoom={2}
  >
    <Background variant="dots" gap={20} size={1} color="rgba(0, 0, 0, 0.03)" />
    <Controls
      showZoom={true}
      showFitView={true}
      showInteractive={false}
      style="background: rgba(255, 255, 255, 0.8); backdrop-filter: blur(20px); border-radius: 12px; border: 0.5px solid rgba(0, 0, 0, 0.04);"
    />
    <MiniMap
      nodeColor="#a855f7"
      maskColor="rgba(255, 255, 255, 0.6)"
      style="background: rgba(255, 255, 255, 0.8); backdrop-filter: blur(20px); border-radius: 12px; border: 0.5px solid rgba(0, 0, 0, 0.04);"
    />
  </SvelteFlow>
</div>

<style>
  .graph-editor {
    width: 100%;
    height: 100%;
    position: relative;
  }

  :global(.svelte-flow__node) {
    cursor: grab;
  }

  :global(.svelte-flow__node:active) {
    cursor: grabbing;
  }

  :global(.svelte-flow__edge-path) {
    stroke: rgba(168, 85, 247, 0.4);
    stroke-width: 2px;
  }

  :global(.svelte-flow__edge.selected .svelte-flow__edge-path) {
    stroke: rgba(168, 85, 247, 0.8);
    stroke-width: 3px;
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
</style>
