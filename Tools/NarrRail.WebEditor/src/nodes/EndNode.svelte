<script>
  import { Handle, Position } from '@xyflow/svelte';

  export let selected = false;
</script>

<div class="end-node" class:selected>
  <Handle type="target" position={Position.Left} />

  <div class="node-header">
    <span class="material-symbols-outlined node-icon">stop_circle</span>
    <span class="node-type">结束</span>
  </div>
  <div class="node-content">
    <div class="end-message">剧情结束</div>
  </div>
</div>

<style>
  .end-node {
    min-width: 160px;
    background: rgba(255, 255, 255, 0.9);
    backdrop-filter: blur(20px);
    border-radius: 16px;
    border: 2px solid rgba(255, 59, 48, 0.3);
    box-shadow: 0 4px 16px rgba(255, 59, 48, 0.15),
                inset 0 1px 0 rgba(255, 255, 255, 0.8);
    transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    cursor: pointer;
  }

  .end-node:hover {
    transform: translateY(-2px);
    box-shadow: 0 8px 24px rgba(255, 59, 48, 0.25),
                inset 0 1px 0 rgba(255, 255, 255, 0.9);
    border-color: rgba(255, 59, 48, 0.5);
  }

  .end-node.selected {
    border-color: rgba(255, 59, 48, 0.88);
    box-shadow: 0 10px 26px rgba(15, 23, 42, 0.18),
                0 0 0 2px rgba(255, 255, 255, 0.92),
                0 0 0 6px rgba(255, 59, 48, 0.22),
                inset 0 1px 0 rgba(255, 255, 255, 0.95);
    transform: translateY(-1px);
  }

  .node-header {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 12px 16px;
    border-bottom: 1px solid rgba(0, 0, 0, 0.05);
    background: linear-gradient(180deg, rgba(255, 59, 48, 0.08) 0%, transparent 100%);
  }

  .node-icon {
    font-size: 20px;
    color: #ff3b30;
  }

  .node-type {
    font-family: 'Plus Jakarta Sans', sans-serif;
    font-size: 13px;
    font-weight: 700;
    color: #ff3b30;
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }

  .node-content {
    padding: 12px 16px;
    text-align: center;
  }

  .end-message {
    font-size: 12px;
    font-weight: 600;
    color: #86868b;
    font-style: italic;
  }
</style>
