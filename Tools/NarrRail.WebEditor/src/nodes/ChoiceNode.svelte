<script>
  export let data;
  export let selected = false;
</script>

<div class="choice-node" class:selected>
  <div class="node-header">
    <span class="material-symbols-outlined node-icon">fork_right</span>
    <span class="node-type">选择</span>
  </div>
  <div class="node-content">
    <div class="node-field">
      <span class="field-label">选项数:</span>
      <span class="field-value">{data.choices?.length || 0}</span>
    </div>
    {#if data.choices && data.choices.length > 0}
      <div class="choices-preview">
        {#each data.choices.slice(0, 2) as choice, i}
          <div class="choice-item">
            <span class="choice-bullet">•</span>
            <span class="choice-text">{choice.textKey || `选项 ${i + 1}`}</span>
          </div>
        {/each}
        {#if data.choices.length > 2}
          <div class="choice-more">+{data.choices.length - 2} 更多</div>
        {/if}
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
    border-color: rgba(255, 204, 0, 0.8);
    box-shadow: 0 8px 32px rgba(255, 204, 0, 0.3),
                0 0 0 4px rgba(255, 204, 0, 0.1),
                inset 0 1px 0 rgba(255, 255, 255, 0.9);
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
    font-family: 'Plus Jakarta Sans', sans-serif;
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
    gap: 8px;
  }

  .node-field {
    display: flex;
    align-items: center;
    gap: 8px;
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
    font-weight: 700;
    color: #1d1d1f;
  }

  .choices-preview {
    display: flex;
    flex-direction: column;
    gap: 4px;
    margin-top: 4px;
  }

  .choice-item {
    display: flex;
    align-items: center;
    gap: 6px;
    font-size: 12px;
    color: #1d1d1f;
  }

  .choice-bullet {
    color: #ffcc00;
    font-weight: 700;
  }

  .choice-text {
    font-weight: 500;
    max-width: 150px;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }

  .choice-more {
    font-size: 11px;
    color: #86868b;
    font-weight: 600;
    font-style: italic;
    margin-top: 2px;
  }
</style>
