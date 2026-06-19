<script>
  import IconGlyph from '../components/IconGlyph.svelte';
  import { Handle, Position } from '@xyflow/svelte';

  export let data;
  export let selected = false;
</script>

<div class="rail-note-node" class:selected>
  <Handle type="target" position={Position.Left} />
  <div class="node-header">
    <span class="node-icon"><IconGlyph name="sticky_note_2" /></span>
    <span class="node-type">章节标记</span>
  </div>
  <div class="node-content">
    <span>{data?.summary || '用于总纲说明，可自动通过'}</span>
  </div>
  <Handle type="source" position={Position.Right} />
</div>

<style>
  .rail-note-node {
    min-width: 210px;
    background: rgba(255, 255, 255, 0.9);
    backdrop-filter: blur(20px);
    border-radius: 16px;
    border: 2px solid rgba(16, 185, 129, 0.32);
    box-shadow: 0 4px 16px rgba(16, 185, 129, 0.14), inset 0 1px 0 rgba(255, 255, 255, 0.8);
    transition: all 0.25s ease;
    cursor: pointer;
  }
  .rail-note-node:hover { transform: translateY(-2px); border-color: rgba(16, 185, 129, 0.55); }
  .rail-note-node.selected {
    border-color: rgba(16, 185, 129, 0.9);
    box-shadow: 0 12px 30px rgba(15, 23, 42, 0.18), 0 0 0 6px rgba(16, 185, 129, 0.2);
  }
  .node-header {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 12px 16px;
    border-bottom: 1px solid rgba(0, 0, 0, 0.05);
    background: linear-gradient(180deg, rgba(16, 185, 129, 0.1), transparent);
  }
  .node-icon, .node-type { color: #10b981; }
  .node-type {
    font-family: 'Outfit', sans-serif;
    font-size: 13px;
    font-weight: 800;
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }
  .node-content { display: flex; flex-direction: column; gap: 6px; padding: 12px 16px; }
  .node-content span { color: #64748b; font-size: 12px; }

  :global(body.dark-theme) .rail-note-node,
  :global(body[data-theme="dark"]) .rail-note-node {
    background: rgba(15, 23, 42, 0.95);
    border-color: rgba(52, 211, 153, 0.5);
    box-shadow:
      0 10px 24px rgba(0, 0, 0, 0.45),
      inset 0 1px 0 rgba(255, 255, 255, 0.05);
  }

  :global(body.dark-theme) .rail-note-node:hover,
  :global(body[data-theme="dark"]) .rail-note-node:hover {
    border-color: rgba(52, 211, 153, 0.78);
    box-shadow:
      0 14px 30px rgba(0, 0, 0, 0.5),
      0 0 0 1px rgba(52, 211, 153, 0.18),
      inset 0 1px 0 rgba(255, 255, 255, 0.06);
  }

  :global(body.dark-theme) .rail-note-node.selected,
  :global(body[data-theme="dark"]) .rail-note-node.selected {
    border-color: rgba(52, 211, 153, 0.95);
    box-shadow:
      0 12px 30px rgba(0, 0, 0, 0.55),
      0 0 0 2px rgba(15, 23, 42, 1),
      0 0 0 6px rgba(52, 211, 153, 0.3),
      inset 0 1px 0 rgba(255, 255, 255, 0.06);
  }

  :global(body.dark-theme) .rail-note-node .node-header,
  :global(body[data-theme="dark"]) .rail-note-node .node-header {
    border-bottom-color: rgba(52, 211, 153, 0.18) !important;
    background: linear-gradient(180deg, rgba(16, 185, 129, 0.2), transparent) !important;
  }

  :global(body.dark-theme) .rail-note-node .node-content span,
  :global(body[data-theme="dark"]) .rail-note-node .node-content span {
    color: #cbd5e1;
  }
</style>
