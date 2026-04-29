<script>
  import { Handle, Position } from '@xyflow/svelte';

  export let data;
  export let selected = false;
</script>

<div class="jump-node" class:selected>
  <Handle type="target" position={Position.Left} />

  <div class="node-header">
    <span class="material-symbols-outlined node-icon">arrow_forward</span>
    <span class="node-type">跳转</span>
  </div>
  <div class="node-content">
    <div class="node-field">
      <span class="field-label">目标节点:</span>
      <span class="field-value">{data.targetNodeId || '未设置'}</span>
    </div>
  </div>

  <Handle type="source" position={Position.Right} />
</div>

<style>
  .jump-node {
    min-width: 180px;
    background: rgba(255, 255, 255, 0.9);
    backdrop-filter: blur(20px);
    border-radius: 16px;
    border: 2px solid rgba(0, 122, 255, 0.3);
    box-shadow: 0 4px 16px rgba(0, 122, 255, 0.15),
                inset 0 1px 0 rgba(255, 255, 255, 0.8);
    transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    cursor: pointer;
  }

  .jump-node:hover {
    transform: translateY(-2px);
    box-shadow: 0 8px 24px rgba(0, 122, 255, 0.25),
                inset 0 1px 0 rgba(255, 255, 255, 0.9);
    border-color: rgba(0, 122, 255, 0.5);
  }

  .jump-node.selected {
    border-color: rgba(0, 122, 255, 0.92);
    box-shadow: 0 10px 28px rgba(0, 122, 255, 0.24),
                0 0 0 2px rgba(255, 255, 255, 0.95),
                0 0 0 6px rgba(0, 122, 255, 0.16),
                inset 0 1px 0 rgba(255, 255, 255, 0.95);
    transform: translateY(-1px);
  }

  .node-header {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 12px 16px;
    border-bottom: 1px solid rgba(0, 0, 0, 0.05);
    background: linear-gradient(180deg, rgba(0, 122, 255, 0.08) 0%, transparent 100%);
  }

  .node-icon {
    font-size: 20px;
    color: #007aff;
  }

  .node-type {
    font-family: 'Plus Jakarta Sans', sans-serif;
    font-size: 13px;
    font-weight: 700;
    color: #007aff;
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }

  .node-content {
    padding: 12px 16px;
  }

  .node-field {
    display: flex;
    flex-direction: column;
    gap: 4px;
  }

  .field-label {
    font-size: 10px;
    font-weight: 700;
    color: #86868b;
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }

  .field-value {
    font-size: 13px;
    font-weight: 600;
    color: #1d1d1f;
    max-width: 150px;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }
</style>
