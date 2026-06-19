<script>
  import IconGlyph from '../components/IconGlyph.svelte';
  import { Handle, Position } from '@xyflow/svelte';

  export let data;
  export let selected = false;

  $: branches = Array.isArray(data?.branches) ? data.branches : [];
</script>

<div class="rail-branch-node" class:selected>
  <Handle type="target" position={Position.Left} />
  <div class="node-header">
    <span class="node-icon"><IconGlyph name="rule" /></span>
    <span class="node-type">总纲分支</span>
  </div>
  <div class="node-content">
    {#if data?.summary}
      <p class="branch-summary">{data.summary}</p>
    {/if}
    {#each branches as branch, index}
      <div class="branch-row">
        <span>{branch?.label || `分支 ${index + 1}`}</span>
        <Handle type="source" position={Position.Right} id={`branch-${index}`} />
      </div>
    {/each}
    <div class="branch-row fallback">
      <span>未匹配</span>
      <Handle type="source" position={Position.Right} id="branch-fallback" />
    </div>
  </div>
</div>

<style>
  .rail-branch-node {
    min-width: 240px;
    background: rgba(255, 255, 255, 0.9);
    backdrop-filter: blur(20px);
    border-radius: 16px;
    border: 2px solid rgba(139, 92, 246, 0.32);
    box-shadow: 0 4px 16px rgba(139, 92, 246, 0.16), inset 0 1px 0 rgba(255, 255, 255, 0.8);
    transition: all 0.25s ease;
    cursor: pointer;
  }
  .rail-branch-node:hover { transform: translateY(-2px); border-color: rgba(139, 92, 246, 0.55); }
  .rail-branch-node.selected {
    border-color: rgba(139, 92, 246, 0.95);
    box-shadow: 0 12px 30px rgba(15, 23, 42, 0.18), 0 0 0 6px rgba(139, 92, 246, 0.22);
  }
  .node-header {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 12px 16px;
    border-bottom: 1px solid rgba(0, 0, 0, 0.05);
    background: linear-gradient(180deg, rgba(139, 92, 246, 0.1), transparent);
  }
  .node-icon, .node-type { color: #8b5cf6; }
  .node-type {
    font-family: 'Outfit', sans-serif;
    font-size: 13px;
    font-weight: 800;
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }
  .node-content { padding: 12px 16px; }
  .branch-summary {
    margin: 0 0 8px;
    color: #64748b;
    font-size: 12px;
    line-height: 1.4;
  }
  .branch-row {
    position: relative;
    min-height: 24px;
    display: flex;
    align-items: center;
    padding-right: 22px;
    color: #64748b;
    font-size: 12px;
    font-weight: 700;
  }
  .branch-row.fallback {
    margin-top: 6px;
    padding-top: 8px;
    border-top: 1px solid rgba(148, 163, 184, 0.22);
  }
  .branch-row :global(.svelte-flow__handle) {
    position: absolute;
    right: -8px;
    top: 50%;
    transform: translateY(-50%);
  }

  :global(body.dark-theme) .rail-branch-node,
  :global(body[data-theme="dark"]) .rail-branch-node {
    background: rgba(15, 23, 42, 0.95);
    border-color: rgba(167, 139, 250, 0.5);
    box-shadow:
      0 10px 24px rgba(0, 0, 0, 0.45),
      inset 0 1px 0 rgba(255, 255, 255, 0.05);
  }

  :global(body.dark-theme) .rail-branch-node:hover,
  :global(body[data-theme="dark"]) .rail-branch-node:hover {
    border-color: rgba(167, 139, 250, 0.78);
    box-shadow:
      0 14px 30px rgba(0, 0, 0, 0.5),
      0 0 0 1px rgba(167, 139, 250, 0.18),
      inset 0 1px 0 rgba(255, 255, 255, 0.06);
  }

  :global(body.dark-theme) .rail-branch-node.selected,
  :global(body[data-theme="dark"]) .rail-branch-node.selected {
    border-color: rgba(167, 139, 250, 0.95);
    box-shadow:
      0 12px 30px rgba(0, 0, 0, 0.55),
      0 0 0 2px rgba(15, 23, 42, 1),
      0 0 0 6px rgba(167, 139, 250, 0.32),
      inset 0 1px 0 rgba(255, 255, 255, 0.06);
  }

  :global(body.dark-theme) .rail-branch-node .node-header,
  :global(body[data-theme="dark"]) .rail-branch-node .node-header {
    border-bottom-color: rgba(167, 139, 250, 0.2) !important;
    background: linear-gradient(180deg, rgba(139, 92, 246, 0.2), transparent) !important;
  }

  :global(body.dark-theme) .rail-branch-node .branch-row,
  :global(body.dark-theme) .rail-branch-node .branch-summary,
  :global(body[data-theme="dark"]) .rail-branch-node .branch-row {
    color: #cbd5e1;
  }

  :global(body[data-theme="dark"]) .rail-branch-node .branch-summary {
    color: #cbd5e1;
  }

  :global(body.dark-theme) .rail-branch-node .branch-row.fallback,
  :global(body[data-theme="dark"]) .rail-branch-node .branch-row.fallback {
    border-top-color: rgba(148, 163, 184, 0.28);
  }
</style>
