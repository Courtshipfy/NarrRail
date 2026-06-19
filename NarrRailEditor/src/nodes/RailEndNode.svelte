<script>
  import IconGlyph from '../components/IconGlyph.svelte';
  import { Handle, Position } from '@xyflow/svelte';

  export let data;
  export let selected = false;
</script>

<div class="rail-end-node" class:selected>
  <Handle type="target" position={Position.Left} />

  <div class="node-header">
    <span class="node-icon"><IconGlyph name="stop_circle" /></span>
    <span class="node-type">总纲结束</span>
  </div>
  <div class="node-content">
    <span>{data?.summary || '当前路线到此结束'}</span>
  </div>
</div>

<style>
  .rail-end-node {
    min-width: 180px;
    background: rgba(255, 255, 255, 0.9);
    backdrop-filter: blur(20px);
    border-radius: 16px;
    border: 2px solid rgba(244, 63, 94, 0.32);
    box-shadow:
      0 4px 16px rgba(244, 63, 94, 0.15),
      inset 0 1px 0 rgba(255, 255, 255, 0.8);
    transition: all 0.25s ease;
    cursor: pointer;
  }

  .rail-end-node:hover {
    transform: translateY(-2px);
    border-color: rgba(244, 63, 94, 0.55);
    box-shadow:
      0 8px 24px rgba(244, 63, 94, 0.24),
      inset 0 1px 0 rgba(255, 255, 255, 0.9);
  }

  .rail-end-node.selected {
    border-color: rgba(244, 63, 94, 0.92);
    box-shadow:
      0 12px 30px rgba(15, 23, 42, 0.18),
      0 0 0 6px rgba(244, 63, 94, 0.22);
  }

  .node-header {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 12px 16px;
    border-bottom: 1px solid rgba(0, 0, 0, 0.05);
    background: linear-gradient(180deg, rgba(244, 63, 94, 0.1), transparent);
  }

  .node-icon,
  .node-type {
    color: #f43f5e;
  }

  .node-type {
    font-family: 'Outfit', sans-serif;
    font-size: 13px;
    font-weight: 800;
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }

  .node-content {
    display: flex;
    flex-direction: column;
    gap: 6px;
    padding: 12px 16px;
  }

  .node-content span {
    color: #64748b;
    font-size: 12px;
  }

  :global(body.dark-theme) .rail-end-node,
  :global(body[data-theme="dark"]) .rail-end-node {
    background: rgba(15, 23, 42, 0.95);
    border-color: rgba(251, 113, 133, 0.5);
    box-shadow:
      0 10px 24px rgba(0, 0, 0, 0.45),
      inset 0 1px 0 rgba(255, 255, 255, 0.05);
  }

  :global(body.dark-theme) .rail-end-node:hover,
  :global(body[data-theme="dark"]) .rail-end-node:hover {
    border-color: rgba(251, 113, 133, 0.78);
    box-shadow:
      0 14px 30px rgba(0, 0, 0, 0.5),
      0 0 0 1px rgba(251, 113, 133, 0.18),
      inset 0 1px 0 rgba(255, 255, 255, 0.06);
  }

  :global(body.dark-theme) .rail-end-node.selected,
  :global(body[data-theme="dark"]) .rail-end-node.selected {
    border-color: rgba(251, 113, 133, 0.95);
    box-shadow:
      0 12px 30px rgba(0, 0, 0, 0.55),
      0 0 0 2px rgba(15, 23, 42, 1),
      0 0 0 6px rgba(251, 113, 133, 0.3),
      inset 0 1px 0 rgba(255, 255, 255, 0.06);
  }

  :global(body.dark-theme) .rail-end-node .node-header,
  :global(body[data-theme="dark"]) .rail-end-node .node-header {
    border-bottom-color: rgba(251, 113, 133, 0.18) !important;
    background: linear-gradient(180deg, rgba(244, 63, 94, 0.2), transparent) !important;
  }

  :global(body.dark-theme) .rail-end-node .node-content span,
  :global(body[data-theme="dark"]) .rail-end-node .node-content span {
    color: #cbd5e1;
  }
</style>
