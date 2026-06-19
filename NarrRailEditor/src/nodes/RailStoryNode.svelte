<script>
  import IconGlyph from '../components/IconGlyph.svelte';
  import { Handle, Position } from '@xyflow/svelte';

  export let data;
  export let selected = false;
</script>

<div class="rail-story-node" class:selected>
  <Handle type="target" position={Position.Left} />
  <div class="node-header">
    <span class="node-icon"><IconGlyph name="article" /></span>
    <span class="node-type">脚本</span>
  </div>
  <div class="node-content">
    <span class="story-id">{data?.storyId || '未绑定 storyId'}</span>
    {#if data?.summary}
      <p>{data.summary}</p>
    {/if}
  </div>
  <Handle type="source" position={Position.Right} />
</div>

<style>
  .rail-story-node {
    min-width: 230px;
    background: rgba(255, 255, 255, 0.9);
    backdrop-filter: blur(20px);
    border-radius: 16px;
    border: 2px solid rgba(14, 165, 233, 0.32);
    box-shadow: 0 4px 16px rgba(14, 165, 233, 0.16), inset 0 1px 0 rgba(255, 255, 255, 0.8);
    transition: all 0.25s ease;
    cursor: pointer;
  }
  .rail-story-node:hover { transform: translateY(-2px); border-color: rgba(14, 165, 233, 0.55); }
  .rail-story-node.selected {
    border-color: rgba(14, 165, 233, 0.95);
    box-shadow: 0 12px 30px rgba(15, 23, 42, 0.18), 0 0 0 6px rgba(14, 165, 233, 0.22);
  }
  .node-header {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 12px 16px;
    border-bottom: 1px solid rgba(0, 0, 0, 0.05);
    background: linear-gradient(180deg, rgba(14, 165, 233, 0.1), transparent);
  }
  .node-icon, .node-type { color: #0ea5e9; }
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
  .node-content span, .node-content p { color: #64748b; font-size: 12px; margin: 0; }
  .story-id { font-weight: 700; }

  :global(body.dark-theme) .rail-story-node,
  :global(body[data-theme="dark"]) .rail-story-node {
    background: rgba(15, 23, 42, 0.95);
    border-color: rgba(56, 189, 248, 0.5);
    box-shadow:
      0 10px 24px rgba(0, 0, 0, 0.45),
      inset 0 1px 0 rgba(255, 255, 255, 0.05);
  }

  :global(body.dark-theme) .rail-story-node:hover,
  :global(body[data-theme="dark"]) .rail-story-node:hover {
    border-color: rgba(56, 189, 248, 0.78);
    box-shadow:
      0 14px 30px rgba(0, 0, 0, 0.5),
      0 0 0 1px rgba(56, 189, 248, 0.18),
      inset 0 1px 0 rgba(255, 255, 255, 0.06);
  }

  :global(body.dark-theme) .rail-story-node.selected,
  :global(body[data-theme="dark"]) .rail-story-node.selected {
    border-color: rgba(56, 189, 248, 0.95);
    box-shadow:
      0 12px 30px rgba(0, 0, 0, 0.55),
      0 0 0 2px rgba(15, 23, 42, 1),
      0 0 0 6px rgba(56, 189, 248, 0.3),
      inset 0 1px 0 rgba(255, 255, 255, 0.06);
  }

  :global(body.dark-theme) .rail-story-node .node-header,
  :global(body[data-theme="dark"]) .rail-story-node .node-header {
    border-bottom-color: rgba(56, 189, 248, 0.18) !important;
    background: linear-gradient(180deg, rgba(14, 165, 233, 0.2), transparent) !important;
  }

  :global(body.dark-theme) .rail-story-node .node-content span,
  :global(body.dark-theme) .rail-story-node .node-content p,
  :global(body[data-theme="dark"]) .rail-story-node .node-content span,
  :global(body[data-theme="dark"]) .rail-story-node .node-content p {
    color: #cbd5e1;
  }
</style>
