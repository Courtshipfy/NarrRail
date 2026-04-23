<script>
  export let data;
  export let selected = false;
</script>

<div class="dialogue-node" class:selected>
  <div class="node-header">
    <span class="material-symbols-outlined node-icon">chat</span>
    <span class="node-type">对话</span>
  </div>
  <div class="node-content">
    <div class="node-field">
      <span class="field-label">说话人:</span>
      <span class="field-value">{data.speakerId || '未设置'}</span>
    </div>
    <div class="node-field">
      <span class="field-label">文本:</span>
      <span class="field-value text-preview">{data.textKey || '未设置'}</span>
    </div>
  </div>
</div>

<style>
  .dialogue-node {
    min-width: 200px;
    background: rgba(255, 255, 255, 0.9);
    backdrop-filter: blur(20px);
    border-radius: 16px;
    border: 2px solid rgba(52, 199, 89, 0.3);
    box-shadow: 0 4px 16px rgba(52, 199, 89, 0.15),
                inset 0 1px 0 rgba(255, 255, 255, 0.8);
    transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    cursor: pointer;
  }

  .dialogue-node:hover {
    transform: translateY(-2px);
    box-shadow: 0 8px 24px rgba(52, 199, 89, 0.25),
                inset 0 1px 0 rgba(255, 255, 255, 0.9);
    border-color: rgba(52, 199, 89, 0.5);
  }

  .dialogue-node.selected {
    border-color: rgba(52, 199, 89, 0.8);
    box-shadow: 0 8px 32px rgba(52, 199, 89, 0.3),
                0 0 0 4px rgba(52, 199, 89, 0.1),
                inset 0 1px 0 rgba(255, 255, 255, 0.9);
  }

  .node-header {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 12px 16px;
    border-bottom: 1px solid rgba(0, 0, 0, 0.05);
    background: linear-gradient(180deg, rgba(52, 199, 89, 0.08) 0%, transparent 100%);
  }

  .node-icon {
    font-size: 20px;
    color: #34c759;
  }

  .node-type {
    font-family: 'Plus Jakarta Sans', sans-serif;
    font-size: 13px;
    font-weight: 700;
    color: #34c759;
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
  }

  .text-preview {
    max-width: 180px;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }
</style>
