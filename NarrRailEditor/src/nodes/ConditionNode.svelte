<script>
  import IconGlyph from '../components/IconGlyph.svelte';
  import { Handle, Position } from '@xyflow/svelte';

  export let data;
  export let selected = false;

  function getBranches(condition) {
    if (Array.isArray(condition?.branches)) {
      return condition.branches;
    }
    if (Array.isArray(condition?.terms)) {
      return [
        {
          label: '条件 1',
          logic: condition?.logic || 'All',
          terms: condition.terms
        }
      ];
    }
    return [];
  }

  function summarizeBranch(branch, index) {
    const logic = branch?.logic || 'All';
    const terms = Array.isArray(branch?.terms) ? branch.terms : [];
    const label = String(branch?.label || `条件 ${index + 1}`).trim();
    if (terms.length === 0) return `${label}: ${logic} (always)`;

    const first = terms[0] || {};
    const varName = first?.variable?.name || 'var';
    const op = first?.operator || '==';
    const value = first?.compareValue ?? '';
    const suffix = terms.length > 1 ? ` +${terms.length - 1}` : '';
    return `${label}: ${varName} ${op} ${value}${suffix}`;
  }

  $: branches = getBranches(data?.condition);
</script>

<div class="condition-node" class:selected>
  <Handle type="target" position={Position.Left} />

  <div class="node-header">
    <span class="node-icon"><IconGlyph name="rule" /></span>
    <span class="node-type">条件</span>
  </div>

  <div class="node-content">
    {#if branches.length === 0}
      <div class="condition-summary">暂无条件分支</div>
    {:else}
      {#each branches as branch, index}
        <div class="branch">
          <span class="branch-label">{summarizeBranch(branch, index)}</span>
          <Handle type="source" position={Position.Right} id={`condition-${index}`} />
        </div>
      {/each}
    {/if}

    <div class="branch branch-fallback">
      <span class="branch-label">未匹配</span>
      <Handle type="source" position={Position.Right} id="condition-fallback" />
    </div>
  </div>
</div>

<style>
  .condition-node {
    min-width: 230px;
    background: rgba(255, 255, 255, 0.9);
    backdrop-filter: blur(20px);
    border-radius: 16px;
    border: 2px solid rgba(14, 165, 233, 0.3);
    box-shadow: 0 4px 16px rgba(14, 165, 233, 0.15),
                inset 0 1px 0 rgba(255, 255, 255, 0.8);
    transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    cursor: pointer;
  }

  .condition-node:hover {
    transform: translateY(-2px);
    box-shadow: 0 8px 24px rgba(14, 165, 233, 0.25),
                inset 0 1px 0 rgba(255, 255, 255, 0.9);
    border-color: rgba(14, 165, 233, 0.5);
  }

  .condition-node.selected {
    border-color: rgba(99, 102, 241, 0.85);
    box-shadow: 0 10px 28px rgba(15, 23, 42, 0.2),
                0 0 0 2px rgba(255, 255, 255, 0.95),
                0 0 0 6px rgba(99, 102, 241, 0.24),
                inset 0 1px 0 rgba(255, 255, 255, 0.95);
  }

  .node-header {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 12px 16px;
    border-bottom: 1px solid rgba(0, 0, 0, 0.05);
    background: linear-gradient(180deg, rgba(14, 165, 233, 0.08) 0%, transparent 100%);
  }

  .node-icon {
    font-size: 20px;
    color: #0ea5e9;
  }

  .node-type {
    font-family: 'Outfit', sans-serif;
    font-size: 13px;
    font-weight: 700;
    color: #0ea5e9;
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }

  .node-content {
    padding: 12px 16px;
  }

  .condition-summary {
    font-size: 12px;
    color: #1d1d1f;
    margin-bottom: 10px;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
  }

  .branch {
    position: relative;
    min-height: 24px;
    display: flex;
    align-items: center;
    padding-right: 22px;
    margin-top: 4px;
  }

  .branch-label {
    font-size: 12px;
    font-weight: 700;
    color: #64748b;
    max-width: 175px;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
  }

  .branch-fallback {
    margin-top: 8px;
    padding-top: 8px;
    border-top: 1px solid rgba(148, 163, 184, 0.22);
  }

  .branch :global(.svelte-flow__handle) {
    position: absolute;
    right: -8px;
    top: 50%;
    transform: translateY(-50%);
  }
</style>
