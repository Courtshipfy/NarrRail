<script>
  import IconGlyph from '../components/IconGlyph.svelte';
  import { Handle, Position } from '@xyflow/svelte';

  export let data;
  export let selected = false;
</script>

<div class="choice-node" class:selected>
  <Handle type="target" position={Position.Left} />

  <div class="node-header">
    <span class="node-icon"><IconGlyph name="fork_right" /></span>
    <span class="node-type">选择</span>
  </div>
  <div class="node-content">
    {#if data.choices && data.choices.length > 0}
      <div class="choices-list">
        {#each data.choices as choice, i}
          <div class="choice-item-with-handle">
            <div class="choice-item">
              <span class="choice-text">{choice.textKey || `选项 ${i + 1}`}</span>
            </div>
            <Handle
              type="source"
              position={Position.Right}
              id={`choice-${i}`}
            />
          </div>
        {/each}
      </div>
    {:else}
      <div class="no-choices">暂无选项</div>
    {/if}

    {#if data?.choiceMode === 'ExhaustiveUntilComplete'}
      <div class="completion-row-with-handle" aria-hidden="true">
        <Handle
          type="source"
          position={Position.Right}
          id="choice-complete"
        />
      </div>
    {/if}
  </div>
</div>

<style>
  .choice-node {
    min-width: 200px;
    background: rgba(255, 255, 255, 0.9);
    backdrop-filter: blur(20px);
    border-radius: 16px;
    border: 2px solid rgba(255, 204, 0, 0.3);
    box-shadow: 0 4px 16px rgba(255, 204, 0, 0.15),
                inset 0 1px 0 rgba(255, 255, 255, 0.8);
    transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    cursor: pointer;
  }

  .choice-node:hover {
    transform: translateY(-2px);
    box-shadow: 0 8px 24px rgba(255, 204, 0, 0.25),
                inset 0 1px 0 rgba(255, 255, 255, 0.9);
    border-color: rgba(255, 204, 0, 0.5);
  }

  .choice-node.selected {
    border-color: rgba(255, 204, 0, 0.78);
    transform: translateY(-1px);
    box-shadow:
      0 10px 28px rgba(15, 23, 42, 0.14),
      0 0 0 2px rgba(255, 255, 255, 0.9),
      0 0 0 6px rgba(168, 85, 247, 0.2),
      inset 0 1px 0 rgba(255, 255, 255, 0.95);
  }

  .node-header {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 12px 16px;
    border-bottom: 1px solid rgba(0, 0, 0, 0.05);
    background: linear-gradient(180deg, rgba(255, 204, 0, 0.08) 0%, transparent 100%);
  }

  .node-icon {
    font-size: 20px;
    color: #ffcc00;
  }

  .node-type {
    font-family: 'Outfit', sans-serif;
    font-size: 13px;
    font-weight: 700;
    color: #ffcc00;
    text-transform: uppercase;
    letter-spacing: 0.05em;
  }

  .node-content {
    padding: 12px 16px;
    display: flex;
    flex-direction: column;
    gap: 0;
  }

  .choices-list {
    display: flex;
    flex-direction: column;
    gap: 0;
  }

  .completion-row-with-handle {
    position: relative;
    height: 18px;
    padding-right: 20px;
    margin-top: 6px;
  }

  .completion-row-with-handle :global(.svelte-flow__handle) {
    position: absolute;
    right: -8px;
    top: 50%;
    transform: translateY(-50%);
    background: #7c3aed;
    border-color: #7c3aed;
  }

  .choice-item-with-handle {
    position: relative;
    min-height: 32px;
    display: flex;
    align-items: center;
    padding-right: 20px;
  }

  .choice-item {
    display: flex;
    align-items: flex-start;
    gap: 8px;
    font-size: 13px;
    color: #1d1d1f;
    padding: 6px 0;
    flex: 1;
  }

  .choice-item-with-handle :global(.svelte-flow__handle) {
    position: absolute;
    right: -8px;
    top: 50%;
    transform: translateY(-50%);
  }



  .choice-text {
    font-weight: 500;
    line-height: 1.4;
    white-space: normal;
    overflow-wrap: anywhere;
    word-break: break-word;
  }

  .no-choices {
    font-size: 12px;
    color: #86868b;
    font-style: italic;
    text-align: center;
    padding: 8px 0;
  }
</style>
