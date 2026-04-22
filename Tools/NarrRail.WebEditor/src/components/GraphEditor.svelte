<script>
  import { onMount } from 'svelte';
  import { writable } from 'svelte/store';

  export let nodes = [];
  export let edges = [];

  const nodesStore = writable(nodes);
  const edgesStore = writable(edges);

  // 监听外部 props 变化
  $: nodesStore.set(nodes);
  $: edgesStore.set(edges);

  let mounted = false;

  onMount(() => {
    mounted = true;
  });

  function handleNodeClick(event) {
    // 触发 Vue 父组件的事件
    const customEvent = new CustomEvent('node-click', {
      detail: event.detail
    });
    document.dispatchEvent(customEvent);
  }
</script>

<div class="graph-editor">
  {#if mounted}
    <div class="placeholder">
      <div class="placeholder-content">
        <h3>Svelte Flow 图编辑器</h3>
        <p>正在集成 @xyflow/svelte...</p>
        <p class="hint">节点数: {nodes.length} | 边数: {edges.length}</p>
      </div>
    </div>
  {/if}
</div>

<style>
  .graph-editor {
    width: 100%;
    height: 100%;
    background: #ecf0f1;
    position: relative;
  }

  .placeholder {
    width: 100%;
    height: 100%;
    display: flex;
    align-items: center;
    justify-content: center;
  }

  .placeholder-content {
    text-align: center;
    color: #7f8c8d;
  }

  .placeholder-content h3 {
    margin: 0 0 10px 0;
    color: #2c3e50;
  }

  .placeholder-content p {
    margin: 5px 0;
  }

  .placeholder-content .hint {
    margin-top: 20px;
    font-size: 14px;
    color: #95a5a6;
  }
</style>
